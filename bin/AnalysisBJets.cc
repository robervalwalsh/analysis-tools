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
#include "Analysis/Tools/interface/BTagCalibrationStandalone.h"

#include "Analysis/Tools/bin/macro_config.h"

using namespace std;
using namespace analysis;
using namespace analysis::tools;


// =============================================================================================   
int main(int argc, char * argv[])
{
   if ( macro_config(argc, argv) != 0 ) return -1;
   std::transform(btagalgo_.begin(), btagalgo_.end(), btagalgo_.begin(), ::tolower);

   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   // Input files list
   Analysis analysis(inputlist_);
   
   // b-tag scale factors
   BTagCalibration calib(btagalgo_, btagsf_ );
   BTagCalibrationReader reader(BTagEntry::OP_TIGHT,   // operating point
                                "central",             // central sys type
                                {"up", "down"});       // other sys types
   reader.load(calib,             // calibration instance
            BTagEntry::FLAV_B,    // btag flavour - B
            "comb");              // measurement type   
   reader.load(calib,             // calibration instance
            BTagEntry::FLAV_C,    // btag flavour - C
            "comb");              // measurement type   
   reader.load(calib,             // calibration instance
            BTagEntry::FLAV_UDSG, // btag flavour - UDSG
            "incl");              // measurement type   
   
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
      int nbjets = 0;
      for ( int j = 0 ; j < jets->size() ; ++j )
      {
         Jet jet = jets->at(j);
         if ( btagalgo_ != "deepcsv" ) continue;
         if ( (jet.btag("btag_deepb") + jet.btag("btag_deepbb")) < jetsbtagmin_[0] ) continue;  // probing btag DeepCSV jets
         
         ++nbjets;
         
         // b-tag scale factors
         double jet_bscalefactor;
         if ( jet.flavour("Hadron") == 5 ) jet_bscalefactor = reader.eval_auto_bounds("central", BTagEntry::FLAV_B,    fabs(jet.eta()), jet.pt() ); 
         if ( jet.flavour("Hadron") == 4 ) jet_bscalefactor = reader.eval_auto_bounds("central", BTagEntry::FLAV_C,    fabs(jet.eta()), jet.pt() ); 
         if ( jet.flavour("Hadron") == 0 ) jet_bscalefactor = reader.eval_auto_bounds("central", BTagEntry::FLAV_UDSG, fabs(jet.eta()), jet.pt() ); 
     
         std::cout << "    Jet #" << j << ": ";
         std::cout << "pT  = "     << jet.pt()      << ", ";
         std::cout << "eta = "     << jet.eta()     << ", ";
         std::cout << "flavour = " << jet.flavour() << ", ";
         std::cout << "btag = "    << jet.btag()    << " with scale factor = " <<  jet_bscalefactor << std::endl;
      }
      
      std::cout << "===================" << std::endl;
      
      if ( nbjets < nbjetsmin_ ) continue;
      
   }
   
   
//    
}

