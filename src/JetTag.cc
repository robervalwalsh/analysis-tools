/**\class JetTag JetTag.cc Analysis/Core/src/JetTag.cc

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
 
#include "Analysis/Core/interface/JetTag.h"


//
// class declaration
//

using namespace analysis;
using namespace analysis::core;

//
// constructors and destructor
//
JetTag::JetTag() : Candidate() 
{
}
JetTag::JetTag(const float & pt, const float & eta, const float & phi, const float & e) : 
      Candidate(pt,eta,phi,e,0.) 
{
}
JetTag::~JetTag()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//
float JetTag::btag()                                  const { return btag_;    }
void  JetTag::btag(const float & btag)                      { btag_  = btag; } 
