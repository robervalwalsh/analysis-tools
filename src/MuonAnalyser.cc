// system include files
#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
//
// user include files
#include "Analysis/Tools/interface/MuonAnalyser.h"

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

MuonAnalyser::MuonAnalyser() {
}


MuonAnalyser::MuonAnalyser(int argc, char * argv[]) : BaseAnalyser(argc,argv) {
   // Physics objects
   // Muons
   muons_analysis_  = ( analysis_->addTree<Muon> ("Muons",config_->muonsCollection()) != nullptr  && config_ -> nMuonsMin() > 0 );
   if(config_->onlinemuonSF() != "" &&  config_->isMC() && ! config_->muonsVeto()) {
      muon_trigger_efficiency_ = std::make_unique<MuonTriggerEfficiencies>(config_->onlinemuonSF());
      muon_id_weights_ = std::make_unique<MuonIdWeight>(config_->muonIDWeights());
   }
}

MuonAnalyser::~MuonAnalyser() {
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}
//
// member functions
//
// ------------ method called for each event  ------------

bool MuonAnalyser::analysisWithMuons() {
   muons_.clear();
   selected_muons_.clear();
   online_matched_muons_.clear();
   // trigger emulation
   // L1 muons
   std::string trigger_objects_l1muons;
   if ( config_->triggerObjectsL1Muons() != "" ) {
      trigger_objects_l1muons = config_->triggerObjectsL1Muons();
      if ( config_->triggerEmulateL1Muons() != "" &&  config_->triggerEmulateL1MuonsNMin() > 0 )  {
         int nmin = config_->triggerEmulateL1MuonsNMin();
         float ptmin = config_->triggerEmulateL1MuonsPtMin();
         float etamax = config_->triggerEmulateL1MuonsEtaMax();
         std::string newL1Muons = config_->triggerEmulateL1Muons();
         triggerEmulation(trigger_objects_l1muons,nmin,ptmin,etamax,newL1Muons);
         trigger_objects_l1muons = newL1Muons;
      }
   }
   // L3 muons
   std::string trigger_objects_l3muons;
   if ( config_->triggerObjectsL3Muons() != "" ) {
      trigger_objects_l3muons = config_->triggerObjectsL3Muons();
      if ( config_->triggerEmulateL3Muons() != "" &&  config_->triggerEmulateL3MuonsNMin() > 0 ) {
         int nmin = config_->triggerEmulateL3MuonsNMin();
         float ptmin = config_->triggerEmulateL3MuonsPtMin();
         float etamax = config_->triggerEmulateL3MuonsEtaMax();
         std::string newL3Muons = config_->triggerEmulateL3Muons();
         triggerEmulation(trigger_objects_l3muons,nmin,ptmin,etamax,newL3Muons);
         trigger_objects_l3muons = newL3Muons;
      }
   }
   if ( ! muons_analysis_ )
      return false;
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Using Muon collection: %s",(config_->muonsCollection()).c_str()));
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   if ( config_->triggerObjectsL1Muons() != "" )
      analysis_->match<Muon,TriggerObject>("Muons",trigger_objects_l1muons,config_->triggerMatchL1MuonsDrMax());
   if ( config_->triggerObjectsL3Muons() != "" )
      analysis_->match<Muon,TriggerObject>("Muons",trigger_objects_l3muons,config_->triggerMatchL3MuonsDrMax());
   auto muons = analysis_->collection<Muon>("Muons");
   for ( int j = 0 ; j < muons->size() ; ++j )  muons_.push_back(std::make_shared<Muon>(muons->at(j)));
   selected_muons_ = muons_;
   return true;
}



// FIXME: need to pass label to the histograms
void MuonAnalyser::muonHistograms(const std::string & obj, const int & n) {
   if ( obj == "muon" ) {
      for ( int j = 0; j < n; ++j ) {
         h1_[Form("pt_%s%d"  , obj.c_str(),j+1)]    = std::make_shared<TH1F>(Form("pt_%s%d"  , obj.c_str(),j+1)   , "" ,100 , 0   , 1000  );
         h1_[Form("eta_%s%d" , obj.c_str(),j+1)]    = std::make_shared<TH1F>(Form("eta_%s%d" , obj.c_str(),j+1)   , "" , 60 , -3, 3 );
         h1_[Form("phi_%s%d" , obj.c_str(),j+1)]    = std::make_shared<TH1F>(Form("phi_%s%d" , obj.c_str(),j+1)   , "" , 64 , -3.2, 3.2 );
      }
   }
}


bool MuonAnalyser::selectionMuon(const int & r) {
   bool isgood = true;
   ++cutflow_;
   int m = r-1;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) {
      if ( config_->muonsPtMax().size() > 0 && config_->muonsPtMax()[m] > config_->muonsPtMin()[m] )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Muon %d: pt > %5.1f GeV and pt < %5.1f GeV and |eta| < %3.1f",r,config_->muonsPtMin()[m], config_->muonsPtMax()[m],config_->muonsEtaMax()[m] ));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Muon %d: pt > %5.1f GeV and |eta| < %3.1f",r,config_->muonsPtMin()[m], config_->muonsEtaMax()[m] ));
   }
   // kinematic selection
   if ( selected_muons_[m] -> pt() < config_->muonsPtMin()[m]           && !(config_->muonsPtMin()[m] < 0) )
      return false;
   if ( fabs(selected_muons_[m] -> eta()) > config_->muonsEtaMax()[m]   && !(config_->muonsEtaMax()[m] < 0) )  
      return false;
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return isgood;
}

bool MuonAnalyser::selectionMuons() {
   // selectedMuons will be composed of muons with the lowest pt threshold
   if ( ! muons_analysis_ ) // will skip this selection
      return true; 
   bool isgood = true;
   ++cutflow_;
//
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) {
      if ( config_->muonsPtMax().size() > 0 && config_->muonsPtMax().back() > config_->muonsPtMin().back() )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Muons selected: pt > %5.1f GeV and pt < %5.1f GeV and |eta| < %3.1f", config_->muonsPtMin().back(), config_->muonsPtMax().back(), config_->muonsEtaMax().back() ));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Muons selected: pt > %5.1f GeV and |eta| < %3.1f", config_->muonsPtMin().back(), config_->muonsEtaMax().back() ));
   }
   // kinematic selection
   auto muon = std::begin(selected_muons_);
   while ( muon != std::end(selected_muons_) )   {
      if ( config_->muonsPtMax().size() > 0 && config_->muonsPtMax().back() > config_->muonsPtMin().back() ) {
         if ( (*muon)->pt() < config_->muonsPtMin().back() || (*muon)->pt() > config_->muonsPtMax().back() || fabs((*muon)->eta()) > config_->muonsEtaMax().back() )
            muon = selected_muons_.erase(muon);
         else
            ++muon;
      } else {
         if ( (*muon)->pt() < config_->muonsPtMin().back() || fabs((*muon)->eta()) > config_->muonsEtaMax().back() )
            muon = selected_muons_.erase(muon);
         else
            ++muon;
      }
   }
   if ( ! config_->muonsVeto() ) {
      if ( (int)selected_muons_.size() < config_->nMuonsMin() )
         return false;
   }
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return isgood;
}



bool MuonAnalyser::selectionMuonId() {
   if ( ! muons_analysis_ ) return true;  // will skip this selection
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("MuonId: %s",config_->muonsId().c_str()));
   auto muon = std::begin(selected_muons_);
   while ( muon != std::end(selected_muons_) ) {
      if ( ! (*muon)->id(config_->muonsId() ) )
         muon = selected_muons_.erase(muon);
      else
         ++muon;
   }
   if ( ! config_->muonsVeto() ) {
      if ( (int)selected_muons_.size() < 1 )
         return false;
   }
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return true;
}

bool MuonAnalyser::selectionNMuons() {
   if ( ! muons_analysis_ ) return true;  // will skip this selection
   ++cutflow_;
   if ( ! config_->muonsVeto() )
      if  ((int)selected_muons_.size() < config_->nMuonsMin()) return false;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("NMuons >= %d",config_->nMuonsMin()));
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return true;

}
bool MuonAnalyser::selectionMuonDr(const int & r1, const int & r2, const float & delta) {
   if ( r1 > config_->nMuonsMin() ||  r2 > config_->nMuonsMin() ) return true;
   bool isgood = true;
   std::string label = Form("DR(muon %d, muon %d) < %4.2f",r1,r2,fabs(delta));
   if ( delta < 0 )
      label = Form("DR(muon %d, muon %d) > %4.2f",r1,r2,fabs(delta));
   int m1 = r1-1;
   int m2 = r2-1;
   if ( delta > 0 )
      isgood = ( selected_muons_[m1]->deltaR(*selected_muons_[m2]) < fabs(delta) );
   else
      isgood = ( selected_muons_[m1]->deltaR(*selected_muons_[m2]) > fabs(delta) );
   cutflow(label,isgood);
   return isgood;
}

bool MuonAnalyser::selectionMuonDr(const int & r1, const int & r2) {
   bool ok = true;
   if (config_->muonsDrMax() < 0 )
      ok = ok && true;
   else
      ok = ok && selectionMuonDr(r1,r2,config_->muonsDrMax());
   if (config_->muonsDrMin() < 0 )
      ok = ok && true;
   else
      ok = ok && selectionMuonDr(r1,r2,-1*config_->muonsDrMin());
   return ok;
}

bool MuonAnalyser::onlineMuonMatching(const bool & matched_only) {
   if ( ! muons_analysis_ ) return true;  // will skip this selection
   if ( config_->triggerObjectsL1Muons() == "" && config_->triggerObjectsL3Muons() == ""  ) return true;
   ++cutflow_;
   std::string label = Form("Online muon matching: L1 (deltaR < %4.3f) and L3 (deltaR < %4.3f)",config_-> triggerMatchL1MuonsDrMax(),config_-> triggerMatchL3MuonsDrMax());
   if ( matched_only ) label = Form("Muons selected: Online muon matching: L1 (deltaR < %4.3f) and L3 (deltaR < %4.3f)",config_-> triggerMatchL1MuonsDrMax(),config_-> triggerMatchL3MuonsDrMax());
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,label.c_str());
   if ( matched_only ) {
      auto muon = std::begin(selected_muons_);
      while ( muon != std::end(selected_muons_) ) {
         if ( !((*muon)->matched(config_->triggerObjectsL1Muons()) && (*muon)->matched(config_->triggerObjectsL3Muons()) ))
            muon = selected_muons_.erase(muon);
         else
            ++muon;
      }
   }
   if ( (int)selected_muons_.size() < config_->nMuonsMin() )
      return false;
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return true;
}

bool MuonAnalyser::onlineL1MuonMatching(const int & r) {
   int j = r-1;
   if ( config_->triggerObjectsL1Muons() == "" )
      return true;
   ++cutflow_;
   std::string trigger_objects_l1muons = config_->triggerObjectsL1Muons();
   if ( config_->triggerEmulateL1Muons() != "" &&  config_->triggerEmulateL1MuonsNMin() > 0 ) {
      trigger_objects_l1muons = config_->triggerEmulateL1Muons();
   }
   if ( r > config_->nMuonsMin() ) {
      std::cout << "*Warning* MuonAnalyser::onlineL1MuonMatching(): asking for matching of unselected muon. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( selected_muons_.size() == 0 ) {
      std::cout << "*Warning* MuonAnalyser::onlineL1MuonMatching(): selectedMuons is empty. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( ! selected_muons_[j]->matched(trigger_objects_l1muons) )
      return false;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Muon %d: L1 muon match (deltaR < %4.3f)",r,config_-> triggerMatchL1MuonsDrMax()));
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return true;
}

bool MuonAnalyser::onlineL3MuonMatching(const int & r) {
   int j = r-1;
   if ( config_->triggerObjectsL3Muons() == "" )
      return true;
   ++cutflow_;
   std::string trigger_objects_l3muons = config_->triggerObjectsL3Muons();
   if ( config_->triggerEmulateL3Muons() != "" &&  config_->triggerEmulateL3MuonsNMin() > 0 ) {
      trigger_objects_l3muons = config_->triggerEmulateL3Muons();
   }
   if ( r > config_->nMuonsMin() ) {
      std::cout << "*Warning* MuonAnalyser::onlineL3MuonMatching(): asking for matching of unselected muon. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( selected_muons_.size() == 0 ) {
      std::cout << "*Warning* MuonAnalyser::onlineL3MuonMatching(): selectedMuons is empty. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( ! selected_muons_[j]->matched(trigger_objects_l3muons) )
      return false;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Muon %d: L3 muon match  (deltaR < %4.3f)",r,config_-> triggerMatchL3MuonsDrMax()));
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return true;
}


// FIXME: need to pass label to the histograms
void MuonAnalyser::fillMuonHistograms() {
   int n = config_->nMuonsMin();
   for ( int j = 0; j < n; ++j )  {
      h1_[Form("pt_muon%d",j+1)] -> Fill(selected_muons_[j]->pt());
      h1_[Form("eta_muon%d",j+1)] -> Fill(selected_muons_[j]->eta());
      h1_[Form("phi_muon%d",j+1)] -> Fill(selected_muons_[j]->phi());
   }
}

bool MuonAnalyser::muonCorrections() {
   // muon online trigger scale factor
   if (config_->onlinemuonSF() != "" &&  config_->isMC() && selected_muons_.size()!= 0)
      applyMuonOnlineSF(selected_muons_[0]->pt()); // apply muon online SF according to the first muon in selected_muons_ vector
   return true;
}

bool MuonAnalyser::muonCorrections(const double & muonpT) {
   // muon online trigger scale factor
   if (config_->onlinemuonSF() != "" &&  config_->isMC() && selected_muons_.size()!= 0)
      applyMuonOnlineSF(muonpT); // apply muon online SF according to the pt indicated
   return true;
}

void MuonAnalyser::applyMuonOnlineSF(const double & muonpT) {
// Muon Online Corrections to be applied to MC
   if ( ! muons_analysis_ || ! config_->isMC() || selected_muons_.size() < 1) return; //check and print error message
   double scale_factor = 1;
   std::string label = "WARNING: NO Muon Online Scale factor (*** missing Scale Factor Info ***)";
   if ( config_->onlinemuonSF() != "") {
      std::string bnsf = basename(config_->onlinemuonSF());
      label = Form("Muon Online Scale Factor (%s)",bnsf.c_str());
      if ( config_->onlinemuonSystematics() != 0 )  {
         if (fabs(config_->onlinemuonSystematics()) == 1 || fabs(config_->onlinemuonSystematics()) == 2) {
            label = Form("Muon Online Scale Factor: (%s), syst: %+d sig",bnsf.c_str(),config_->onlinemuonSystematics());
         } else {
            std::string label = Form("WARNING: NO Muon Online Scale factor (*** missing Scale Factor Info for syst = %+d sig ***)",config_->onlinemuonSystematics());       
            cutflow(label);
            return;
         }  
      }
      scale_factor *= muon_trigger_efficiency_->findSF(muonpT, config_->onlinemuonSystematics());
      weight_ *= scale_factor; //apply scale_factor to event weight
   } 
   cutflow(label);
}

void MuonAnalyser::actionApplyMuonOnlineSF(const int & rank) {
   if (!muons_analysis_ || !config_->isMC() || config_->muonsVeto()) // action not applicable in some cases
      return; 
   int m = rank-1;
   float scale_factor = 1.;
   int systematic = config_->onlinemuonSystematics();
   std::string label = "WARNING: NO Muon Online Scale factor (*** assuming SF = 1 ***)";
   if (config_->onlinemuonSF() != "") {
      std::string bnsf = basename(config_->onlinemuonSF());
      label = Form("Muon %d: online scale factor (%s)", rank, bnsf.c_str()); // assuming central value
      if ( systematic != 0 ) {
         label = Form("Muon %d: online scale factor syst = %+d sig (%s) ", rank, systematic, bnsf.c_str());
      }
      auto muon_pt = selected_muons_[m]->pt();
      if ( abs(systematic) > 2 ) 
         std::cout << " *** Error ***: there is no systematic variation > 2 sigma!" << std::endl;
      scale_factor = muon_trigger_efficiency_->findSF(muon_pt, systematic);
   }
   weight_ *= scale_factor; // apply scale_factor to event weight
   cutflow(label);
}


void MuonAnalyser::actionApplyMuonIDSF(const int & rank) {
   if (!muons_analysis_ || !config_->isMC() || config_->muonsVeto()) // action not applicable in some cases
      return; 
   int m = rank-1;
   float scale_factor = 1.;
   int systematic = config_->muonIDWeightSystematics(); 
   std::string label = "WARNING: NO Muon ID Scale factor (*** assuming SF = 1 ***)";
   if (config_->muonIDWeights().size() != 0) {
      label = Form("Muon %d: ID scale factor ", rank);
      if ( systematic == 0) {
         for(unsigned int f = 0; f < config_->muonIDWeights().size(); f++) {
            std::string bnsf = basename(config_->muonIDWeights()[f]);
            if (f == 0)
            label += "("; 
            label += Form("%s", bnsf.c_str());
            if (f != config_->muonIDWeights().size()-1)
            label += ",";
            else
            label += ")";
         }
      }
      if ( systematic != 0 ) {
         label = Form("Muon %d: ID scale factor syst = %+d sig ", rank, systematic);
      }
      auto muon_pt = selected_muons_[m]->pt();
      auto muon_eta = selected_muons_[m]->eta();
      scale_factor = muon_id_weights_->findSF(muon_pt, muon_eta, systematic);
   }
   weight_ *= scale_factor; // apply scale_factor to event weight
   cutflow(label);
}

std::shared_ptr<MuonIdWeight> MuonAnalyser::muonIDWeights() {
   return muon_id_weights_;
}

std::shared_ptr<MuonTriggerEfficiencies> MuonAnalyser::muonTriggerEfficiency() {
   return muon_trigger_efficiency_;
}

std::vector< std::shared_ptr<Muon> > MuonAnalyser::muons() {
   return muons_;
}

std::vector< std::shared_ptr<Muon> > MuonAnalyser::selectedMuons() {
   return selected_muons_;
}

std::vector< std::shared_ptr<Muon> > MuonAnalyser::onlineMatchedMuons() {
   return online_matched_muons_;
}

bool MuonAnalyser::muonsAnalysis() {
   return muons_analysis_;
}




