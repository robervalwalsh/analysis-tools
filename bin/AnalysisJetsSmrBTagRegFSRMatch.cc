/**
 * @author Fabio Zoratti <fabio.zoratti96@gmail.com> <fabio.zoratti@desy.de> <fabio.zoratti@cern.ch>
 * @date 30/7/2018
 * @file AnalysisJetsSmrBTagRegFSRMatch.cc
 * First apply smearing to data, then apply scale factors to correct MC for btag, then uses regression algorithm to correct energy and then corrects for final state radiation.
 * At the end I check the number of correct higgs matches for the first 2 jets.
 */

/**
 * DOCS: select the value for the global variable correction_level and build.
 * Also set the other global variable debug to false.
 */




#include <string>
#include <iostream>
#include <fstream>
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
#include "Analysis/Tools/interface/BTagCalibrationStandalone.h"



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

enum class Match {
  higgsd,
  spectator,
  other,
  no_match
};



const Float_t rcone(0.4);
const unsigned int kLen = 100;
const Float_t deepcsv_cut = 0.4941;
const Float_t b_tagging_efficiency = 0.99;
const Int_t kTight = 2;
const array<Float_t, 3> pt_cut{{100, 100, 40}};
const array<Float_t, 3> eta_cut{{2.2, 2.2, 2.2}};
const bool debug = false;
// The following line includes 3 variables:
// const CorrectionLevel correction_level = something;
// const bool apply_trigger_cut = something;
// I used this method to script more easily when I have to do all the plots
#include "Parameters.cc"
// const string file("something");
// const UInt_t highx = 500;
// const UInt_t bins = 50;
#include "_tmp/mass_point.cc"
const Float_t soft_jet_distance = 0.8;
const Int_t pdgBId = 5;
const Int_t heavyHiggs = 36;
const Float_t gen_part_deltarcut = 0.3;



bool check_distance_enough(const TLorentzVector &a, const TLorentzVector &b) {
  if (debug) {
    cout << "Delta R: " << a.DeltaR(b) << endl;
    cout << "Eta bw 4vec: " << TMath::Abs(a.Eta() - b.Eta()) << endl;
  }
  return a.DeltaR(b) > 1;
}


                                              
std::ostream& operator<<(std::ostream& os, const Match& m) {
  switch (m) {
  case Match::higgsd:
    return os << "higgs daughter";
    break;
  case Match::spectator:
    return os << "spectator";
    break;
  case Match::other:
    return os << "other";
    break;
  case Match::no_match:
    return os << "no match";
    break;
  default:
    throw std::runtime_error("Trying to print a non declared kind of Match");
    return os;
    break;
  }
  return os;
}




class Jet : public TLorentzVector {
 public:
  Jet(const TLorentzVector& a, UInt_t i) : TLorentzVector(a) {
    orig_index_ = i;
  }
  UInt_t Idx() const { return orig_index_; }
  Match GetMatch() const { return match_; }
  void SetMatch(const Match m) { match_ = m; }
  
 private:
  Match match_;
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

  /**
   * @brief Looks for all jets with btag_deep > deepcsv_cut and checks if there are
   * very close light flavour jets. If yes, it adds the closest TLorentzVector of the
   * soft jet to the original one. (which is saved in corrected_jet[i])
   */
  void apply_fsr_correction();
  
  /**
   * @brief Reads lev and decides what corrections have to be applied on pt
   */
  void apply_all_corrections(CorrectionLevel lev);

  /**
   * @brief Matches GenPart from GenJet from Jet to check whether if
   * the Jet is a Higgs daughter or not. It checks the idx corrected_jet
   * and saves the result in the corrected_jet
   */
  void match_jet_with_part(Jet& jet);


  /**
   * @brief Recursive function that looks at the origin of the genpart i.
   * It will stop when it encounters a higgs or something that is not a b-flavour jet
   */
  Match match_genpart(UInt_t i);

  
 private:
  JME::JetResolution resolution;
  JME::JetResolutionScaleFactor resolution_sf;
  BTagCalibration calib;
  BTagCalibrationReader reader;
  vector<Jet> corrected_jet;
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
  UInt_t n_gen_jet_;
  Float_t genjet_pt_[kLen];
  Float_t genjet_eta_[kLen];
  Float_t genjet_phi_[kLen];
  Bool_t trigger_path;
  UInt_t ngenpart_;
  Int_t genjet_parton_flavour_[kLen];
  Int_t genpart_genpart_idxmother_[kLen];
  Float_t genpart_pt_[kLen];
  Float_t genpart_eta_[kLen];
  Float_t genpart_phi_[kLen];
  Float_t genpart_mass_[kLen];
  Int_t genpart_pdgid_[kLen];
  Float_t jet_b_reg_res_[kLen];

  // Extra values to put in the new tree
  UInt_t matches_;
  Float_t mass_;
  Float_t new_pt_[kLen];
  Float_t new_eta_[kLen];
  Float_t new_phi_[kLen];
  Float_t new_mass_[kLen];
  /**
   * This variable is used to see if the corrected_jets are not in the same
   * order as the original ones. If this array is for example [1, 3, 2] means
   * that the corrected jet #1 was in first position, the corrected_jet #2 was
   * in third position and the corrected_jet #3 was in second position.
   */
  UInt_t reordering_[kLen];
};

/**
 * @danger I am using the old data table because I did not find the new one.
 */
DataContainer::DataContainer() : resolution("tables/Fall17_25nsV1_MC_PtResolution_AK4PFchs.txt"), \
                                 resolution_sf("tables/Fall17_25nsV1_MC_SF_AK4PFchs.txt") , \
                                 calib("deepcsv", "tables/DeepCSV_94XSF_V3_B_F.csv"), \
                                 reader(&calib, BTagEntry::OP_MEDIUM) {}


void DataContainer::sort_by_Pt() {
  std::sort(corrected_jet.begin(), corrected_jet.end(), \
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
  t->SetBranchAddress("nGenJet", &n_gen_jet_);
  t->SetBranchAddress("GenJet_pt", genjet_pt_);
  t->SetBranchAddress("GenJet_eta", genjet_eta_);
  t->SetBranchAddress("GenJet_phi", genjet_phi_);
  t->SetBranchAddress("Jet_bRegRes", jet_b_reg_res_);
  t->SetBranchAddress("HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33", &trigger_path);
  t->SetBranchAddress("nGenPart", &ngenpart_);
  t->SetBranchAddress("GenJet_partonFlavour", genjet_parton_flavour_);
  t->SetBranchAddress("GenPart_genPartIdxMother", genpart_genpart_idxmother_);
  t->SetBranchAddress("GenPart_pt", genpart_pt_);
  t->SetBranchAddress("GenPart_eta", genpart_eta_);
  t->SetBranchAddress("GenPart_phi", genpart_phi_);
  t->SetBranchAddress("GenPart_mass", genpart_mass_);
  t->SetBranchAddress("GenPart_pdgId", genpart_pdgid_);
  return 0;
}



void DataContainer::CreateTree(TTree* t) {
  // Event Info
  t->Branch("event", &event_, "event/i");
  t->Branch("run", &run_, "run/l");
  t->Branch("luminosityBlock", &lumisection_, "luminosityBlock/i");
  // gen jets
  t->Branch("nJet", &njet_, "nJet/i");
  t->Branch("Jet_pt", jet_pt_, "Jet_pt[nJet]/F");
  t->Branch("Jet_eta", jet_eta_, "Jet_eta[nJet]/F");
  t->Branch("Jet_phi", jet_phi_, "Jet_phi[nJet]/F");
  t->Branch("Jet_mass", jet_mass_, "Jet_mass[nJet]/F");
  t->Branch("Jet_jetId", jet_id_, "Jet_jetId[nJet]/I");
  t->Branch("Jet_btagDeepB", btag_deep_, "Jet_btagDeepB[nJet]/F");
  t->Branch("Jet_bRegCorr", jet_b_reg_corr, "Jet_bRegCorr[nJet]/F");
  t->Branch("Jet_nElectrons", &jet_n_electrons, "Jet_nElectrons/I");
  t->Branch("Jet_nMuons", &jet_n_muons, "Jet_nMuons/F");
  t->Branch("Jet_genJetIdx", jet_genjet_idx_, "Jet_genJetIdx[nJet]/I");
  t->Branch("nGenJet", &n_gen_jet_, "nGenJet/i");
  t->Branch("GenJet_pt", genjet_pt_, "GenJet_pt[nGenJet]/F");
  t->Branch("GenJet_eta", genjet_eta_, "GenJet_eta[nGenJet]/F");
  t->Branch("GenJet_phi", genjet_phi_, "GenJet_phi[nGenJet]/F");
  t->Branch("Jet_bRegRes", jet_b_reg_res_, "Jet_bRegRes[nJet]/F");
  t->Branch("HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33", &trigger_path, \
            "HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33/O");
  t->Branch("nGenPart", &ngenpart_, "nGenPart/i");
  t->Branch("GenJet_partonFlavour", genjet_parton_flavour_, "GenJet_partonFlavour[nGenJet]/I");
  t->Branch("GenPart_genPartIdxMother", genpart_genpart_idxmother_, "GenPart_genPartIdxMother[nGenPart]/i");
  t->Branch("GenPart_pt", genpart_pt_, "GenPart_pt[nGenPart]/F");
  t->Branch("GenPart_eta", genpart_eta_, "GenPart_eta[nGenPart]/F");
  t->Branch("GenPart_phi", genpart_phi_, "GenPart_phi[nGenPart]/F");
  t->Branch("GenPart_mass", genpart_mass_, "GenPart_mass[nGenPart]/F");
  t->Branch("GenPart_pdgId", genpart_pdgid_, "GenPart_pdgId[nGenPart]/F");
  // new data
  t->Branch("nMatches", &matches_, "nMatches/i");
  t->Branch("Mass", &mass_, "Mass/F");
  t->Branch("Jet_newPt", new_pt_, "Jet_newPt[nJet]/F");
  t->Branch("Jet_newEta", new_eta_, "Jet_newEta[nJet]/F");
  t->Branch("Jet_newPhi", new_phi_, "Jet_newPhi[nJet]/F");
  t->Branch("Jet_newMass", new_mass_, "Jet_newMass[nJet]/F");
  t->Branch("Jet_newOriginalOrder", reordering_, "Jet_newOriginalOrder[nJet]/i");
}



/**
 * Modifies current_jet[i] smearing the pt.
 */
void DataContainer::smear_current_data(UInt_t i) {
  TVector3 genjet;
  Float_t appo;
  UInt_t j = corrected_jet[i].Idx();
  genjet.SetPtEtaPhi(genjet_pt_[j], genjet_eta_[j], genjet_phi_[j]);
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


/* 
 * Loops over genpart with id equal to b or bbar and select the closest one
 * Then reconstructs the original particle
 */
void DataContainer::match_jet_with_part(Jet& jet) {
  unsigned int i = jet.Idx();
  if (TMath::Abs(genjet_parton_flavour_[i]) != pdgBId) {
    jet.SetMatch(Match::other);
    if (debug) {
      cout << "Current genjet is not of the right flavour." << \
        " A bit strange (not the flavour). Its flavour is " << \
        genjet_parton_flavour_[i] << endl;
    }
    return;
  }
  Float_t min_value = 1.;
  Float_t min_index = -1;
  for (UInt_t j = 0; j < ngenpart_; j++) {
    if (TMath::Abs(genpart_pdgid_[j]) != pdgBId) {
      continue;
    }
    TLorentzVector appo;
    appo.SetPtEtaPhiM(genpart_pt_[j], genpart_eta_[j], \
                      genpart_phi_[j], genpart_mass_[i]);
    Float_t dR = appo.DeltaR(jet);
    if (dR < min_value) {
      min_value = dR;
      min_index = j;
    }
  }
  if (min_value < gen_part_deltarcut) {
    if (debug) {
      cout << "Found matching genpart. Now I check its origin" << endl;
    }
    Match appo = match_genpart(min_index);
    jet.SetMatch(appo);
    if (debug) {
      cout << "Matched origin of genpart: this is origin:" << appo << endl;
    }
  } else {
    if (debug) {
      cout << "Not found any matching genpart." << endl;
    }
    jet.SetMatch(Match::no_match);
  }
}


Match DataContainer::match_genpart(UInt_t i) {
  if (debug) {
    cout << "Mother index of genpart: " << genpart_genpart_idxmother_[i] \
         << "\t current pdgid: " << genpart_pdgid_[i] << endl;
  }
  if (genpart_genpart_idxmother_[i] == -1) {
    if (TMath::Abs(genpart_pdgid_[i]) == pdgBId) {
      return Match::spectator;
    } else if (TMath::Abs(genpart_pdgid_[i]) == heavyHiggs) {
      return Match::higgsd;
    } else {
      return Match::other;
    }
  } else {
    if (TMath::Abs(genpart_pdgid_[i]) == pdgBId){
      return match_genpart(genpart_genpart_idxmother_[i]);
    } else if (TMath::Abs(genpart_pdgid_[i]) == heavyHiggs) {
      return Match::higgsd;
    } else {
      return Match::no_match;
    }
  }
}



void DataContainer::fill_histogram(TTree* tree, \
                                   std::vector<TH1F*> plots, TTree* out_tree) {
  ULong64_t how_many;
  if (debug) {
    how_many = 1000;
  } else {
    how_many = tree->GetEntries();
  }

  for (ULong64_t event = 0; event < how_many; event++) {
    tree->GetEntry(event);
    if (apply_trigger_cut) {
      if (!trigger_path) {
        continue;
      }
    }
    weigth = 1;
    corrected_jet.clear();
    if (debug) {
      cout << "\033[0;33m\n\nNew event\033[0m" << endl;
      cout << "Run = " << run_ << " | Lumi Section = "          \
           << lumisection_ << " | Event = " << event_ << endl;
      cout << " number of gen jets from njet = " << njet_ << endl;
      // If we have less than 3 jets the event is not useful
      cout << "njets: " << njet_ << endl;
    }
    if (njet_ < 3) {
      continue;
    }
    
    apply_all_corrections(correction_level);
    
    // Now check btags
    unsigned int btaggati = 0;
    vector<Jet> bjets;
    if (debug) {
      cout << "Starting btagging check" << endl;
    }
    for (auto it = corrected_jet.begin(); it < corrected_jet.end() && \
           btaggati < 3; it++) {
      if (debug) {
        cout << "pt before: " << jet_pt_[it->Idx()] << "\neta: " <<     \
          jet_eta_[it->Idx()] << "\tcurrent_jet_pt: " << it->Pt()       \
             << "\tcurrent_jet_eta_:" << it->Eta() << "\nid: " << jet_id_[it->Idx()] << \
          "\tjet_b_reg_correction: " << jet_b_reg_corr[it->Idx()] << "\tjetid: " \
             << jet_id_[it->Idx()] << "\tktight: " << kTight << "\tbitwise: " \
             << (jet_id_[it->Idx()] & kTight) << endl;
        cout << "btag_deep: " << btag_deep_[it->Idx()] <<               \
          "\tpt smeared corrected: " << it->Pt() << "\teta: " <<        \
          jet_eta_[it->Idx()] << "\tcurrent_jet_eta_: " <<              \
          it->Eta() << "\tjet_b_regcor: " << jet_b_reg_corr[it->Idx()] << endl;
      }
      // First i check if we are already with a too small pt.
      if (it->Pt() < pt_cut[pt_cut.size() - 1]) {
        break;
      }
      if ((jet_id_[it->Idx()] & kTight) == 0) {
        continue;
      }
      // Look at pt only if it is btagged
      if (btag_deep_[it->Idx()] > deepcsv_cut) {
        // cuts on eta and pt
        if (TMath::Abs(it->Eta()) > eta_cut[btaggati]) {
          continue;
        } else {
          if (it->Pt() < pt_cut[btaggati])
            continue;
        }
        btaggati++;
        bjets.push_back(*it);
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
    for (auto it = bjets.begin(); it != bjets.end(); it++) {
      for (auto jt = it + 1; jt != bjets.end(); jt++) {
        if (!check_distance_enough(*it, *jt)) {
          success = false;
          break;
        }
      }
    }
    if (!success) { continue; }
    if (TMath::Abs(bjets[0].Eta() - bjets[1].Eta()) > 1.5) {
      continue;
    }
    unsigned int matched = 0;
    for (unsigned int i = 0; i < 2; i++) {
      match_jet_with_part(bjets[i]);
      if (debug) {
        cout << "Jet matched as " << bjets[i].GetMatch() << endl;
      }
      if (bjets[i].GetMatch() == Match::higgsd) {
        matched++;
      }
    }
    if (debug) {
      cout << "Matched " << matched << " jets" << endl;
    }

    mass_ = -(bjets[0] - bjets[1]).M();
    matches_ = matched;
    for (unsigned int k = 0; k < corrected_jet.size(); k++) {
      new_pt_[k] = corrected_jet[k].Pt();
      new_eta_[k] = corrected_jet[k].Eta();
      new_phi_[k] = corrected_jet[k].Phi();
      new_mass_[k] = corrected_jet[k].M();
      reordering_[k] = corrected_jet[k].Idx();
    }
    cout << "\033[0;32mFilling with m = " << mass_ << \
      " and pt = " << bjets[0].Pt() << "\033[0m" << endl;
    out_tree->Fill();
    if (debug) {
      cout << "Leptons in the event: electron " << \
        jet_n_electrons << "\tmuons: " << jet_n_muons << endl;
    }
    
    if (jet_n_muons != 0 || jet_n_electrons != 0) {
      plots[matched + 3]->Fill(mass_, weigth);      
      plots[7]->Fill(bjets[0].Pt(), weigth);
      plots[9]->Fill(jet_b_reg_res_[bjets[0].Idx()], weigth);
    } else {
      plots[matched]->Fill(mass_, weigth);
      plots[6]->Fill(bjets[0].Pt(), weigth);
      plots[8]->Fill(jet_b_reg_res_[bjets[0].Idx()], weigth);
    }
  }
}


void DataContainer::apply_btag_sf() {
  if (debug) {
    cout << "Using scale factors for btagging" << endl;
  }
  for (auto it = corrected_jet.begin(); it != corrected_jet.end(); it++) {
    Double_t sf = reader.eval(BTagEntry::FLAV_B, it->Eta(), it->Pt(), 0);
    if (debug) {
      cout << "sf: " << sf << endl;
    }
    it->SetPtEtaPhiM((it->Pt())*sf, it->Eta(), it->Phi(), it->M());
  }
}


void DataContainer::apply_jet_reg_sf(UInt_t i) {
  if (jet_b_reg_corr[corrected_jet[i].Idx()] != 0) {
    corrected_jet[i].SetPtEtaPhiM(corrected_jet[i].Pt()*jet_b_reg_corr[corrected_jet[i].Idx()], \
                                  corrected_jet[i].Eta(), corrected_jet[i].Phi(), \
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
    if (btag_deep_[corrected_jet[i].Idx()] > deepcsv_cut) {
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
  TH1F mass_histo_chromo_0_match("mass_histo_chromo_0_match", \
                      "Invariant mass of bb jets", bins, 0, highx);
  TH1F mass_histo_lepton_0_match("mass_histo_lepton_0_match", \
                            "Invariant mass of bb jets", bins, 0, highx);
  TH1F mass_histo_chromo_1_match("mass_histo_chromo_1_match",                           \
                         "Invariant mass of bb jets", bins, 0, highx);
  TH1F mass_histo_lepton_1_match("mass_histo_lepton_1_match",                           \
                            "Invariant mass of bb jets", bins, 0, highx);
  TH1F mass_histo_chromo_2_match("mass_histo_chromo_2_match",           \
                                 "Invariant mass of bb jets", bins, 0, highx);
  TH1F mass_histo_lepton_2_match("mass_histo_lepton_2_match",           \
                         "Invariant mass of bb jets", bins, 0, highx);
  TH1F pt_spectrum_chromo("pt_spectrum_chromo", \
                          "Pt spectrum for chromo events", bins, 0, highx);
  TH1F pt_spectrum_lepton("pt_spectrum_lepton", \
                          "Pt spectrum for lepton events", bins, 0, highx);
  TH1F reg_res_chromo("reg_res_chromo", "RegRes for non lepton events", \
                      bins, 0, 1);
  TH1F reg_res_lepton("reg_res_lepton", "RegRes for lepton events", \
                      bins, 0, 1);
  std::vector<TH1F*> histos;
  histos.push_back(&mass_histo_chromo_0_match);
  histos.push_back(&mass_histo_chromo_1_match);
  histos.push_back(&mass_histo_chromo_2_match);
  histos.push_back(&mass_histo_lepton_0_match);
  histos.push_back(&mass_histo_lepton_1_match);
  histos.push_back(&mass_histo_lepton_2_match);
  histos.push_back(&pt_spectrum_chromo);
  histos.push_back(&pt_spectrum_lepton);
  histos.push_back(&reg_res_chromo);
  histos.push_back(&reg_res_lepton);

  container.fill_histogram(tree, histos, &output_tree);
  output.cd();
  mass_histo_chromo_0_match.SetLineColor(kPink);
  mass_histo_chromo_0_match.SetLineColor(kBlue);
  mass_histo_chromo_0_match.SetLineColor(kGreen);
  mass_histo_lepton_0_match.SetLineColor(kRed);
  mass_histo_lepton_0_match.SetLineColor(kOrange);
  mass_histo_lepton_0_match.SetLineColor(kYellow);
  TH1F mass_histo_sum(mass_histo_chromo_0_match);
  mass_histo_sum.Add(&mass_histo_chromo_1_match, 1.);
  mass_histo_sum.Add(&mass_histo_chromo_2_match, 1.);
  mass_histo_sum.Add(&mass_histo_lepton_0_match, 1.);
  mass_histo_sum.Add(&mass_histo_lepton_1_match, 1.);
  mass_histo_sum.Add(&mass_histo_lepton_2_match, 1.);
  mass_histo_sum.SetLineColor(kBlack);
  cout << "Entries in total histo: " <<        \
    mass_histo_sum.GetEntries() << endl;
  
  for (auto it : histos) {
    it->Write();
  }
  mass_histo_sum.Write();
  output_tree.Write();
  nano.Close();
  return 0;
}
