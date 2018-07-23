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


UInt_t run_;
ULong64_t event_;
UInt_t lumisection_;


UInt_t ngenjet_;
Float_t genjet_pt_[kLen];
Float_t genjet_eta_[kLen];
Float_t genjet_phi_[kLen];
Float_t genjet_mass_[kLen];
Int_t genjet_id_[kLen];
Float_t btag_deep_[kLen];

// I'd better check if it is correct this flag selection
const Int_t kTight = 2;
const array<Float_t, 3> pt_cut{{100, 100, 40}};
const array<Float_t, 3> eta_cut{{2.2, 2.2, 2.2}};
const bool debug = false;


int SetTreeBranches(TTree * t) {
  // Event Info
  t->SetBranchAddress("event", &event_);
  t->SetBranchAddress("run", &run_);
  t->SetBranchAddress("luminosityBlock", &lumisection_);
  // gen jets
  t->SetBranchAddress("nJet", &ngenjet_);
  t->SetBranchAddress("Jet_pt", genjet_pt_);
  t->SetBranchAddress("Jet_eta", genjet_eta_);
  t->SetBranchAddress("Jet_phi", genjet_phi_);
  t->SetBranchAddress("Jet_mass", genjet_mass_);
  t->SetBranchAddress("Jet_jetId", genjet_id_);
  t->SetBranchAddress("Jet_btagDeepB", btag_deep_);
  return 0;
}


bool check_distance_enough(const TLorentzVector &a, const TLorentzVector &b) {
  if (debug) {
    cout << "Delta R: " << a.DeltaR(b) << endl;
    cout << "Eta bw 4vec: " << TMath::Abs(a.Eta() - b.Eta()) << endl;
  }
  return a.DeltaR(b) > 1;
}


void fill_histogram(TTree* tree, TH1F* mass_histo) {
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
      cout << " number of gen jets from ngenjet = " << ngenjet_ << endl;
    }

    // If we have less than 3 jets the event is not useful
    if (debug) {
      cout << "ngenjets: " << ngenjet_ << endl;
    }
    if (ngenjet_ < 3) {
      continue;
    }

    
    // Apply first cut on momentum and eta without checking
    // real identity
    bool prosegui = true;
    for (unsigned int i = 0; i < pt_cut.size(); i++) {
      if (debug) {
        cout << "pt: " << genjet_pt_[i] << "\neta: " << \
          genjet_eta_[i] << "\nid: " << genjet_id_[i] << endl;
      }
      if (genjet_pt_[i] < pt_cut[i]) {
        prosegui = false; break;
      }
      if (TMath::Abs(genjet_eta_[i]) > eta_cut[i]) {
        prosegui = false; break;
      }
      if (debug) {
      cout << "genjetid: " << genjet_id_[i] << "\tktight: "  \
           << kTight << "\tbitwise: " << (genjet_id_[i] & kTight) \
           << endl;
      }
      if ((genjet_id_[i] & kTight) == 0) {
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
    for (unsigned int i = 0; i < ngenjet_ && btaggati < 3; i++) {
      if (debug) {
        cout << "btag_deep: " << btag_deep_[i] << "\tpt: " << genjet_pt_[i] \
             << "\teta: " << genjet_eta_[i] << endl;
      }
      if (btag_deep_[i] > deepcsv_cut && genjet_pt_[i] >                \
          pt_cut[btaggati] && TMath::Abs(genjet_eta_[i]) < eta_cut[btaggati]) {
        btaggati++;
        bjet_indexes.push_back(i);
      } else {
        if (genjet_pt_[i] < pt_cut[pt_cut.size() - 1]) {
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
      jets[i].SetPtEtaPhiM(genjet_pt_[j], genjet_eta_[j], \
                           genjet_phi_[j], genjet_mass_[j]);
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
    mass_histo->Fill(-appo.M());
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
  TH1F mass_histo("mass_histo", "Invariant mass of bb jets", 50, 0, 800);
  fill_histogram(tree, &mass_histo);
  output.cd();
  cout << "Entries in histo: " << mass_histo.GetEntries() << endl;
  mass_histo.Write();
  nano.Close();
  return 0;
}

