#include <string>
#include <iostream>
#include <vector>

#include "TFile.h" 
#include "TFileCollection.h"
#include "TChain.h"
#include "TH1.h" 

#include "Analysis/Tools/interface/Analysis.h"

using namespace std;
using namespace analysis;
using namespace analysis::tools;


// =============================================================================================   
int main(int argc, char * argv[])
{
   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   // Input files list
   std::string inputList = "rootFileList.txt";
   Analysis analysis(inputList);
   
   // Trigger
   analysis.triggerResults("MssmHbb/Events/TriggerResults");
   std::string hltPath = "HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33_v";
   std::vector<std::string> l1seeds;  // in case there is an OR of L1 seeds
   l1seeds.push_back("L1_DoubleJet100er2p3_dEta_Max1p6");
   l1seeds.push_back("L1_DoubleJet112er2p3_dEta_Max1p6");
   
   // Trigger objects
   std::vector<std::string> jetTriggerObjects;
   jetTriggerObjects.push_back("hltL1DoubleJet100er2p3dEtaMax1p6Ior112er2p3dEtaMax1p6");
   jetTriggerObjects.push_back("hltDoubleCaloBJets100eta2p3");
   jetTriggerObjects.push_back("hltDoublePFJets100Eta2p3");
   jetTriggerObjects.push_back("hltBTagCalo80x6CSVp0p92DoubleWithMatching");
   
   std::string trgobj_path = "MssmHbb/Events/slimmedPatTrigger";
   for ( auto & obj : jetTriggerObjects )
      analysis.addTree<TriggerObject>(obj,trgobj_path+"/"+obj);
   
   // Analysis of events
   std::cout << "This analysis has " << analysis.size() << " events" << std::endl;
   int nevts = analysis.size();
   nevts = 10;
   for ( int i = 0 ; i < nevts ; ++i )
   {
      analysis.event(i);
      
      std::cout << "++++++    ENTRY  " << i << std::endl;
      
      // Trigger results: fired and prescales
      // hltPath 
      int trg_fired = analysis.triggerResult(hltPath);
      // prescales
      std::map<std::string,int> l1ps = analysis.triggerPrescale(l1seeds);
      int hltps = analysis.triggerPrescale(hltPath);
      std::string s_accept = " fired ";
      
      // printing info
      if ( ! trg_fired ) s_accept = " did not fire ";
      std::cout << "The path " << hltPath << s_accept << "and has HLT PS = " << hltps << std::endl; 
      std::cout << "and its L1 seeds have the following prescales: " << endl;
      for ( auto & l1 : l1seeds )
         std::cout << "   " << l1 << ": " << l1ps[l1] << std::endl;
      
      // dealing with trigger objects
      auto l1jets = analysis.collection<TriggerObject>("hltL1DoubleJet100er2p3dEtaMax1p6Ior112er2p3dEtaMax1p6");
      for ( int j = 0 ; j < l1jets->size() ; ++j )
      {
         TriggerObject l1jet = l1jets->at(j);
         std::cout << "L1 Jet: pT = " << l1jet.pt() << ", " << l1jet.eta() << ", " << l1jet.phi() << std::endl;
      }
      
   }
   
   
}

