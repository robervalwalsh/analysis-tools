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


float btagMin(const std::string & btagwp);

// =============================================================================================   
int main(int argc, char * argv[])
{
   if ( macro_config(argc, argv) != 0 ) return -1;

   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   // Input files list
   Analysis analysis(inputlist_);
   
   // btag
   float btagmin = btagMin(btagwp_);
   
   // b-tag scale factors
   // inputs from the config file test/analysis_bjetsv2.cfg
   // btagalgo_ = "deepcsv";
   // btagsf_   = "../data/DeepCSV_94XSF_V3_B_F.csv";
   // btagwp_   = "tight";
   auto bsf_reader = analysis.btagCalibration(btagalgo_, btagsf_, btagwp_);
   
   // Physics Objects Collections
   analysis.addTree<Jet> ("Jets",jetsCol_);

   // Analysis of events
   std::cout << "This analysis has " << analysis.size() << " events" << std::endl;
   int nevts = analysis.size();
   if ( nevts > 0 ) nevts = nevtmax_;
   for ( int i = 0 ; i < nevts ; ++i )
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
         if ( btagalgo_ != "deepcsv" && btagalgo_ != "deepflavour" ) continue;
         float btag = -10000.;
         if ( btagalgo_ == "deepcsv" )
            btag = jet.btag("btag_deepb") + jet.btag("btag_deepbb");
         if ( btagalgo_ == "deepflavour" )
            btag = jet.btag("btag_dfb") + jet.btag("btag_dfbb") + jet.btag("btag_dflepb");
         
         if ( btag < btagmin ) continue;  // probing btag jets
         
         ++nbjets;
         
         // b-tag scale factors central, up and down
         double jet_bscalefactor     = jet.btagSF(bsf_reader); // OR jet.btagSF(analysis.btagCalibration()); 
         double jet_bscalefactorup   = jet.btagSFup(bsf_reader,2); 
         double jet_bscalefactordown = jet.btagSFdown(bsf_reader,2); 
     
         std::cout << "    Jet #" << j << ": ";
         std::cout << "pT  = "     << jet.pt()      << ", ";
         std::cout << "eta = "     << jet.eta()     << ", ";
         std::cout << "flavour = " << jet.flavour() << ", ";
         std::cout << "btag = "    << jet.btag("btag_deepb") + jet.btag("btag_deepbb")    << " with scale factor = " <<  jet_bscalefactor;
         std::cout << " up  = " << jet_bscalefactorup << "  down = " << jet_bscalefactordown << std::endl;
      }
      
      std::cout << "===================" << std::endl;
      
      if ( nbjets < nbjetsmin_ ) continue;
      
   }
   
   std::cout << "Btag WP = " << btagwp_ << std::endl;
//    
}

float btagMin(const string & wp)
{
   float min = -1000;
   if ( wp == "loose" )  min = btagwploose_;
   if ( wp == "medium" ) min = btagwpmedium_;
   if ( wp == "tight" )  min = btagwptight_;
   
   return min;
   
}


// // Below is a snippet of how to use the BTV standalone classes
// 
// // before the event loop
//    // b-tag scale factors
//    BTagCalibration calib(btagalgo_, btagsf_ );
//    BTagCalibrationReader reader(BTagEntry::OP_TIGHT,   // operating point - example for TIGHT
//                                 "central",             // central sys type
//                                 {"up", "down"});       // other sys types
//    reader.load(calib,             // calibration instance
//             BTagEntry::FLAV_B,    // btag flavour - B
//             "comb");              // measurement type   
//    reader.load(calib,             // calibration instance
//             BTagEntry::FLAV_C,    // btag flavour - C
//             "comb");              // measurement type   
//    reader.load(calib,             // calibration instance
//             BTagEntry::FLAV_UDSG, // btag flavour - UDSG
//             "incl");              // measurement type   
//    
// // inside the event loop
//    
//          // b-tag scale factors
//          double jet_bscalefactor;
//          if ( jet.flavour("Hadron") == 5 ) jet_bscalefactor = reader.eval_auto_bounds("central", BTagEntry::FLAV_B,    fabs(jet.eta()), jet.pt() ); 
//          if ( jet.flavour("Hadron") == 4 ) jet_bscalefactor = reader.eval_auto_bounds("central", BTagEntry::FLAV_C,    fabs(jet.eta()), jet.pt() ); 
//          if ( jet.flavour("Hadron") == 0 ) jet_bscalefactor = reader.eval_auto_bounds("central", BTagEntry::FLAV_UDSG, fabs(jet.eta()), jet.pt() ); 
//    
