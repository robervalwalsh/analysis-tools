/**\class BaseAnalyser BaseAnalyser.cc Analysis/Tools/src/BaseAnalyser.cc

 Description: A base class for analyser

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Roberval Walsh
//         Created:  6 Sep 2018
//
//

// system include files
#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
// 
// user include files
#include "Analysis/Tools/interface/BaseAnalyser.h"

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

//
// constructors and destructor
//
BaseAnalyser::BaseAnalyser()
{
}


BaseAnalyser::BaseAnalyser(int argc, char * argv[])
{
   exe_ = std::string(argv[0]);
   
   config_   = std::make_shared<Config>(argc,argv);
   analysis_ = std::make_shared<Analysis>(config_->ntuplesList());
   
   seed_ = analysis_ ->seed(config_->seedFile());
   
   weight_ = 1.;
   
   // JSON for data   
   if( !config_->isMC() && config_->json_ != "" ) analysis_->processJsonFile(config_->json_);
   
   // output file
   if ( config_->outputRoot_ != "" )
   {
      hout_= std::make_shared<TFile>(config_->outputRoot_.c_str(),"recreate");
   }
   
   h1_["cutflow"] = std::make_shared<TH1F>("cutflow","", 50,0,50);
      if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(1)) == "" ) 
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(1,"Total events read");
   if ( config_->isMC() )
   {
      if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(2)) == "" ) 
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(2,"Generated weighted events");
   }
   
}

BaseAnalyser::~BaseAnalyser()
{
   std::cout << std::endl;
   cutflow();
   if ( hout_ )
   {
      std::cout << std::endl;
      std::cout << "output root file: " << config_->outputRoot_ << std::endl;
      
      hout_->Write();
      hout_->Close();
   }   
   
//   std::string cutflow = "Cutflow " + config_->outputRoot_;
//   std::system(cutflow.c_str());
   
   std::cout << exe_ << " finished!" << std::endl;
   printf("%s\n", std::string(100,'_').c_str());
   std::cout << std::endl;
}


//
// member functions
//
// ------------ method called for each event  ------------

bool BaseAnalyser::event(const int & i) { return true; }
void BaseAnalyser::histograms(const std::string & s, const int & i) { }

std::shared_ptr<Analysis> BaseAnalyser::analysis()
{
   return analysis_;
}

std::shared_ptr<Config> BaseAnalyser::config()
{
   return config_;
}

int BaseAnalyser::nEvents()
{
   int maxevt = config_->nEventsMax();
   if ( maxevt < 0 ) maxevt = analysis_->size();
   return maxevt;
}

std::shared_ptr<TH1F> BaseAnalyser::histogram(const std::string & hname)
{
   if ( h1_.find(hname) == h1_.end() ) 
   {
      std::cout << "-e- BaseAnalyser::H1F(const string & hname) -> no histogram with hname = " << hname << std::endl;
      return nullptr;
   }
   
   return h1_[hname];
}

std::map<std::string, std::shared_ptr<TH1F> > BaseAnalyser::histograms()
{
   return h1_;
}


void BaseAnalyser::cutflow()
{
   printf("+%s+\n", std::string(150,'-').c_str());
   printf("| %-88s |    %10s |   %16s |   %16s |\n","workflow","n events","ratio wrt first","ratio wrt previous");
   printf("+%s+\n", std::string(150,'-').c_str());
   int firstbin = 2;
   for ( int i = 1; i <= h1_["cutflow"] ->GetNbinsX(); ++i )
   {
      std::string label = std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(i));
      if ( label == "" ) continue;
//      if ( firstbin < 0 ) firstbin = i;
      float n = h1_["cutflow"]->GetBinContent(i);
      float rn1 = h1_["cutflow"]->GetBinContent(i)/h1_["cutflow"]->GetBinContent(firstbin);
      float rni = 0;
      if ( i == 1 )
      {
         printf("| %-88s |    %10.1f |   %16s |  %19s |\n",label.c_str(),n,"-","-");
      }
      else
      {
         rni = h1_["cutflow"]-> GetBinContent(i)/h1_["cutflow"]->GetBinContent(i-1);
         printf("| %2d - %-83s |    %10.1f |   %16.4f |     %16.4f |\n",i-1,label.c_str(),n,rn1,rni);
      }
      
   }
   printf("+%s+\n", std::string(150,'-').c_str());
   
}

int  BaseAnalyser::seed()
{
   return seed_;
}

int  BaseAnalyser::seed(const std::string & f)
{
   seed_ = analysis_ ->seed(f);
   return seed_;
}

void BaseAnalyser::seed(const int & seed)
{
   seed_ = seed;
}

void  BaseAnalyser::weight(const float & w)
{
   weight_ = w;
}

float  BaseAnalyser::weight()
{
   return weight_;
}

