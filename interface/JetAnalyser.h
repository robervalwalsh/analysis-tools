#ifndef Analysis_Tools_JetAnalyser_h
#define Analysis_Tools_JetAnalyser_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      Analysis
// 
/**\class Analysis JetAnalyser.cc Analysis/Tools/src/JetAnalyser.cc

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
#include "Analysis/Tools/interface/BaseAnalyser.h"
//
// class declaration
//

namespace analysis {
   namespace tools {

      class JetAnalyser  : virtual public analysis::tools::BaseAnalyser {
         
         public:
            /// constructors
            JetAnalyser();
            JetAnalyser(int argc, char * argv[]);
            /// desctructor
           ~JetAnalyser();
           
            // ----------member data ---------------------------
         protected:
            std::vector<Jet*> jets_;
            std::vector<Jet*> selectedJets_;
            
            bool jetsanalysis_;
         
         private:
               
         public:
            std::vector<Jet*> selectedJets();
            std::vector<Jet*> jets();
            float btag(const Jet & , const std::string & );
         
            // Sets
            void jets(const std::string &);
            
            // Actions
            virtual bool analysisWithJets();
            virtual bool selectionJet();
            virtual bool selectionJet(const int &);
            virtual bool selectionJetDeta(const int &, const int &, const float &);
            virtual bool selectionJetDeta(const int &, const int &);
            virtual bool selectionJetDr(const int &, const int &, const float &);
            virtual bool selectionJetDr(const int &, const int &);
            virtual bool selectionJetId();
            virtual bool selectionJetPileupId();
            virtual bool selectionNJets();
            virtual bool selectionBJet();
            virtual bool selectionBJet(const int &);
            virtual bool selectionNonBJet(const int &);
            virtual bool onlineJetMatching();
            virtual bool onlineJetMatching(const int &);
            virtual bool onlineBJetMatching();
            virtual bool onlineBJetMatching(const int &);
            virtual void histograms(const std::string &, const int & n = 1);
            virtual void fillJetHistograms();

      };
   }
}

#endif  // Analysis_Tools_JetAnalyser_h
