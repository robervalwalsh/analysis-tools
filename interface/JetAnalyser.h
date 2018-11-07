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
            std::vector< std::shared_ptr<Jet> > jets_;
            std::vector< std::shared_ptr<Jet> > selectedJets_;
            
            // number of histogrammed jets
            int n_hjets_;
            
            bool jetsanalysis_;
            
            std::map<std::string, std::shared_ptr<BTagCalibrationReader> >bsf_reader_;
         
         private:
               
         public:
            std::vector< std::shared_ptr<Jet> > selectedJets();
            std::vector< std::shared_ptr<Jet> > jets();
            float btag(const Jet & , const std::string & );
         
            // Sets
            void jets(const std::string &);
            
            // Actions
            virtual bool analysisWithJets();
            virtual bool selectionJet(const int &);
            virtual bool selectionJetDeta(const int &, const int &, const float &);
            virtual bool selectionJetDeta(const int &, const int &);
            virtual bool selectionJetDr(const int &, const int &, const float &);
            virtual bool selectionJetDr(const int &, const int &);
            virtual bool selectionJetId();
            virtual bool selectionJetPileupId();
            virtual bool selectionNJets();
            virtual bool selectionBJet(const int &);
            virtual ScaleFactors btagSF(const int &, const std::string &);
            virtual bool selectionNonBJet(const int &);
            virtual bool onlineJetMatching(const int &);
            virtual bool onlineBJetMatching(const int &);
            virtual void jetHistograms(const int & n = 1, const std::string & label = "x");
            virtual void fillJetHistograms(const std::string & label = "x");

      };
   }
}

#endif  // Analysis_Tools_JetAnalyser_h
