#ifndef Analysis_Tools_Analyser_h
#define Analysis_Tools_Analyser_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      Analysis
// 
/**\class Analysis Analyser.cc Analysis/Tools/src/Analyser.cc

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

      class Analyser {
         
         public:
            /// constructors
//            Analyser(const std::string & inputFilelist, const std::string & evtinfo = "MssmHbb/Events/EventInfo");
//            Analyser(Analysis *);
            Analyser();
            Analyser(int argc, char * argv[]);
            /// desctructor
           ~Analyser();
           
         
            // ----------member data ---------------------------
         protected:
            std::shared_ptr<Analysis> analysis_;
            std::shared_ptr<Config> config_;
            
            std::vector<Jet*> jets_;
            std::vector<Jet*> selectedJets_;
            std::vector<Muon*> selectedMuons_;
            
            bool jetsanalysis_;
            bool muonsanalysis_;
            
            TH1s h1_;
            TH2s h2_;
         
         private:
               
            std::shared_ptr<TFile> hout_;
            
         public:
            // Gets
            std::shared_ptr<Analysis> analysis();
            std::shared_ptr<Config>   config();
            std::vector<Jet*> selectedJets();
            std::vector<Jet*> jets();
            std::vector<Muon*> selectedMuons();
            
            float btag(const Jet & , const std::string & );
         
            // Sets
            void jets(const std::string &);
            void triggerResults(const std::string &);
            void triggerObjects(const std::string &,const std::string &);
            
            // Gets
            TH1s H1Fs();
            TH1F * H1F(const std::string &);

            
            // Actions
            virtual void analyse();
            virtual void end();
            virtual bool event(const int &);
            virtual bool analysisWithJets();
            virtual bool selectionTrigger();
            virtual bool selectionJet();
            virtual bool selectionJet(const int &);
            virtual bool selectionJetDeta(const int &, const int &, const float &);
            virtual bool selectionJetDr(const int &, const int &, const float &);
            virtual bool selectionJetId();
            virtual bool selectionBJet();
            virtual bool selectionBJet(const int &);
            virtual bool selectionNonBJet(const int &);
            virtual bool selectionMuon();
            virtual bool onlineJetMatching();
            virtual bool onlineBJetMatching();
            virtual void histograms(const std::string &, const int & n = 1);
            virtual void fillJetHistograms();
            virtual void fillMuonHistograms();
            int nEvents();
               

      };
   }
}

#endif  // Analysis_Tools_Analyser_h
