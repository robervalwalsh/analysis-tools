#include "Analysis/Tools/interface/Analyser.h"

// this example has no selection
// for MC it will apply the generator weights

using namespace std;
using namespace analysis;
using namespace analysis::tools;

int main(int argc, char ** argv)
{
   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   Analyser analyser(argc,argv);
   
// HISTOGRAMS   
   // create some predefined jet histograms
   // if not defined, the number of jets is nJetMin from the configurations
   analyser.jetHistograms("selection01");
   analyser.jetHistograms("selection02");
   
   // you can always get the ususal analysis class
   auto analysis = analyser.analysis();
   
   for ( int i = 0 ; i < analyser.nEvents() ; ++i )
   {
      if ( ! analyser.event(i)                  )   continue;
      
   // JETS
      // jet energy resolution to all selected jets
      analyser.actionApplyJER();
      // jet identification selection
      if ( ! analyser.selectionJetId()          )   continue;
      if ( ! analyser.selectionJetPileupId()    )   continue;
      // number of jets selection
      if ( ! analyser.selectionNJets()          )   continue;
      //  1st and 2nd jet kinematic selection, pt and eta
      if ( ! analyser.selectionJet(1)           )   continue;
      if ( ! analyser.selectionJet(2)           )   continue;
      // delta phi jet selection
//      if ( ! analyser.selectionJetDphi(1,2)    )   continue;
// HISTOGRAMS
      analyser.fillJetHistograms("selection01");
      
      
   }  //end event loop
} // end main
      
