#ifndef Analysis_Tools_L1Jet_h
#define Analysis_Tools_L1Jet_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      L1Jet
// 
/**\class L1Jet L1Jet.cc Analysis/Tools/src/L1Jet.cc

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
#include "Analysis/Tools/interface/Candidate.h"
//
// class declaration
//

namespace analysis {
   namespace tools {

      class L1Jet : public Candidate {
         public:
            /// default constructor
            L1Jet();
	    /// constructor from 4-momentum information
            L1Jet(const float & pt, const float & eta, const float & phi, const float & e, const float & q);
	    /// destructor
           ~L1Jet();
//            using Candidate::set; // in case needed to overload the function set
        	   // Gets
        
        	   // Sets
   
          protected:
       // ----------member data ---------------------------

         private:
            // ----------member data ---------------------------
            
            // 
      };
   }
}

#endif  // Analysis_Tools_Jet_h
