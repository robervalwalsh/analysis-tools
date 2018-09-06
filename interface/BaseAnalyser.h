#ifndef Analysis_Tools_BaseAnalyser_h
#define Analysis_Tools_BaseAnalyser_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      Analysis
// 
/**\class Analysis BaseAnalyser.cc Analysis/Tools/src/BaseAnalyser.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Roberval Walsh Bastos Rangel
//         Created:  Mon, 20 Oct 2014 14:24:08 GMT
//
//

// system include files
#include <memory>
#include <vector>
#include <string>
// 
// user include files
#include "boost/program_options.hpp"

#include "Analysis/Tools/interface/Analysis.h"
#include "Analysis/Tools/interface/Config.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"


namespace po = boost::program_options;
using TH1s = std::map<std::string, TH1F*>;
using TH2s = std::map<std::string, TH2F*>;

//
// class declaration
//

namespace analysis {
   namespace tools {

      class BaseAnalyser {
         
         public:
            /// constructors
            BaseAnalyser();
            BaseAnalyser(int argc, char * argv[]);
            /// desctructor
           ~BaseAnalyser();
           
         
            // ----------member data ---------------------------
         protected:
            std::shared_ptr<Analysis> analysis_;
            std::shared_ptr<Config> config_;
            
            int cutflow_;
            
            TH1s h1_;
            TH2s h2_;
         
         private:
               
            std::shared_ptr<TFile> hout_;
         
            std::string exe_;
            
         public:
            // Gets
            std::shared_ptr<Analysis> analysis();
            std::shared_ptr<Config>   config();
            TH1s histograms();
            TH1F * histogram(const std::string &);
            int nEvents();
            
            void cutflow();

            
            // Actions
            virtual bool event(const int &);
            virtual void histograms(const std::string &, const int &);
               

      };
   }
}

#endif  // Analysis_Tools_BaseAnalyser_h
