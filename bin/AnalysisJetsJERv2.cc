#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"
#include <string>
#include <iostream>
#include <vector>

#include "TFile.h" 
#include "TFileCollection.h"
#include "TChain.h"
#include "TH1.h" 

#include "Analysis/Tools/interface/Analysis.h"

#include "Analysis/Tools/bin/macro_config.h"

using namespace std;
using namespace analysis;
using namespace analysis::tools;


// =============================================================================================   
int main(int argc, char * argv[])
{
   if ( macro_config(argc, argv) != 0 ) return -1;

   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   // Input files list
   Analysis analysis(inputlist_);
   
   // Physics Objects Collections
   analysis.addTree<Jet> ("Jets",jetsCol_);
   analysis.addTree<GenJet> ("GenJets",genjetsCol_);
   
   // Jet energy resolution scale factors and pt resolution
   auto jerinfo = analysis.jetResolutionInfo(jerpt_,jersf_);
   
   // Analysis of events
   std::cout << "This analysis has " << analysis.size() << " events" << std::endl;
//   for ( int i = 0 ; i < analysis.size() ; ++i )
   for ( int i = 0 ; i < 10 ; ++i )
   {
      analysis.event(i);
      
      std::cout << "++++++    ENTRY  " << i;
      std::cout << std::endl;
      
      // Jets
      auto jets = analysis.collection<Jet>("Jets");
      auto genjets = analysis.collection<GenJet>("GenJets");
      
      jets->addGenJets(genjets);  // if not defined -> jerMatch = false, smearing will be done using the stochastic method only
      
      for ( int j = 0 ; j < jets->size() ; ++j )
      {
         Jet jet = jets->at(j);
         jet.jerInfo(*jerinfo,0.2); // this also performs matching to the added gen jets above, with delta R < 0.2 which is default and can be omitted
         
         
         std::cout << "    Jet #" << j << ": ";
         std::cout << "pT  = "     << jet.pt()      << ", ";
         std::cout << "eta = "     << jet.eta()     << ", ";
         std::cout << "resolution = " << jet.jerPtResolution() << ", ";
         std::cout << "JER SF  = "    << jet.jerSF()  << ", match = " << jet.jerMatch() << " jer corr = " << jet.jerCorrection() << " + ";
         std::cout << jet.jerCorrection("up") << " - " << jet.jerCorrection("down") << std::endl;
         
         
         
      }
      
      std::cout << "===================" << std::endl;
      
   }
   
   
//    
}

