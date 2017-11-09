#ifndef Analysis_Core_L1Muon_h
#define Analysis_Core_L1Muon_h 1

// -*- C++ -*-
//
// Package:    Analysis/Core
// Class:      L1Muon
// 
/**\class L1Muon L1Muon.cc Analysis/Core/src/L1Muon.cc

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

      class L1Muon : public Candidate {
         public:
            /// default constructor
            L1Muon();
	    /// constructor from 4-momentum information
            L1Muon(const float & pt, const float & eta, const float & phi, const float & e, const float & q);
	    /// destructor
           ~L1Muon();
//            using Candidate::set; // in case needed to overload the function set
        	   // Gets
        	   int   hwQual()     const ;
        	   float etaAtVtx()   const ; 
        	   float phiAtVtx()   const ; 
        
        	   // Sets
        	   void hwQual  (const int & hwQual);    
            void etaAtVtx(const float & etaAtVtx); 
            void phiAtVtx(const float & phiAtVtx); 
   
          protected:
       // ----------member data ---------------------------
            int   hwQual_;             
            float etaAtVtx_;         
            float phiAtVtx_;        

         private:
            // ----------member data ---------------------------
            
            // 
      };
   }
}

#endif  // Analysis_Core_Muon_h
