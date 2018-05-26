 /**\class RecoTrack RecoTrack.cc Analysis/Tools/src/RecoTrack.cc
 
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

#include "Analysis/Tools/interface/RecoTrack.h"


//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

//
// constructors and destructor
//
RecoTrack::RecoTrack() : Candidate()
{
}
RecoTrack::RecoTrack(const float & px, const float & py, const float & pz, const float & q) :
  Candidate(px,py,pz,q)
{
}
RecoTrack::~RecoTrack()
{
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

// Gets 

// Sets


