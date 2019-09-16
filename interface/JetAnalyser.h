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
            bool applyjer_;
            
            std::map<std::string, std::shared_ptr<BTagCalibrationReader> >bsf_reader_;
            
            std::shared_ptr<JetResolutionInfo> jerinfo_;
            
            std::vector<std::string> flavours_;
         
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
            virtual bool selectionJetDphi(const int &, const int &, const float &);
            virtual bool selectionJetDphi(const int &, const int &);
            virtual bool selectionJetDr(const int &, const int &, const float &);
            virtual bool selectionJetDr(const int &, const int &);
            virtual bool selectionJetPtImbalance(const int &, const int &, const float &);
            virtual bool selectionJetPtImbalance(const int &, const int &);
            virtual bool selectionJetId();
            virtual bool selectionJetPileupId();
            virtual bool selectionNJets();
            virtual bool selectionDiJetMass(const int &, const int &);
            virtual bool selectionBJet(const int &);
            virtual bool selectionBJetProbB(const int &);
            virtual bool selectionBJetProbBB(const int &);
            virtual bool selectionBJetProbLepB(const int &);
            virtual bool selectionBJetProbC(const int &);
            virtual bool selectionBJetProbG(const int &);
            virtual bool selectionBJetProbLight(const int &);
            virtual ScaleFactors btagSF(const int &, const std::string &);
            virtual bool selectionNonBJet(const int &);
            virtual bool onlineJetMatching(const int &);
            virtual bool onlineBJetMatching(const int &);
            virtual void jetHistograms(const int & n, const std::string & label = "x");
            virtual void jetHistograms(const std::string & label = "x");
            virtual void fillJetHistograms(const std::string & label = "x");
            virtual void fillJetHistograms(const int & r = -1, const std::string & label = "x", const float & sf = 1.,const bool & workflow = false);
            virtual void actionApplyJER();
            virtual float actionApplyBtagSF(const int &, const bool & global_weight = true);
            virtual float getBtagSF(const int &);
            virtual void actionApplyBjetRegression();
            virtual void jetSwap(const int &, const int &);
            virtual bool selectionJetQGlikelihood(const int &, const float &);
            virtual bool selectionJetQGlikelihood(const int &);

      };
   }
}

#endif  // Analysis_Tools_JetAnalyser_h
