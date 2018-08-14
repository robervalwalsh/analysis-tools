


#include <array>
#include <string>
#include <iostream>

#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TRandom3.h"
#include "TMath.h"


const int prescale = 7;
const UInt_t kLen = 100;

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
Int_t jet_n_electrons[kLen];
Int_t jet_n_muons[kLen];
Bool_t trigger_path_had;
Bool_t trigger_path_lep;
Float_t jet_b_reg_res_[kLen];
Int_t jet_puId_[kLen];
Int_t jet_electron_idx1[kLen];
UInt_t n_electron;
Float_t electron_pt_[kLen];
Float_t electron_eta_[kLen];
Float_t electron_phi_[kLen];
Int_t jet_muon_idx1[kLen];
UInt_t n_muon;
Float_t muon_pt_[kLen];
Float_t muon_eta_[kLen];
Float_t muon_phi_[kLen];
Bool_t muon_tight_id_[kLen];
UInt_t n_trig_obj_;
Int_t trig_obj_id_[kLen];
Float_t trig_obj_pt_[kLen];
Float_t trig_obj_eta_[kLen];
Float_t trig_obj_phi_[kLen];
Float_t trig_obj_l1pt_[kLen];
Float_t trig_obj_l2pt_[kLen];
Int_t trig_obj_filter_bits_[kLen];
Float_t rho_jec_;
  
// Extra values to put in the new tree
Float_t mass_;
Float_t new_pt_[kLen];
Float_t new_eta_[kLen];
Float_t new_phi_[kLen];
Float_t new_mass_[kLen];
Float_t new_btag_deep_[kLen];
Bool_t leptonic;
UInt_t reordering_[kLen];
Float_t weigth;






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
  t->SetBranchAddress("Jet_bRegRes", jet_b_reg_res_);
  t->SetBranchAddress("HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33", &trigger_path_had);
  t->SetBranchAddress("HLT_Mu12_DoublePFJets40MaxDeta1p6_DoubleCaloBTagCSV_p33", &trigger_path_lep);
  t->SetBranchAddress("Jet_puId", jet_puId_);
  t->SetBranchAddress("Jet_electronIdx1", &jet_electron_idx1);
  t->SetBranchAddress("nElectron", &n_electron);
  t->SetBranchAddress("Electron_pt", electron_pt_);
  t->SetBranchAddress("Electron_eta", electron_eta_);
  t->SetBranchAddress("Electron_phi", electron_phi_);
  t->SetBranchAddress("Jet_muonIdx1", &jet_muon_idx1);
  t->SetBranchAddress("nMuon", &n_muon);
  t->SetBranchAddress("Muon_pt", muon_pt_);
  t->SetBranchAddress("Muon_eta", muon_eta_);
  t->SetBranchAddress("Muon_phi", muon_phi_);
  t->SetBranchAddress("Muon_tightId", &muon_tight_id_);
  t->SetBranchAddress("nTrigObj", &n_trig_obj_);
  t->SetBranchAddress("TrigObj_id", trig_obj_id_);
  t->SetBranchAddress("TrigObj_pt", trig_obj_pt_);
  t->SetBranchAddress("TrigObj_eta", trig_obj_eta_);
  t->SetBranchAddress("TrigObj_phi", trig_obj_phi_);
  t->SetBranchAddress("TrigObj_l1pt", trig_obj_l1pt_);
  t->SetBranchAddress("TrigObj_l2pt", trig_obj_l2pt_);
  t->SetBranchAddress("fixedGridRhoFastjetAll", &rho_jec_);
  t->SetBranchAddress("TrigObj_filterBits", trig_obj_filter_bits_);
  t->SetBranchAddress("Mass", &mass_);
  t->SetBranchAddress("Jet_newPt", new_pt_);
  t->SetBranchAddress("Jet_newEta", new_eta_);
  t->SetBranchAddress("Jet_newPhi", new_phi_);
  t->SetBranchAddress("Jet_newMass", new_mass_);
  t->SetBranchAddress("Jet_newOriginalOrder", reordering_);
  t->SetBranchAddress("Jet_newBtagDeepB", new_btag_deep_);
  t->SetBranchAddress("Weigth", &weigth);

  return 0;
}





int main(int argc, char* argv[]) {
  using std::cout; using std::endl;
  using std::array; using std::string;
  TRandom3 randomone(20180813);

  array<TFile*, prescale> out_files;
  array<TTree*, prescale> out_trees;
  TChain old_tree("output_tree");
  string correction = "nothing_false";
  old_tree.Add((string("output/hists/bkg/fourth/bkg_*_") + correction + string(".root")).c_str());
  SetTreeBranches(&old_tree);
  int counter = 0;
  for (Int_t i = 0; i < prescale; i++) {
    out_files[i] = new TFile((string("output/split/bkg/") + correction + string("_") + std::to_string(counter) + string(".root")).c_str(), "recreate");
    out_trees[i] = old_tree.CloneTree(0);
    counter++;
  }
  cout << "Total entries to process: " << old_tree.GetEntries() << endl;
  for (Long64_t i = 0; i < old_tree.GetEntries(); i++) {
    old_tree.GetEntry(i);
    Int_t appo = TMath::Floor(randomone.Uniform(prescale));
    out_files[appo]->cd();
    out_trees[appo]->Fill();
    if ((i % 50000) == 0) {
      cout << "Done " << i << " events" << endl;
      cout << "Random: " << appo << endl;
    }
  }

  for (Int_t i = 0; i < prescale; i++) {
    out_files[i]->cd();
    out_trees[i]->Write();
    out_files[i]->Close();
  }
  return  0;
}
