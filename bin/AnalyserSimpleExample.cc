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
   
   // you can always get the ususal analysis class
   auto analysis = analyser.analysis();
   
// HISTOGRAMS   
   // create some predefined jet histograms
   analyser.jetHistograms(2,"dijet");
   // create some predefined muon histograms
   // muon histograms still not available
   
   // get a map with all TH1F histograms pointers
   std::map<std::string, std::shared_ptr<TH1F> > histos = analyser.histograms();

   for ( int i = 0 ; i < analyser.nEvents() ; ++i )
   {
      if ( ! analyser.event(i)                  )   continue;
      
// TRIGGER selection
      if ( ! analyser.selectionHLT()            )   continue;
      if ( ! analyser.selectionL1 ()            )   continue;  // to be used mainly in case of "OR" of seeds
      
// MUONS pre-selection
      // muon identification selection
      if ( ! analyser.selectionMuonId()         )   continue;
      if ( ! analyser.selectionNMuons()         )   continue;
      // if you need the selected muons explicitly - be careful where you use it
      std::vector< std::shared_ptr<Muon> > selmuon = analyser.selectedMuons();
        
// JETS pre-selection 
      // jet identification selection
      if ( ! analyser.selectionJetId()          )   continue;
      if ( ! analyser.selectionJetPileupId()    )   continue;
      if ( ! analyser.selectionNJets()          )   continue;
      // if you need the selected jets explicitly - be careful where you use it
      std::vector< std::shared_ptr<Jet> > seljets = analyser.selectedJets();
      
// CORRECTIONS to pre-selected jets
   // b energy regression
      if ( analyser.config()->bRegression() )
         analyser.actionApplyBjetRegression();
      
   // MC-only jet corrections
      if ( analyser.config()->isMC() )
      {
      // apply btag SF
         analyser.actionApplyBtagSF(1);
         analyser.actionApplyBtagSF(2);
         
      // jet energy resolution to all selected jets
         analyser.actionApplyJER();
      }
      
// MAIN SELECTION
   // JETS
      //  1st and 2nd jet kinematic selection, pt and eta
      if ( ! analyser.selectionJet(1)          )   continue;
      if ( ! analyser.selectionJet(2)          )   continue;
      // delta R jet selection
      if ( ! analyser.selectionJetDr(1,2)      )   continue;
      // btag of two leading jets
      if ( ! analyser.selectionBJet(1)         )   continue;
      if ( ! analyser.selectionBJet(2)         )   continue;
      // jets 1 and 2 matching to online jets
      if ( ! analyser.onlineJetMatching(1)     )   continue;
      if ( ! analyser.onlineJetMatching(2)     )   continue;
      // dije mass selection
      if ( ! analyser.selectionDiJetMass(1,2)  )   continue;
      
   // MUON
      // muon kinematic selection
      if ( ! analyser.selectionMuons()         )   continue;
      // muon trigger matching - at least nmin offline muons matched to online objects
      if ( ! analyser.onlineMuonMatching()     )   continue;
      
   // MUONJET
      if ( ! analyser.muonJet(1)               )   continue;

// HISTOGRAMS
      analyser.fillJetHistograms("dijet");
      
   }  //end event loop
} // end main
      
