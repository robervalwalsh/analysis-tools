#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "TFile.h" 
#include "TFileCollection.h"
#include "TChain.h"
#include "TH1.h" 

#include "Analysis/Core/interface/Analysis.h"
#include "Analysis/Core/bin/macro_config.h"

using namespace std;
using namespace analysis;
using namespace analysis::core;

// =============================================================================================   
int main(int argc, char * argv[])
{
   
   if ( macro_config(argc, argv) != 0 ) return -1;
   
   TH1::SetDefaultSumw2();  // proper treatment of errors when scaling histograms
   
   // Input files list
   Analysis analysis(inputlist_);
   
   // Ntuples # events
   std::cout << "These ntuples have " << analysis.size() << " events" << std::endl;
   
} //end main

