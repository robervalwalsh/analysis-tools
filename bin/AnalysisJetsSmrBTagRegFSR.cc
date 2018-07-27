/**
 * @author Fabio Zoratti <fabio.zoratti96@gmail.com> <fabio.zoratti@desy.de> <fabio.zoratti@cern.ch>
 * @date 26/7/2018
 * @file AnalysisJetsSmrBTagRegFSR.cc
 * First apply smearing to data, then apply scale factors to correct MC for btag, then uses regression algorithm to correct energy and then corrects for final state radiation.
 */

/**
 * DOCS: select the value for the global variable correction_level and build.
 * Also set the other global variable debug to false.
 */




#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <tuple>

#include "TTree.h"
#include "TFile.h"
#include "TH1.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TRandom3.h"

#include "JetMETCorrections/Modules/interface/JetResolution.h"
#include "CondFormats/BTauObjects/interface/BTagCalibration.h"
#include "CondTools/BTau/interface/BTagCalibrationReader.h"



using std::cout;
using std::string;
using std::endl;
using std::array;
using std::cerr;
using std::vector;

enum class CorrectionLevel { nothing, only_smearing, smearing_btag, \
                             smearing_btag_regression, smearing_btag_regression_fsr };
namespace Smearing {
  enum class method {scaling, stochastic};
}


const string file("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-350_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180712_150216/0000/nano_1.root");
const Float_t rcone(0.4);
const unsigned int kLen = 100;
const Float_t deepcsv_cut = 0.4941;
const Float_t b_tagging_efficiency = 0.99;
const Int_t kTight = 2;
const array<Float_t, 3> pt_cut{{100, 100, 40}};
const array<Float_t, 3> eta_cut{{2.2, 2.2, 2.2}};
const bool debug = false;
const CorrectionLevel correction_level = CorrectionLevel::smearing_btag_regression_fsr;
const Float_t soft_jet_distance = 0.8;



bool check_distance_enough(const TLorentzVector &a, const TLorentzVector &b) {
  if (debug) {
    cout << "Delta R: " << a.DeltaR(b) << endl;
    cout << "Eta bw 4vec: " << TMath::Abs(a.Eta() - b.Eta()) << endl;
  }
  return a.DeltaR(b) > 1;
}

class Jet : public TLorentzVector {
public:
  Jet(const TLorentzVector& a, UInt_t i) : TLorentzVector(a) {
    orig_index_ = i;
  }
  UInt_t Idx() const { return orig_index_; }
  
private:
  
  UInt_t orig_index_;
};


class DataContainer {
 public:
  
  DataContainer();
  int SetTreeBranches(TTree* tree);
  void CreateTree(TTree* tree);
  bool get_real_pt(Int_t index, Float_t* pt);
  void fill_histogram(TTree*, std::vector<TH1F*>, TTree*);
  void initialize_current_data(UInt_t i);
  void sort_by_Pt();
  Smearing::method smearing(const TVector3& jet, const TVector3& genjet, \
                            const Float_t& radius, Float_t* realpt) const;
  
  /**
   * @brief Applies the smearing to the pt
   * MC data is better than the real one, so MC data have to been smeared.
   * The new pt is stored in current_jet_pt_[index]
   */
  void smear_current_data(UInt_t index);

  /**
   * @brief Other scale factors for the MC with btagging
   * The new pt is always current_jet_pt_[index]
   */
  void apply_btag_sf();

  /**
   * @brief Applied the new regression algorithm that improves jet energy
   * resolution for events with leptons. In fact it is a scale factor on the pt
   */
  void apply_jet_reg_sf(UInt_t index);
  void apply_fsr_correction();
  
  /**
   * @brief Reads lev and decides what corrections have to be applied on pt
   */
  void apply_all_corrections(CorrectionLevel lev);
  
 private:
  JME::JetResolution resolution;
  JME::JetResolutionScaleFactor resolution_sf;
  BTagCalibration calib;
  BTagCalibrationReader reader;


  Float_t weigth;
  // Data from tree
  UInt_t run_;
  ULong64_t event_;
  UInt_t lumisection_;
  UInt_t njet_;
  Float_t jet_pt_[kLen];
  Float_t jet_eta_[kLen];
  Float_t jet_phi_[kLen];
  Float_t jet_mass_[kLen];
  Int_t jet_id_[kLen];
  Float_t btag_deep_[kLen];
  Float_t jet_b_reg_corr[kLen];
  Int_t jet_n_electrons;
  Int_t jet_n_muons;
  Int_t jet_genjet_idx_[kLen];
  Float_t genjet_pt_[kLen];
  Float_t genjet_eta_[kLen];
  Float_t genjet_phi_[kLen];
  Bool_t trigger_path;
  
  vector<Jet> corrected_jet;
  Float_t jet_b_reg_res_[kLen];
};

/**
 * @danger I am using the old data table because I did not find the new one.
 */
DataContainer::DataContainer() : resolution("tables/Fall17_25nsV1_MC_PtResolution_AK4PFchs.txt"), \
                                 resolution_sf("tables/Fall17_25nsV1_MC_SF_AK4PFchs.txt"), \
                                 calib("csvv1", "tables/8TeV_SF/CSVV1.csv"),                       \
                                 reader(BTagEntry::OP_TIGHT, "central", {"up", "down"}) {
  reader.load(calib, BTagEntry::FLAV_B, "comb");
}


void DataContainer::sort_by_Pt() {
  std::sort(corrected_jet.begin(), corrected_jet.end(),\
            [](const TLorentzVector&a, const TLorentzVector& b) {
              return a.Pt() > b.Pt();
            });
}

/**
 * @brief Smears MC pt to simulate real data
 * See https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetResolution
 */
Smearing::method DataContainer::smearing(const TVector3& jet, const TVector3& genjet, \
              const Float_t& radius, Float_t* realpt) const {
  JME::JetParameters parameters;
  parameters.setJetPt(jet.Pt());
  parameters.setJetEta(jet.Eta());
  parameters.setRho((jet.Pt() - genjet.Pt())/jet.Pt());
  Float_t sigmajer = resolution.getResolution(parameters);
  Float_t sjer = resolution_sf.getScaleFactor(parameters);
  if ((2*jet.DeltaR(genjet) < radius) && \
      (TMath::Abs(jet.Pt() - genjet.Pt()) < 3*sigmajer*jet.Pt())) {
    Float_t cjer = 1 + (sjer - 1)*(jet.Pt() - genjet.Pt())/jet.Pt();
    if (cjer < 0) {
      cjer = 0;
    }
    *realpt = jet.Pt()*cjer;
    if (debug) {
      cout << "Smearing scaling: cjer " << cjer << endl;
      cout << "Old pt: " << jet.Pt() << "\tnew pt: " << *realpt << endl;
    }
    return Smearing::method::scaling;
  } else {
    TRandom3 generator;
    Float_t cjer = 1 + generator.Gaus(0, sigmajer) * \
      TMath::Sqrt(TMath::Max(sjer*sjer - 1, Float_t(0)));
    *realpt = jet.Pt()*cjer;
    if (debug) {
      cout << "Smearing stochastic: cjer " << cjer << endl;
      cout << "Old pt: " << jet.Pt() << "\tnew pt: " << *realpt << endl;
    }
    return Smearing::method::stochastic;
  }
}



int DataContainer::SetTreeBranches(TTree * t) {
  // Event Info
  t->SetBranchAddress("event", &event_);
  t->SetBranchAddress("run", &run_);
  t->SetBranchAddress("luminosityBlock", &lumisection_);
  // gen jets
  t->SetBranchAddress("nJet", &njet_);
  t->SetBranchAddress("Jet_pt", jet_pt_);
  t->SetBranchAddress("Jet_eta", jet_eta_);
  t->SetBranchAddress("Jet_phi", jet_phi_);
  t->SetBranchAddress("Jet_mass", jet_mass_);
  t->SetBranchAddress("Jet_jetId", jet_id_);
  t->SetBranchAddress("Jet_btagDeepB", btag_deep_);
  t->SetBranchAddress("Jet_bRegCorr", jet_b_reg_corr);
  t->SetBranchAddress("Jet_nElectrons", &jet_n_electrons);
  t->SetBranchAddress("Jet_nMuons", &jet_n_muons);
  t->SetBranchAddress("Jet_genJetIdx", jet_genjet_idx_);
  t->SetBranchAddress("GenJet_pt", genjet_pt_);
  t->SetBranchAddress("GenJet_eta", genjet_eta_);
  t->SetBranchAddress("GenJet_phi", genjet_phi_);
  t->SetBranchAddress("Jet_bRegRes", jet_b_reg_res_);
  t->SetBranchAddress("HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33", &trigger_path);
  return 0;
}



void DataContainer::CreateTree(TTree* t) {
  // Event Info
  t->Branch("event", &event_, "event/i");
  t->Branch("run", &run_, "run/l");
  t->Branch("luminosityBlock", &lumisection_, "luminosityBlock/i");
  // gen jets
  t->Branch("nJet", &njet_, "nJet/i");
  t->Branch("Jet_pt", jet_pt_, "Jet_pt/F");
  t->Branch("Jet_eta", jet_eta_, "Jet_eta/F");
  t->Branch("Jet_phi", jet_phi_, "Jet_phi/F");
  t->Branch("Jet_mass", jet_mass_, "Jet_mass/F");
  t->Branch("Jet_jetId", jet_id_, "Jet_jetId/I");
  t->Branch("Jet_btagDeepB", btag_deep_, "Jet_btagDeepB/F");
  t->Branch("Jet_bRegCorr", jet_b_reg_corr, "Jet_bRegCorr/F");
  t->Branch("Jet_nElectrons", &jet_n_electrons, "Jet_nElectrons/I");
  t->Branch("Jet_nMuons", &jet_n_muons, "Jet_nMuons/F");
  t->Branch("Jet_genJetIdx", jet_genjet_idx_, "Jet_genJetIdx/I");
  t->Branch("GenJet_pt", genjet_pt_, "GenJet_pt/F");
  t->Branch("GenJet_eta", genjet_eta_, "GenJet_eta/F");
  t->Branch("GenJet_phi", genjet_phi_, "GenJet_phi/F");
  t->Branch("Jet_bRegRes", jet_b_reg_res_, "Jet_bRegRes/F");
}



/**
 * Modifies current_jet[i] smearing the pt.
 */
void DataContainer::smear_current_data(UInt_t i) {
  TVector3 genjet;
  Float_t appo;
  genjet.SetPtEtaPhi(genjet_pt_[corrected_jet[i].Idx()], \
                     genjet_eta_[corrected_jet[i].Idx()], \
                     genjet_phi_[corrected_jet[i].Idx()]);
  smearing(corrected_jet[i].Vect(), genjet, rcone, &appo);
  corrected_jet[i].SetPtEtaPhiM(appo, corrected_jet[i].Eta(),\
                              corrected_jet[i].Phi(), corrected_jet[i].M());
  if (debug) {
    cout << "smeared value: "  << appo << endl;
  }
}

void DataContainer::apply_all_corrections(CorrectionLevel lev) {
  for (unsigned int i = 0; i < njet_; i++) {
    initialize_current_data(i);
  }
  switch (lev) {
  case CorrectionLevel::nothing: break;
  case CorrectionLevel::only_smearing:
    for (unsigned int i = 0; i < njet_; i++) {
      smear_current_data(i);
    }
    sort_by_Pt();
    break;
  case CorrectionLevel::smearing_btag:
    for (unsigned int i = 0; i < njet_; i++) {
      smear_current_data(i);
    }
    sort_by_Pt();
    apply_btag_sf();
    sort_by_Pt();
    break;
  case CorrectionLevel::smearing_btag_regression:
    for (unsigned int i = 0; i < njet_; i++) {
      smear_current_data(i);
    }
    sort_by_Pt();
    apply_btag_sf();
    sort_by_Pt();
    for (unsigned int i = 0; i < njet_; i++) {
      apply_jet_reg_sf(i);
    }
    sort_by_Pt();
    break;
  case CorrectionLevel::smearing_btag_regression_fsr:
    for (unsigned int i = 0; i < njet_; i++) {
      smear_current_data(i);
    }
    sort_by_Pt();
    apply_btag_sf();
    sort_by_Pt();
    for (unsigned int i = 0; i < njet_; i++) {
      apply_jet_reg_sf(i);
    }
    sort_by_Pt();
    apply_fsr_correction();
    sort_by_Pt();
    break;        
  }  
}


void DataContainer::initialize_current_data(UInt_t i) {
  TLorentzVector appo;
  appo.SetPtEtaPhiM(jet_pt_[i], jet_eta_[i], jet_phi_[i], jet_mass_[i]);
  Jet appo1(appo, i);
  corrected_jet.push_back(appo1);
}

/**
 * Deprecated
 */
bool DataContainer::get_real_pt(Int_t index, Float_t *pt) {
  if (jet_b_reg_corr[index] != 0) {
    *pt = jet_b_reg_corr[index] * jet_pt_[index];
    return true;
  } else {
    *pt = jet_pt_[index];
    return false;
  }
}



void DataContainer::fill_histogram(TTree* tree, std::vector<TH1F*> plots, TTree* out_tree) {
  ULong64_t how_many;
  if (debug) {
    how_many = 1000;
  } else {
    how_many = tree->GetEntries();
  }

  for (ULong64_t i = 0; i < how_many; i++) {
    tree->GetEntry(i);
    if (!trigger_path) {
      continue;
    }
    weigth = 1;
    corrected_jet.clear();
    if (debug) {
      cout << "\033[0;33m\n\nNew event\033[0m" << endl;
      cout << "Run = " << run_ << " | Lumi Section = "          \
           << lumisection_ << " | Event = " << event_ << endl;
      cout << " number of gen jets from njet = " << njet_ << endl;
    }
    // If we have less than 3 jets the event is not useful
    if (debug) {
      cout << "njets: " << njet_ << endl;
    }
    if (njet_ < 3) {
      continue;
    }

    
    // Apply first cut on momentum and eta without checking
    // real identity
    bool prosegui = true;    
    apply_all_corrections(correction_level);
    for (unsigned int i = 0; i < pt_cut.size(); i++) {
      // Correction on pt
      if (debug) {
        cout << "pt before: " << jet_pt_[corrected_jet[i].Idx()] << "\neta: " <<    \
          jet_eta_[corrected_jet[i].Idx()] << "\tcurrent_jet_pt: " << corrected_jet[i].Pt() \
             << "\tcurrent_jet_eta_:" << corrected_jet[i].Eta() <<      \
          "\nid: " << jet_id_[corrected_jet[i].Idx()] <<                \
          "\tjet_b_reg_correction: " << jet_b_reg_corr[i] << "\tjetid: " \
             << jet_id_[corrected_jet[i].Idx()] << "\tktight: " << kTight << "\tbitwise: " \
             << (jet_id_[corrected_jet[i].Idx()] & kTight) << endl;
      }
      if ((jet_id_[corrected_jet[i].Idx()] & kTight) == 0) {
        prosegui = false; break;
      }
      if (TMath::Abs(corrected_jet[i].Eta()) > eta_cut[i]) {
        prosegui = false; break;
      }
      if (corrected_jet[i].Pt() < pt_cut[i]) {
        prosegui = false; break;
      }
    }
    if (!prosegui) {
      continue;
    }

    // Now check btags
    unsigned int btaggati = 0;
    vector<UInt_t> bjet_indexes;
    if (debug) {
      cout << "Starting btagging check" << endl;
    }
    for (unsigned int i = 0; i < njet_ && btaggati < 3; i++) {
      if (debug) {
        cout << "btag_deep: " << btag_deep_[corrected_jet[i].Idx()] << "\tpt smeared corrected: " << \
          corrected_jet[i].Pt()                                            \
             << "\teta: " << jet_eta_[corrected_jet[i].Idx()] << "\tcurrent_jet_eta_: " << \
          corrected_jet[i].Eta() << "\tjet_b_regcor: " <<                 \
          jet_b_reg_corr[corrected_jet[i].Idx()] << endl;
      }
      bool success = true;
      if (btag_deep_[corrected_jet[i].Idx()] > deepcsv_cut) {
        if (i >= 3) {
          if (TMath::Abs(corrected_jet[i].Eta()) > eta_cut[btaggati]) {
            success = false;
          } else {
            if (corrected_jet[i].Pt() < pt_cut[btaggati])
              success = false;
          }
        }
        if (success) {
          btaggati++;
          bjet_indexes.push_back(i);
        }
      }
      if (corrected_jet[i].Pt() < pt_cut[pt_cut.size() - 1]) {
        break;
      }
    }
    if (debug) {
      cout << "btagged: " << btaggati << endl;
    }
    if (btaggati < 3) {
      continue;
    }

    
    // Now i check if the couples jets are distant enough
    bool success = true;
    for (unsigned int i = 0; i < 3; i++) {
      for (unsigned int j = i + 1; j < 3; j++) {
        unsigned int a = bjet_indexes[i];
        unsigned int b = bjet_indexes[j];
        if (!check_distance_enough(corrected_jet[a], corrected_jet[b])) {
          success = false;
          break;
        }
      }
    }
    if (!success) { continue; }
    if (TMath::Abs(corrected_jet[bjet_indexes[0]].Eta() - \
                   corrected_jet[bjet_indexes[1]].Eta()) > 1.5) {
      continue;
    }
    TLorentzVector appo = corrected_jet[bjet_indexes[0]] - corrected_jet[bjet_indexes[1]];
    cout << "\033[0;32mFilling with m = " << -appo.M() << \
      " and pt = " << corrected_jet[bjet_indexes[0]].Pt() << "\033[0m" << endl;
    out_tree->Fill();
    if (debug) {
      cout << "Leptons in the event: electron " << \
        jet_n_electrons << "\tmuons: " << jet_n_muons << endl;
    }
    
    if (jet_n_muons != 0 || jet_n_electrons != 0) {
      plots[0]->Fill(-appo.M(), weigth);
      plots[2]->Fill(corrected_jet[bjet_indexes[0]].Pt(), weigth);
      plots[4]->Fill(jet_b_reg_res_[0], weigth);
    } else {
      plots[1]->Fill(-appo.M(), weigth);
      plots[3]->Fill(corrected_jet[bjet_indexes[0]].Pt(), weigth);
      plots[5]->Fill(jet_b_reg_res_[0], weigth);
    }
  }
}

void DataContainer::apply_btag_sf() {
  weigth = 1;
  if (debug) {
    cout << "Here weigth should be 1: "  << weigth << endl;
    if (weigth != 1.) {
      cerr << "Error. weigth is not 1. Something wrong happened" << endl;
    }
  }
  
  bool tagged[njet_];
  int how_many = njet_;
  for (unsigned int i = 0; i < njet_; i++) {
    tagged[i] = (btag_deep_[i] > deepcsv_cut);
    Float_t sf = reader.\
      eval_auto_bounds("central", BTagEntry::FLAV_B, corrected_jet[i].Pt(), \
                       corrected_jet[i].Eta());
    sf = 1; // this is because the current file is not the right one
    if (debug) {
      cout << "Retrieved scale factor: " << sf << endl;
    }
    if (tagged[i]) {
      how_many--;
      weigth *= (sf);
      if (debug) {
        cout << "weight: (calculating) " << weigth << " factor1 " << sf << endl;
      }
    } else {
      weigth *= (1. - b_tagging_efficiency*sf)/(1. - b_tagging_efficiency);
      if (debug) {
        cout << "weight: (calculating) " << weigth << " factor2 " << (1. - b_tagging_efficiency*sf)/(1. - b_tagging_efficiency) << endl;
      }
    }
  }
  if (debug) {
    cout << "Weigth of this event: " << weigth << endl; 
  }           
  return;
}

void DataContainer::apply_jet_reg_sf(UInt_t i) {
  if (jet_b_reg_corr[i] != 0) {
    corrected_jet[i].SetPtEtaPhiM(corrected_jet[i].Pt()*jet_b_reg_corr[i],\
                                  corrected_jet[i].Eta(), corrected_jet[i].Phi(),\
                                  corrected_jet[i].M());
  } else {
    cerr << "Something nasty is happening. jet_b_reg_correction is 0." << endl;
  }
}

void DataContainer::apply_fsr_correction() {
  std::vector<std::tuple<Jet, UInt_t>> bjets;
  std::vector<Jet> softjets;
  if (debug) {
    cout << "FSR\n\n" << endl;
  }
  if (njet_ < 4) {
    return;
  }
  for (UInt_t i = 0; i < njet_; i++) {
    if (btag_deep_[corrected_jet[i].Idx()] > deepcsv_cut){
      bjets.push_back(std::make_tuple(corrected_jet[i], i));
    } else {
      softjets.push_back(corrected_jet[i]);
    }
  }
  if (softjets.empty()) {
    return;
  }
  for (auto it : bjets) {
    auto appo = std::get<0>(it);
    auto i = std::get<1>(it);
    vector<Jet>::iterator best = std::min_element(\
       softjets.begin(), softjets.end(), [&appo](const Jet& a, const Jet& b) {
         return appo.DeltaR(a) < appo.DeltaR(b);
       });
    if (best != softjets.end() && best->DeltaR(appo) > soft_jet_distance) { continue; }
    
    if (debug) {
      cout << "Correcting for fsr. Added something.\n" <<               \
        "PtBefore: " << appo.Pt() << "\tPt after: " << (appo + *best).Pt() << \
        "\tEta before: " << appo.Eta() << "\tEta after: " << (appo + *best).Eta() << endl;
      cout << "Best candidate had pt: " << best->Pt() << " R distance: " \
           << appo.DeltaR(*best) << endl;
    }
    appo += *best;
    corrected_jet[i].SetPtEtaPhiM(appo.Pt(), appo.Eta(), appo.Phi(), appo.M());
  }
  return;
}


int main(int argc, char* argv[]) {
  TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
  TFile nano(file.c_str(), "old");
  if (argc != 2) {
    cerr << "\033[0;31mPlease give the name of the file where you want to" <<
      "put the output as an argument. Aborting.\033[0m" << endl;
    return -1;
  }
  if (nano.IsZombie()) {
    cerr << "Error opening input file! Aborting." << endl;
    return -2;
  }
  TFile output(argv[1], "create");
  if (output.IsZombie()) {
    cerr << "Error opening output file " << argv[1] << ". Make sure that "
      "the file does not exist. Aborting." << endl;
    return -3;
  }
  TTree* tree = reinterpret_cast<TTree*>(nano.Get("Events"));
  TTree output_tree("output_tree", "Histograms and data for H->bb decay");
  DataContainer container;
  container.SetTreeBranches(tree);
  container.CreateTree(&output_tree);
  TH1F mass_histo_chromo("mass_histo_chromo", \
                      "Invariant mass of bb jets", 50, 0, 800);
  TH1F mass_histo_lepton("mass_histo_lepton", \
                            "Invariant mass of bb jets", 50, 0, 800);
  TH1F pt_spectrum_chromo("pt_spectrum_chromo", \
                          "Pt spectrum for chromo events", 50, 0, 800);
  TH1F pt_spectrum_lepton("pt_spectrum_lepton", \
                          "Pt spectrum for lepton events", 50, 0, 800);
  TH1F reg_res_chromo("reg_res_chromo", "RegRes for non lepton events", \
                      50, 0, 1);
  TH1F reg_res_lepton("reg_res_lepton", "RegRes for lepton events", \
                      50, 0, 1);
  std::vector<TH1F*> histos;
  histos.push_back(&mass_histo_chromo);
  histos.push_back(&mass_histo_lepton);
  histos.push_back(&pt_spectrum_chromo);
  histos.push_back(&pt_spectrum_lepton);
  histos.push_back(&reg_res_chromo);
  histos.push_back(&reg_res_lepton);
  container.fill_histogram(tree, histos, &output_tree);
  output.cd();
  cout << "Entries in chromo histo: " << \
    mass_histo_chromo.GetEntries() << endl;
  cout << "Entries in lepton histo: " << \
    mass_histo_lepton.GetEntries() << endl;
  mass_histo_chromo.SetLineColor(kGreen);
  mass_histo_lepton.SetLineColor(kRed);
  TH1F mass_histo_sum(mass_histo_chromo);
  mass_histo_sum.Add(&mass_histo_lepton, 1.);
  mass_histo_sum.SetLineColor(kBlue);

  for (auto it : histos) {
    it->Write();
  }
  mass_histo_sum.Write();
  output_tree.Write();
  nano.Close();
  return 0;
}
