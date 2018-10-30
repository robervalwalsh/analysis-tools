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
   analysis.addTree<Jet>  ("Jets" ,jetsCol_ );
   analysis.addTree<Muon> ("Muons",muonsCol_);

   // Analysis of events
   std::cout << "This analysis has " << analysis.size() << " events" << std::endl;
   for ( int i = 0 ; i < analysis.size() ; ++i )
   {
      analysis.event(i);
      
      std::cout << "++++++    ENTRY  " << i;
      std::cout << std::endl;
      
      // Jets
      auto jets  = analysis.collection<Jet>("Jets");
      auto muons = analysis.collection<Muon>("Muons");
      
      if ( muons->size() < 1 ) continue;
      if ( jets ->size() < 1 ) continue;
      
      std::shared_ptr<Muon> muon = std::make_shared<Muon>(muons->at(0));
      
      for ( int j = 0 ; j < jets->size() ; ++j )
      {
         std::shared_ptr<Jet> jet = std::make_shared<Jet>(jets->at(j));
         float deltaR = jet->deltaR(*muon);
         
         if ( !jet->muon() && deltaR < 0.4 ) jet->addMuon(muon);
         
         if ( jet->muon() )
         {
            std::cout << "jet eta,phi = " << jet->eta() << ", " << jet->phi() << "     muon eta,phi = " << jet->muon()->eta() << ", " << jet->muon()->phi() << std::endl;
         }
      }
      
      std::cout << "===================" << std::endl;
      
      
   }
   
//    
}
