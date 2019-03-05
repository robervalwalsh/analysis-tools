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
            
         // analysis control
            std::string inputlist_;
            int nevtmax_;
            bool isMC_;
            bool signalregion_;
            bool blind_;
            bool override_;
            bool nlo_;
            int workflow_;
            
            int seed_;
            std::string seedfile_;
            
            bool pythia8_;


         // jets
            std::string jetsCol_; 
            int njetsmin_;
            int njetsmax_;
            std::vector<float> jetsptmin_;
            std::vector<float> jetsptmax_;
            std::vector<float> jetsetamax_;
            std::string jetsid_;
            std::string jetspuid_;
            // JER resolution and scale factors from txt file
            std::string jerptres_;
            std::string jersf_;
            //
            std::string l1tjetsCol_; 
            //
            std::vector<std::string> jetsbtagwp_;
            std::string nonbtagwp_;
            int nonbtagjet_;
            //
            std::vector<std::string> triggerObjectsJets_;
            bool usejetsflv_;
            bool usejetsextflv_;
            bool dodijet_;
            
            bool bregression_;


         // muons
            std::string muonsCol_; 
            int nmuonsmin_;
            int nmuonsmax_;
            std::vector<float> muonsptmin_;
            std::vector<float> muonsptmax_;
            std::vector<float> muonsetamax_;
            std::string muonsid_;
            std::string l1tmuonsCol_; 
            
         // trigger
            std::string triggerCol_;

         // generator level collections
            std::string genjetsCol_;
            std::string genpartsCol_;
            


         // btag
            float btagwploose_;
            float btagwpmedium_;
            float btagwptight_;
            float btagwpxxx_;
            
         // general
            float massmin_;
            float massmax_;

                        
         private:
               
         public:
            // configuration variables - some basic ones out in the private section and make set/get methods
            std::string cfg_;
         
         // gets (to replace the public variables)
         // analysis control
            std::string ntuplesList() const;
            int  nEventsMax() const;
            bool isMC() const;
            bool signalRegion() const;
            bool blind() const;
            bool override() const;
            bool nlo() const;
            int  workflow() const;
            
            std::string seedFile() const;
            int seed() const;
            
            bool pythia8() const;
            
         // jets
            std::string jetsCollection() const;
            int nJetsMin() const;
            int nJetsMax() const;
            std::vector<float> jetsPtMin() const;
            std::vector<float> jetsPtMax() const;
            std::vector<float> jetsEtaMax() const;
            std::string jetsId() const;
            std::string jetsPuId() const;
            std::string jerPtRes() const;
            std::string jerSF() const;
            std::string l1tJetsCollection() const; 
            std::vector<std::string> jetsBtagWP() const;
            bool bRegression() const;
            std::string nonBtagWP()  const;
            int   nonBtagJet() const;
            std::vector<std::string> triggerObjectsJets() const;
            void triggerObjectsJets(const std::string & label, const int & index);
            bool useJetsFlavour() const;
            bool useJetsExtendedFlavour() const;
            bool doDijet() const;
            
         // muons
            std::string muonsCollection() const;
            int nMuonsMin() const;
            int nMuonsMax() const;
            std::vector<float> muonsPtMin() const;
            std::vector<float> muonsPtMax() const;
            std::vector<float> muonsEtaMax() const;
            std::string muonsId() const;
            std::string l1tMuonsCollection() const; 
            
         // trigger
            std::string triggerResults() const;
            
         // generator level
            std::string genJetsCollection() const;
            std::string genParticlesCollection() const;
            
         // btag
            float btagWP(const std::string &) const;
            
         // general
            float massMin() const;
            float massMax() const;
            
         // ========================
         
         // analysis control
            std::string treePath_;
            int nlumis_;
            int runmin_;
            int runmax_;
            std::string outputRoot_;
            std::string json_;
            

            //
            float trgmatchdrmax_;


            // btag SF csv file
            std::string btagsf_;
            int nbjetsmin_;
            std::vector<float> jetsbtagmin_;




            // additional cuts of unidentified objects or for extra selections
            int nmin_;
            int nmax_;
            std::vector<float> ptmin_;
            std::vector<float> ptmax_;
            std::vector<float> etamax_;


            float drmin_;
            float drmax_;
            float detamax_;
            float detamin_;
            float dphimin_;
            float dphimax_;

            float ptimbalmax_;
            float ptimbalmin_;
            
            float qgmin_;
            float qgmax_;

            std::string btagalgo_;
            std::string btagwp_;
            //float btagwp_;


            std::string hltPath_;
            std::string l1Seed_;
            std::vector<std::string> triggerObjects_;
            std::vector<int> triggerObjectsMatches_;
            std::vector<int> triggerObjectsMatchesRank_;
            std::vector<std::string> triggerObjectsBJets_;
            std::vector<std::string> triggerObjectsL1Jets_;
            int triggerObjectsJetsMatches_;
            int triggerObjectsBJetsMatches_;
            int triggerObjectsL1JetsMatches_;
            std::vector<std::string> triggerObjectsMuons_;
            std::string triggerObjectsL1Muons_;
            std::string triggerObjectsL3Muons_;
            int triggerObjectsMuonsMatches_;
            int triggerObjectsL1MuonsMatches_;
            int triggerObjectsL3MuonsMatches_;

            // ntuples collections
            std::string triggerObjDir_;

         protected:
            int argc_;
            char ** argv_;
         
      };
   }
}

#endif  // Analysis_Tools_Config_h
