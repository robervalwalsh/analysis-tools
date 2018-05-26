#ifndef Analysis_Tools_RecoTrack_h
#define Analysis_Tools_RecoTrack_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      RecoTrack
// 
/**\class RecoTrack RecoTrack.cc Analysis/Tools/src/RecoTrack.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Roberval Walsh Bastos Rangel
//         Created:  Fri, 25 May 2018 16:00:00 GMT
//
//

// system include files
#include <memory>
// 
// user include files
#include "Analysis/Tools/interface/Candidate.h"
//
// class declaration
//

namespace analysis {
   namespace tools {

      class RecoTrack : public Candidate {
         public:
            /// default constructor
            RecoTrack();
            /// constructor from 3-momentum information
            RecoTrack(const float & px, const float & py, const float & pz, const float & q);
            /// destructor
           ~RecoTrack();
            // Gets

            // Sets
          protected:
          // ----------member data ---------------------------

         private:
         // ----------member data ---------------------------
            
      };
   }
}

#endif  // Analysis_Tools_RecoTrack_h
