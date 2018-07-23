/**
 *
 */

#include <string>
#include <iostream>
#include <vector>
#include <array>

#include "TTree.h"
#include "TFile.h"
#include "TH1.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TCanvas.h"


using std::cout;
using std::string;
using std::endl;
using std::array;
using std::cerr;
using std::vector;


const string file("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-350_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180712_150216/0000/nano_1.root");
const unsigned int kLen = 100;
const Float_t deepcsv_cut = 0.4941;
const Int_t kTight = 2;
const array<Float_t, 3> pt_cut{{100, 100, 40}};
const array<Float_t, 3> eta_cut{{2.2, 2.2, 2.2}};
const bool debug = false;



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


int SetTreeBranches(TTree * t) {
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
  return 0;
}


bool check_distance_enough(const TLorentzVector &a, const TLorentzVector &b) {
  if (debug) {
    cout << "Delta R: " << a.DeltaR(b) << endl;
    cout << "Eta bw 4vec: " << TMath::Abs(a.Eta() - b.Eta()) << endl;
  }
  return a.DeltaR(b) > 1;
}


bool get_real_pt(Int_t index, Float_t *pt) {
  if (jet_b_reg_corr[index] != 0) {
    *pt = jet_b_reg_corr[index] * jet_pt_[index];
    return true;
  } else {
    *pt = jet_b_reg_corr[index];
    return false;
  }
}

void fill_histogram(TTree* tree, TH1F* plot1, \
                    TH1F* plot2) {
  ULong64_t how_many;
  if (debug) {
    how_many = 100;
  } else {
    how_many = tree->GetEntries();
  }

  for (ULong64_t i = 0; i < how_many; i++) {
    tree->GetEntry(i);
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
    for (unsigned int i = 0; i < pt_cut.size(); i++) {
      if (debug) {
        cout << "pt: " << jet_pt_[i] << "\neta: " << \
          jet_eta_[i] << "\nid: " << jet_id_[i] << \
          "\tcorrection: " << jet_b_reg_corr[i] << endl;
      }
      Float_t real_pt;
      get_real_pt(i, &real_pt);
      if (real_pt < pt_cut[i]) {
        prosegui = false; break;
      }
      if (TMath::Abs(jet_eta_[i]) > eta_cut[i]) {
        prosegui = false; break;
      }
      if (debug) {
      cout << "jetid: " << jet_id_[i] << "\tktight: "  \
           << kTight << "\tbitwise: " << (jet_id_[i] & kTight) \
           << endl;
      }
      if ((jet_id_[i] & kTight) == 0) {
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
        cout << "btag_deep: " << btag_deep_[i] << "\tpt: " << jet_pt_[i] \
             << "\teta: " << jet_eta_[i] << "\tjet_b_regcor: " << \
          jet_b_reg_corr[i] << endl;
      }
      Float_t real_pt;
      get_real_pt(i, &real_pt);
      if (btag_deep_[i] > deepcsv_cut && real_pt >                \
          pt_cut[btaggati] && TMath::Abs(jet_eta_[i]) < eta_cut[btaggati]) {
        btaggati++;
        bjet_indexes.push_back(i);
      } else {
        if (real_pt < pt_cut[pt_cut.size() - 1]) {
          break;
        }
      }
    }
    if (debug) {
      cout << "btagged: " << btaggati << endl;
    }
    if (btaggati < 3) {
      continue;
    }

    // Now let's build the lorentz vectors and check the couples
    array<TLorentzVector, 3> jets;
    for (int i = 0; i < 3; i++) {
      UInt_t j = bjet_indexes[i];
      Float_t real_pt;
      get_real_pt(j, &real_pt);
      jets[i].SetPtEtaPhiM(real_pt, jet_eta_[j], \
                           jet_phi_[j], jet_mass_[j]);
    }


    // Now i check if the couples jets are distant enough
    bool success = true;
    for (unsigned int i = 0; i < 3; i++) {
      for (unsigned int j = i + 1; j < 3; j++) {
        if (!check_distance_enough(jets[i], jets[j])) {
          success = false;
          break;
        }
      }
    }
    if (!success) { continue; }
    if (TMath::Abs(jets[0].Eta() - jets[1].Eta()) > 1.5) {
      continue;
    }
    TLorentzVector appo = jets[0] - jets[1];
    cout << "\033[0;32mFilling with " << -appo.M() << "\033[0m" << endl;

    if (debug) {
      cout << "Leptons in the event: electron " << \
        jet_n_electrons << "\tmuons: " << jet_n_muons << endl;
    }
    
    if (jet_n_muons != 0 || jet_n_electrons != 0) {
      plot1->Fill(-appo.M());
    } else {
      plot2->Fill(-appo.M());
    }
  }
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
  SetTreeBranches(tree);
  TH1F mass_histo_chromo("mass_histo_chromo", \
                      "Invariant mass of bb jets", 50, 0, 800);
  TH1F mass_histo_lepton("mass_histo_lepton", \
                            "Invariant mass of bb jets", 50, 0, 800);
  fill_histogram(tree, &mass_histo_chromo, &mass_histo_lepton);
  output.cd();
  cout << "Entries in chromo histo: " << \
    mass_histo_chromo.GetEntries() << endl;
  cout << "Entries in lepton histo: " << \
    mass_histo_lepton.GetEntries() << endl;
  mass_histo_chromo.SetLineColor(kGreen);
  mass_histo_lepton.SetLineColor(kRed);
  mass_histo_chromo.Write();
  mass_histo_lepton.Write();
  TH1F mass_histo_sum(mass_histo_chromo);
  mass_histo_sum.Add(&mass_histo_lepton, 1.);
  mass_histo_sum.SetLineColor(kBlue);
  mass_histo_sum.Write();
  nano.Close();
  return 0;
}
