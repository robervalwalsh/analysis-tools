#ifndef Analysis_Tools_Config_h
#define Analysis_Tools_Config_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      Analysis
// 
/**\class Analysis Config.cc Analysis/Tools/src/Config.cc

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

      struct Options
      {
         po::options_description cmd;
         po::options_description cfg;
      };
      
      class Config {
         public:
            /// constructors
            Config();
            Config(int argc, char ** argv);
            /// desctructor
           ~Config();
            
            po::options_description & optionsCMD();
            po::options_description & optionsCFG();
            
            void loadOptions();
           
           
            // ----------member data ---------------------------
         protected:
            po::options_description opt_cmd_;
            po::options_description opt_cfg_;
            
         private:
               
         public:
            // configuration variables - some basic ones out in the private section and make set/get methods
            std::string cfg_;
         
         // analysis info
            std::string inputlist_;
            std::string treePath_;
            int nevtmax_;
            int nlumis_;
            int runmin_;
            int runmax_;
            std::string outputRoot_;
            std::string json_;
            bool isMC_;
            bool signalregion_;
            
         // analysis control
            bool override_;

            //
            float trgmatchdrmax_;


            // jets

            int njetsmin_;
            int njetsmax_;
            int nbjetsmin_;
            std::vector<float> jetsptmin_;
            std::vector<float> jetsptmax_;
            std::vector<float> jetsetamax_;
            std::vector<float> jetsbtagmin_;
            std::string jetsid_;
            std::string jetspuid_;

            // btag SF csv file
            std::string btagsf_;


            // muons

            int nmuonsmin_;
            std::vector<float> muonsptmin_;
            std::vector<float> muonsptmax_;
            std::vector<float> muonsetamax_;
            std::string muonsid_;


            // additional cuts of unidentified objects or for extra selections
            int nmin_;
            int nmax_;
            std::vector<float> ptmin_;
            std::vector<float> ptmax_;
            std::vector<float> etamax_;


            float drmin_;
            float drmax_;
            float detamax_;
            float dphimin_;

            float ptimbalmax_;

            std::string btagalgo_;
            std::string btagwp_;
            float btagwploose_;
            float btagwpmedium_;
            float btagwptight_;
            //float btagwp_;
            float nonbtagwp_;
            int nonbtagjet_;


            std::string hltPath_;
            std::string l1Seed_;
            std::vector<std::string> triggerObjects_;
            std::vector<int> triggerObjectsMatches_;
            std::vector<int> triggerObjectsMatchesRank_;
            std::vector<std::string> triggerObjectsJets_;
            std::vector<std::string> triggerObjectsBJets_;
            std::vector<std::string> triggerObjectsL1Jets_;
            int triggerObjectsJetsMatches_;
            int triggerObjectsBJetsMatches_;
            int triggerObjectsL1JetsMatches_;
            std::vector<std::string> triggerObjectsMuons_;
            std::vector<std::string> triggerObjectsL1Muons_;
            int triggerObjectsMuonsMatches_;
            int triggerObjectsL1MuonsMatches_;

            // ntuples collections
            std::string jetsCol_; 
            std::string muonsCol_; 
            std::string l1tjetsCol_; 
            std::string l1tmuonsCol_; 
            std::string triggerCol_;
            std::string genParticleCol_;
            std::string genjetsCol_;
            std::string triggerObjDir_;

         protected:
            int argc_;
            char ** argv_;
         
      };
   }
}

#endif  // Analysis_Tools_Config_h
