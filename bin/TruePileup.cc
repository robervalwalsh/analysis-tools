#include "Analysis/Tools/interface/Analyser.h"

// this example is a dijet analysis in which the leading jet
// is required to have a muon, and both of the leading
// jets are btagged

using namespace std;
using namespace analysis;
using namespace analysis::tools;

int main(int argc, char ** argv)
{
   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   Analyser analyser(argc,argv);
   
   analyser.pileupHistogram();
   
   // events loop
   for ( int i = 0 ; i < analyser.nEvents() ; ++i )
   {
      analyser.event(i);
      analyser.fillPileupHistogram();
      
   }  //end events loop
   
} // end main
      
