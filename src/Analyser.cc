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
   analysis_->event(i);
   if (! config_->isMC_ )
   {
      if (!analysis_->selectJson() ) return false; // To use only goodJSonFiles
   }
   if ( ! analysis_->triggerResult(config_->hltPath_) ) return false;
   
   return true;
   
}

void Analyser::histograms(const std::string & obj, const int & n)
{
   if ( obj == "jet" )
   {
      for ( int j = 0; j < n; ++j )
      {
         h1_[Form("pt_jet%d",j)]       = new TH1F(Form("pt_jet%d",j)     , "" ,100 , 0   , 1000  );
         h1_[Form("eta_jet%d",j)]      = new TH1F(Form("eta_jet%d",j)    , "" , 60 , -3, 3 );
         h1_[Form("btag_jet%d",j)]     = new TH1F(Form("btag_jet%d",j)   , "" , 100 , 0, 1 );
         for ( int k = j+1; k < n && j < n; ++k )
         {
            h1_[Form("dr_jet%d%d",j,k)]     = new TH1F(Form("dr_jet%d%d",j,k)     , "" , 50 , 0, 5 );
            h1_[Form("deta_jet%d%d",j,k)]   = new TH1F(Form("deta_jet%d%d",j,k)   , "" ,100 , 0,10 );
         }
      }
      
   }
   
}


std::shared_ptr<Analysis> Analyser::analysis()
{
   return analysis_;
}

std::shared_ptr<Config> Analyser::config()
{
   return config_;
}




