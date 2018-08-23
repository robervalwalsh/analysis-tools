/**\class Analyser Analyser.cc Analysis/Tools/src/Analyser.cc

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
#include "TString.h" 
#include "Analysis/Tools/interface/Analyser.h"

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

//
// constructors and destructor
//
// Analyser::Analyser(const std::string & inputFilelist, const std::string & evtinfo)
// {
//    analysis_ = new Analysis(inputFilelist,evtinfo);
// }
// 
// Analyser::Analyser(Analysis * a)
// {
//    analysis_ = a;
// }
// 
Analyser::Analyser()
{
}


Analyser::Analyser(int argc, char * argv[])
{
   config_   = std::make_shared<Config>(argc,argv);
   
   analysis_ = std::make_shared<Analysis>(config_->inputlist_);
   
   // Jets
   if ( config_->jetsCol_ != "" )       analysis_->addTree<Jet> ("Jets",config_->jetsCol_);
   // Trigger path info
   if ( config_->triggerCol_ != "" )    analysis_->triggerResults(config_->triggerCol_);
   // Trigger objects
   if ( config_->triggerObjects_.size() > 0 && config_->triggerObjDir_ != "" )
   {
      for ( auto & obj : config_->triggerObjects_ )
      {
         analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
      }
   }
   if ( config_->triggerObjectsJets_.size() > 0 && config_->triggerObjDir_ != "" )
   {
      for ( auto & obj : config_->triggerObjectsJets_ )
      {
         analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
      }
   }
   // JSON for data   
   if( !config_->isMC_ && config_->json_ != "" ) analysis_->processJsonFile(config_->json_);
   
   // output file
   if ( config_->outputRoot_ != "" )
   {
      std::string sr = "SR";
      if ( ! config_->signalregion_ ) sr = "CR";
      boost::algorithm::replace_last(config_->outputRoot_, ".root", "_"+sr+".root"); 
      hout_= std::shared_ptr<TFile>(new TFile(config_->outputRoot_.c_str(),"recreate"));
   }
}

Analyser::~Analyser()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//
// ------------ method called for each event  ------------

void Analyser::jets(const std::string & col)
{
   analysis_->addTree<Jet> ("Jets",col);
}
void Analyser::triggerResults(const std::string & col)
{
   analysis_->triggerResults(col);
}

void Analyser::analyse()
{
}

void Analyser::end()
{
   if ( hout_ )
   {
      hout_->Write();
      hout_->Close();
   }   
}

bool Analyser::event(const int & i)
{
   bool goodEvent = true;
   analysis_->event(i);
   
   if ( config_->runmin_ > 0 && analysis_->run() < config_->runmin_ ) return false;
   if ( config_->runmax_ > 0 && analysis_->run() > config_->runmax_ ) return false;
   
   if (! config_->isMC_ )
   {
      if (!analysis_->selectJson() ) return false; // To use only goodJSonFiles
   }
      
   return goodEvent;
   
}

void Analyser::histograms(const std::string & obj, const int & n)
{
   if ( obj == "jet" )
   {
      for ( int j = 0; j < n; ++j )
      {
         h1_[Form("pt_jet%d",j+1)]       = new TH1F(Form("pt_jet%d",j+1)     , "" ,100 , 0   , 1000  );
         h1_[Form("eta_jet%d",j+1)]      = new TH1F(Form("eta_jet%d",j+1)    , "" , 60 , -3, 3 );
         h1_[Form("btag_jet%d",j+1)]     = new TH1F(Form("btag_jet%d",j+1)   , "" , 100 , 0, 1 );
         for ( int k = j+1; k < n && j < n; ++k )
         {
            h1_[Form("dr_jet%d%d",j+1,k+1)]     = new TH1F(Form("dr_jet%d%d",j+1,k+1)     , "" , 50 , 0, 5 );
            h1_[Form("deta_jet%d%d",j+1,k+1)]   = new TH1F(Form("deta_jet%d%d",j+1,k+1)   , "" ,100 , 0,10 );
            h1_[Form("m_jet%d%d",j+1,k+1)]      = new TH1F(Form("m_jet%d%d",j+1,k+1)      , "" ,300 , 0,3000 );
         }
      }
      
   }
   
   h1_["cutflow"] = new TH1F("cutflow","", 20,0,20);
   
}


std::shared_ptr<Analysis> Analyser::analysis()
{
   return analysis_;
}

std::shared_ptr<Config> Analyser::config()
{
   return config_;
}


float Analyser::btag(const Jet & jet, const std::string & algo)
{
   float btag;
   if ( config_->btagalgo_ == "csvivf" || config_->btagalgo_ == "csv" )   {      btag = jet.btag("btag_csvivf");   }
   else if ( config_->btagalgo_ == "deepcsv" )                            {      btag = jet.btag("btag_deepb") + jet.btag("btag_deepbb");   }
   else if ( config_->btagalgo_ == "deepbvsall" )                         {      btag = jet.btag("btag_deepbvsall");   }
   else                                                                   {      btag = -9999;   }
   
   return btag;
}

bool Analyser::selectionJet()
{
   bool isgood = true;
   
   // jet kinematics
   std::map<std::string,bool> isOk;
   for ( int j = 0; j < config_->njetsmin_ ; ++j )
   {
      if ( ! selectionJet(j+1) ) return false;
   }
   return isgood;
}

bool Analyser::selectionJet(const int & r)
{
   bool isgood = true;
   int j = r-1;
   
   if ( selectedJets_.size() == 0 ) isgood = (isgood && selectionJetId());
   if ( !isgood || (int)selectedJets_.size() < r ) return false;
   
   // kinematic selection
   if ( selectedJets_[j] -> pt() < config_->jetsptmin_[j]           && !(config_->jetsptmin_[j] < 0) )   return false;
   if ( fabs(selectedJets_[j] -> eta()) > config_->jetsetamax_[j]   && !(config_->jetsetamax_[j] < 0) )  return false;
   
   return isgood;
}


bool Analyser::selectionJetId()
{
   bool isgood = true;
   
   if ( config_->jetsCol_ != "" )
   {
      auto slimmedJets = analysis_->collection<Jet>("Jets");
      selectedJets_.clear();
      for ( int j = 0 ; j < slimmedJets->size() ; ++j )
      {
         if ( slimmedJets->at(j).id(config_->jetsid_) && slimmedJets->at(j).pileupJetIdFullId(config_->jetspuid_) ) selectedJets_.push_back(&slimmedJets->at(j));
      }
      if ( (int)selectedJets_.size() < config_->njetsmin_ ) return false;
   }
   
   return isgood;
}

bool Analyser::analysisWithJets()
{
   bool isgood = true;
   
   if ( config_->jetsCol_ == "" ) return false;
   if ( config_->triggerObjectsJets_.size() > 0 )
   {
      analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsJets_,0.3);
   }
   
   return isgood;
}

bool Analyser::selectionBJet()
{
   bool isgood = true;
   // jet btag
   for ( int j = 0; j < config_->nbjetsmin_ ; ++j )
   {
      if ( ! selectionBJet(j+1) ) return false;
   }
   return isgood;
}

bool Analyser::selectionBJet(const int & r )
{
   bool isgood = true;
   int j = r-1;
   if ( config_->jetsbtagmin_[j] < 0 ) return true;
   if ( selectedJets_.size() == 0 ) isgood = (isgood && selectionJetId());
   if ( !isgood || (int)selectedJets_.size() < r ) return false;
   
   // jet  btag
   isgood = ( btag(*selectedJets_[j],config_->btagalgo_) > config_->jetsbtagmin_[j] );
   
   return isgood;
}

bool Analyser::selectionNonBJet(const int & r )
{
   bool isgood = true;
   int j = r-1;
   if ( config_->jetsbtagmin_[j] < 0 ) return true;
   if ( selectedJets_.size() == 0 ) isgood = (isgood && selectionJetId());
   if ( !isgood || (int)selectedJets_.size() < r ) return false;
   
   // jet  non btag
   isgood = ( btag(*selectedJets_[j],config_->btagalgo_) < config_->nonbtagwp_ );
   
   return isgood;
}

bool Analyser::selectionMuon()
{
   bool isgood = true;
   
   if ( config_->muonsCol_ != "" )
   {
   }
   
   return isgood;
}

bool Analyser::onlineJetMatching()
{
   bool isgood = true;
   
   if ( config_->triggerObjectsJetsMatches_ < 0 || config_->triggerObjectsJets_.size() == 0 ) return isgood;
   
   if ( selectedJets_.size() == 0 ) isgood = (isgood && selectionJet());
   if ( !isgood || (int)selectedJets_.size() < config_->triggerObjectsJetsMatches_ ) return false;
   
   for ( int j = 0; j < config_->triggerObjectsJetsMatches_; ++j )  // 
   {
      Jet * jet = selectedJets_[j];
      for ( size_t io = 0; io < config_->triggerObjectsJets_.size() ; ++io )
      {       
         if ( ! jet->matched(config_->triggerObjectsJets_[io]) ) return false;
      }
   }
   
   return isgood;
}

bool Analyser::selectionTrigger()
{
   bool isgood = true;
   
   if ( config_->hltPath_ == "" ) return isgood;
   
   if ( ! analysis_->triggerResult(config_->hltPath_) ) return false;
   
   if ( config_->l1Seed_ != "" )
   {
      if ( ! analysis_->triggerResult(config_->l1Seed_) ) return false;
   }

   return isgood;
}

