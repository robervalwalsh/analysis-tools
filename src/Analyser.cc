// system include files
#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
// 
// user include files
#include "Analysis/Tools/interface/Analyser.h"

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

Analyser::Analyser()
{
}


Analyser::Analyser(int argc, char * argv[]) : BaseAnalyser(argc,argv),  // not sure the BaseAnalyser should be called here
                                              TriggerAnalyser(argc,argv),
                                              JetAnalyser(argc,argv),
                                              MuonAnalyser(argc,argv)
{
   this -> pileupHistogram();
   this -> histograms("final_selection");
}

Analyser::~Analyser()
{
}


//
// member functions
//
// ------------ method called for each event  ------------

bool Analyser::event(const int & i)
{
   if ( !firstevent_ && isgoodevent_ )
   {
      this->fillHistograms("final_selection");
   }
   firstevent_ = false;
//   bool ok = true;
   analysis_->event(i);
   cutflow_ = 0;
   weight_ = 1.;  // reset weight at the beginning of the event analysis
   isgoodevent_ = true;
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   this -> generatorWeight();
   
   if ( config_->runmin_ > 0 && analysis_->run() < config_->runmin_ ) { isgoodevent_ = false; return false; }
   if ( config_->runmax_ > 0 && analysis_->run() > config_->runmax_ ) { isgoodevent_ = false; return false; }

   if (! config_->isMC() ) 
   {
       auto json = basename(config_->json());
       ++cutflow_;
       if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
       {
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Certified data: %s",json.c_str()));
       }
       isgoodevent_ = analysis_->selectJson();
       if ( ! isgoodevent_ ) return false;
       h1_["cutflow"] -> Fill(cutflow_,weight_);
   }
   
   if ( this->genParticlesAnalysis() )
   {
      ++cutflow_;
      if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      {
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Using GenParticles collection: %s",(config_->genParticlesCollection()).c_str()));
      }
      h1_["cutflow"] -> Fill(cutflow_,weight_);
   }
   
   if ( this->genJetsAnalysis() )
   {
      ++cutflow_;
      if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
      {
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Using GenJets collection: %s",(config_->genJetsCollection()).c_str()));
      }
      h1_["cutflow"] -> Fill(cutflow_,weight_);
   }

   analysisWithJets();
   analysisWithMuons();
      
    
   return isgoodevent_;
   
}

bool Analyser::muonJet(const int & r)
{
   
   if ( ! muonsanalysis_ ) return true;  // will skip this selection

   int j = r-1;
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
   {
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: Jet-muon association",r));
   }
   
   auto jet = selectedJets_[j];
   jet -> addMuon(selectedMuons_);
   
   if ( ! jet -> muon() ) return false;
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   return true;
   
}

void Analyser::histograms(const std::string & s)
{
   this->makeDirectory(s);
   if ( jetsanalysis_ ) this -> jetHistograms(s);
}

void Analyser::fillHistograms(const std::string & s)
{
   if ( analysisWithJets() ) this -> fillJetHistograms(s);
}
