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
#include "Analysis/Tools/interface/Analyser.h"

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

Analyser::Analyser()
{
}


Analyser::Analyser(int argc, char * argv[]) : TriggerAnalyser(argc,argv),
                                              JetAnalyser(argc,argv),
                                              MuonAnalyser(argc,argv)
{
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
   bool ok = true;
   analysis_->event(i);
   cutflow_ = 0;
   
   if ( config_->isMC() )
   {
      h1_["cutflow"] -> Fill(cutflow_);
   }
   
   if ( config_->runmin_ > 0 && analysis_->run() < config_->runmin_ ) return false;
   if ( config_->runmax_ > 0 && analysis_->run() > config_->runmax_ ) return false;
   
   if (! config_->isMC() ) ok = analysis_->selectJson();
   
   if ( ! ok ) return false;
   
   analysisWithJets();
   analysisWithMuons();
      
   return ok;
   
}

