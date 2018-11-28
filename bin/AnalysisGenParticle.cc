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
   
   // btag
   // Physics Objects Collections
   analysis.addTree<GenParticle> ("GenParticles",genParticleCol_);

   // Analysis of events
   std::cout << "This analysis has " << analysis.size() << " events" << std::endl;
   
   int nevts = analysis.size();
   if ( nevtmax_ > 0 ) nevts = nevtmax_;
   
   for ( int i = 0 ; i < nevts ; ++i )
   {
      analysis.event(i);
      
      std::cout << "++++++    ENTRY  " << i;
      std::cout << std::endl;
      
      // Jets
      auto genps = analysis.collection<GenParticle>("GenParticles");
      
      printf("| %10s | %10s | %10s | %10s | %10s | %10s | %10s |\n","index","pdg","status","mo1","mo2","da1","da2");
      for ( int j = 0 ; j < genps->size() ; ++j )
      {
         GenParticle genp  = genps -> at(j);
         
         int indx = genp.index();
         int mo1 = genp.mother(1);
         int mo2 = genp.mother(2);  
         int da1 = genp.daughter(1);
         int da2 = genp.daughter(2);  
         int status = genp.status();
         int pdg = genp.pdgId();
         printf("| %10d | %10d | %10d | %10d | %10d | %10d | %10d |\n",indx,pdg,status,mo1,mo2,da1,da2);      
      }
      
      std::cout << "===================" << std::endl;
      
   }
   
//    
}
