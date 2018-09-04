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
#include "Analysis/Tools/interface/JetResolution.h"

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
   JME::JetResolution resolution = JME::JetResolution("/afs/desy.de/user/w/walsh/cms/analysis/cmssw/mssmhbb/2017/CMSSW_9_4_9/src/Analysis/Tools/data/Summer16_25nsV1_MC_PtResolution_AK4PFchs.txt");
   JME::JetResolutionScaleFactor resolution_sf = JME::JetResolutionScaleFactor("/afs/desy.de/user/w/walsh/cms/analysis/cmssw/mssmhbb/2017/CMSSW_9_4_9/src/Analysis/Tools/data/Summer16_25nsV1_MC_SF_AK4PFchs.txt");

   // Analysis of events
   std::cout << "This analysis has " << analysis.size() << " events" << std::endl;
   for ( int i = 0 ; i < analysis.size() ; ++i )
   {
      analysis.event(i);
      
      std::cout << "++++++    ENTRY  " << i;
      std::cout << std::endl;
      
      analysis.match<Jet,GenJet>("Jets","GenJets",0.2);
      
      // Jets
      auto jets = analysis.collection<Jet>("Jets");
      auto genjets = analysis.collection<GenJet>("GenJets");

      for ( int j = 0 ; j < jets->size() ; ++j )
      {
         Jet jet = jets->at(j);
         auto * genjet = jet.matched("GenJets");
         
         JME::JetParameters jetResPars = {{JME::Binning::JetPt, jet.pt()}, {JME::Binning::JetEta, jet.eta()}, {JME::Binning::Rho, jet.rho()}};
         JME::JetParameters jetResSFPars = {{JME::Binning::JetEta, jet.eta()}, {JME::Binning::Rho, jet.rho()}};;
     
         std::cout << "    Jet #" << j << ": ";
         std::cout << "pT  = "     << jet.pt()      << ", ";
         std::cout << "eta = "     << jet.eta()     << ", ";
         std::cout << "resolution = " << resolution.getResolution(jetResPars) << ", ";
         std::cout << "JER SF  = "    << resolution_sf.getScaleFactor(jetResSFPars)  << ", " << genjet << std::endl;
         
         
         
      }
      
      std::cout << "===================" << std::endl;
      
   }
   
   
//    
}

