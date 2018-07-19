#include <string>
#include <iostream>
#include <vector>

#include "TTree.h" 
#include "TFile.h" 
#include "TH1.h" 

#include "Analysis/Tools/bin/SimpleAnalysisNano.h"

using namespace std;

int SetTreeBranches(TTree *);

// =============================================================================================   
int main(int argc, char * argv[])
{
   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   TFile * nano = new TFile("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-350_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180712_150216/0000/nano_1.root","old");
   
   TTree * tree = (TTree*) nano->Get("Events");
   SetTreeBranches(tree);
   for ( int i = 0; i < 10 ; ++i )
   {
       tree->GetEntry(i);
       std::cout << "Run = " << run_ << " | Lumi Section = " << lumisection_ << " | Event = " << event_ << std::endl;
//       std::cout << " number of gen jets from ngenjet = " << ngenjet_ << "  number of gen jets from vector size = " << (*pgenjet_pt_).size() << std::endl;
       std::cout << " number of gen jets from ngenjet = " << ngenjet_ << std::endl;
       for ( size_t j = 0 ; j < ngenjet_; ++j )
       {
          std::cout << " jet pt = " << genjet_pt_[j] << std::endl;
       }

   }
   
//    
}

int SetTreeBranches(TTree * t)
{

   // Event Info
   t -> SetBranchAddress( "event"           , &event_       );
   t -> SetBranchAddress( "run"             , &run_         );
   t -> SetBranchAddress( "luminosityBlock" , &lumisection_ );
   
   // gen jets
   t -> SetBranchAddress( "nGenJet"     , &ngenjet_      );
   t -> SetBranchAddress( "GenJet_pt"   , genjet_pt_   );
   
   return 0;
}
