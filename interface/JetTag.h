#ifndef Analysis_Core_JetTag_h
#define Analysis_Core_JetTag_h 1

// -*- C++ -*-
//
// Package:    Analysis/Core
// Class:      JetTag
// 
/**\class JetTag JetTag.cc Analysis/Core/src/JetTag.cc

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
// 
// user include files
#include "Analysis/Core/interface/Candidate.h"
//
// class declaration
//

namespace analysis {
   namespace core {

      class JetTag : public Candidate {
         public:
            JetTag();
            /// constructor from 4-momentum information
            JetTag(const float & pt, const float & eta, const float & phi, const float & e);
            /// constructor from TLorentzVector
            JetTag(const TLorentzVector & p4);
            
           ~JetTag();
//            using Candidate::set; // in case needed to overload the function set
      
            // Gets
            /// returns the btag value of btag_csvivf
            float btag()                        const;
            
            // Sets
            /// sets the btag value
            void  btag(const float &);
            
         private:
            // ----------member data ---------------------------
            /// btag value 
            float btag_ ;
            
            // 
      };
   }
}

#endif  // Analysis_Core_JetTag_h
