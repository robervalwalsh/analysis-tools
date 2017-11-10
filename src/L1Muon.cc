 /**\class L1Muon L1Muon.cc Analysis/Tools/src/L1Muon.cc
 
Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Roberval Walsh Bastos Rangel
//         Created:  Mon, 09 Sep 2015 09:24:08 GMT
//
//

// system include files
//
// user include files
#include "FWCore/Framework/interface/Event.h"
//
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "Analysis/Tools/interface/L1Muon.h"


//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

//
// constructors and destructor
//
L1Muon::L1Muon() : Candidate()
{
}
L1Muon::L1Muon(const float & pt, const float & eta, const float & phi, const float & e, const float & q) :
  Candidate(pt,eta,phi,e,q)
{
}
L1Muon::~L1Muon()
{
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

// Gets 

// pat muons
int    L1Muon::hwQual()     const { return hwQual_  ; }
float  L1Muon::etaAtVtx()   const { return etaAtVtx_; }
float  L1Muon::phiAtVtx()   const { return phiAtVtx_; }

// Sets

void L1Muon::hwQual   (const int   & hwQual)    { hwQual_   = hwQual  ;  }
void L1Muon::etaAtVtx (const float & etaAtVtx)  { etaAtVtx_ = etaAtVtx;  }
void L1Muon::phiAtVtx (const float & phiAtVtx)  { phiAtVtx_ = phiAtVtx;  }


// ------------ methods  ------------
