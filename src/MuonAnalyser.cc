/**\class MuonAnalyser MuonAnalyser.cc Analysis/Tools/src/MuonAnalyser.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Roberval Walsh Bastos Rangel
//         Created:  Mon, 20 Oct 2014 14:24:08 GMT
//
//

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

MuonAnalyser::MuonAnalyser()
{
}


MuonAnalyser::MuonAnalyser(int argc, char * argv[]) : BaseAnalyser(argc,argv)
{
   // Physics objects
   // Muons
   muonsanalysis_  = ( analysis_->addTree<Muon> ("Muons",config_->muonsCollection()) != nullptr );
   
   if ( muonsanalysis_ )
   {
      if ( config_->triggerObjDir_ != "" )
      {
         analysis_->addTree<TriggerObject> (config_->triggerObjectsL1Muons_,Form("%s/%s",config_->triggerObjDir_.c_str(),config_->triggerObjectsL1Muons_.c_str()));
         analysis_->addTree<TriggerObject> (config_->triggerObjectsL3Muons_,Form("%s/%s",config_->triggerObjDir_.c_str(),config_->triggerObjectsL3Muons_.c_str()));
      }
      muonHistograms("muon",config_->nMuonsMin());
   }
}

MuonAnalyser::~MuonAnalyser()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//
// ------------ method called for each event  ------------

bool MuonAnalyser::analysisWithMuons()
{
   muons_.clear();
   selectedMuons_.clear();
   onlineMatchedMuons_.clear();
   if ( ! muonsanalysis_ ) return false;
   
   analysis_->match<Muon,TriggerObject>("Muons",config_->triggerObjectsL1Muons_,0.3);
   analysis_->match<Muon,TriggerObject>("Muons",config_->triggerObjectsL3Muons_,0.3);

   auto muons = analysis_->collection<Muon>("Muons");
   for ( int j = 0 ; j < muons->size() ; ++j )  muons_.push_back(std::make_shared<Muon>(muons->at(j)));
   
   selectedMuons_ = muons_;
   
   return true;
}


std::vector< std::shared_ptr<Muon> > MuonAnalyser::muons()
{
   return muons_;
}

std::vector< std::shared_ptr<Muon> > MuonAnalyser::selectedMuons()
{
   return selectedMuons_;
}

std::vector< std::shared_ptr<Muon> > MuonAnalyser::onlineMatchedMuons()
{
   return onlineMatchedMuons_;
}




void MuonAnalyser::muonHistograms(const std::string & obj, const int & n)
{
   if ( obj == "muon" )
   {
      for ( int j = 0; j < n; ++j )
      {
         h1_[Form("pt_%s%d"  , obj.c_str(),j+1)]    = std::make_shared<TH1F>(Form("pt_%s%d"  , obj.c_str(),j+1)   , "" ,100 , 0   , 1000  );
         h1_[Form("eta_%s%d" , obj.c_str(),j+1)]    = std::make_shared<TH1F>(Form("eta_%s%d" , obj.c_str(),j+1)   , "" , 60 , -3, 3 );
         h1_[Form("phi_%s%d" , obj.c_str(),j+1)]    = std::make_shared<TH1F>(Form("phi_%s%d" , obj.c_str(),j+1)   , "" , 64 , -3.2, 3.2 );
      }
      
   }
}


bool MuonAnalyser::selectionMuon(const int & r)
{
   ++cutflow_;
   
   bool isgood = true;
   int j = r-1;
   
   if ( selectedMuons_.size() == 0 ) isgood = (isgood && selectionMuonId());
   if ( !isgood || (int)selectedMuons_.size() < r ) return false;
   
   // kinematic selection
   if ( selectedMuons_[j] -> pt() < config_->muonsPtMin()[j]           && !(config_->muonsPtMin()[j] < 0) )   return false;
   if ( fabs(selectedMuons_[j] -> eta()) > config_->muonsEtaMax()[j]   && !(config_->muonsEtaMax()[j] < 0) )  return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Muon %d: pt > %5.1f and |eta| < %3.1f",r,config_->muonsPtMin()[j], config_->muonsEtaMax()[j] ));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return isgood;
}



bool MuonAnalyser::selectionMuonId()
{
   ++cutflow_;
   
   if ( ! muonsanalysis_ ) return false;
   
   auto muon = std::begin(selectedMuons_);
   while ( muon != std::end(selectedMuons_) )
   {
      if ( ! (*muon)->id(config_->muonsId() ) )
         muon = selectedMuons_.erase(muon);
      else
         ++muon;
   }
   if ( selectedMuons_.size() == 0 ) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("MuonId: %s",config_->muonsId().c_str()));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}

bool MuonAnalyser::selectionNMuons()
{
   ++cutflow_;
   
   if  ((int)selectedMuons_.size() < config_->nMuonsMin()) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("NMuons >= %d",config_->nMuonsMin()));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
   
}


bool MuonAnalyser::onlineMuonMatching()
{
   if ( config_->triggerObjectsL1Muons_ == "" && config_->triggerObjectsL3Muons_ == ""  ) return true;
   
   for ( size_t i = 0; i < selectedMuons_.size() ; ++i )
   {
      int m = i+1;
      if ( onlineL1MuonMatching(m) && onlineL3MuonMatching(m) ) onlineMatchedMuons_.push_back(selectedMuons_[i]);
   }
   
   if ( onlineMatchedMuons_.size() == 0 ) return false;
   
   return true;
}

bool MuonAnalyser::onlineL1MuonMatching(const int & r)
{
   int j = r-1;
   if ( config_->triggerObjectsL1Muons_ == "" ) return true;
   
   ++cutflow_;
   
   if ( r > config_->nMuonsMin() )
   {
      std::cout << "*Warning* MuonAnalyser::onlineL1MuonMatching(): asking for matching of unselected muon. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( selectedMuons_.size() == 0 )
   {
      std::cout << "*Warning* MuonAnalyser::onlineL1MuonMatching(): selectedMuons is empty. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   
   if ( ! selectedMuons_[j]->matched(config_->triggerObjectsL1Muons_) ) return false;

   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("L1MuonTriggerMatch_%d",r));
      
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}

bool MuonAnalyser::onlineL3MuonMatching(const int & r)
{
   int j = r-1;
   if ( config_->triggerObjectsL3Muons_ == "" ) return true;
   
   ++cutflow_;
   
   if ( r > config_->nMuonsMin() )
   {
      std::cout << "*Warning* MuonAnalyser::onlineL3MuonMatching(): asking for matching of unselected muon. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( selectedMuons_.size() == 0 )
   {
      std::cout << "*Warning* MuonAnalyser::onlineL3MuonMatching(): selectedMuons is empty. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   
   if ( ! selectedMuons_[j]->matched(config_->triggerObjectsL3Muons_) ) return false;

   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("L3MuonTriggerMatch_%d",r));
      
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}



void MuonAnalyser::fillMuonHistograms()
{
   int n = config_->nMuonsMin();
   
   for ( int j = 0; j < n; ++j )
   {
      h1_[Form("pt_muon%d",j+1)] -> Fill(selectedMuons_[j]->pt());
      h1_[Form("eta_muon%d",j+1)] -> Fill(selectedMuons_[j]->eta());
      h1_[Form("phi_muon%d",j+1)] -> Fill(selectedMuons_[j]->phi());
   }
   
}
