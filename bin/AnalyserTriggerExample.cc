#include "Analysis/Tools/interface/Analyser.h"

using namespace std;
using namespace analysis;
using namespace analysis::tools;

int main(int argc, char ** argv)
{
   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   Analyser trigger(argc,argv);

   for ( int i = 0 ; i < trigger.nEvents() ; ++i )
   {
      if ( ! trigger.event(i) ) continue;
      // trigger selection
      if ( ! trigger.selectionHLT() )   continue;
      if ( ! trigger.selectionL1 () )   continue;  // to be used in case of "OR" of seeds
      
      // L1 jets trigger objects
      auto l1jets = trigger.triggerObjectsL1Jets();
      for ( auto jet : l1jets )
      {
         std::cout << jet->pt() << std::endl;
      }
      std::cout << "----" << std::endl;
   }
   
   return 0;
}
