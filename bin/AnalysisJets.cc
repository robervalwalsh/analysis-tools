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

   
   // Analysis of events
   std::cout << "This analysis has " << analysis.size() << " events" << std::endl;
   for ( int i = 0 ; i < analysis.size() ; ++i )
   {
      analysis.event(i);
      
      std::cout << "++++++    ENTRY  " << i;
      std::cout << std::endl;
      
      // Jets
      auto jets = analysis.collection<Jet>("Jets");
      for ( int j = 0 ; j < jets->size() ; ++j )
      {
         Jet jet = jets->at(j);
         std::cout << "    Jet #" << j << ": ";
         std::cout << "pT  = "     << jet.pt()      << ", ";
         std::cout << "eta = "     << jet.eta()     << ", ";
         std::cout << "phi = "     << jet.phi()     << ", ";
         std::cout << "flavour = " << jet.flavour() << ", ";
         std::cout << "btag = "    << jet.btag("btag_csvivf")    << std::endl;
         std::cout << "     quark-gluon likelihood = " << jet.qgLikelihood() << std::endl;
         std::cout << "     pileup jet id full discriminant = " << jet.pileupJetIdFullDiscriminant() << std::endl;
         std::cout << "     pileup jet id full id = " << jet.pileupJetIdFullId() << std::endl;
         
      }
      std::cout << "===================" << std::endl;
   }
   
//    
}

