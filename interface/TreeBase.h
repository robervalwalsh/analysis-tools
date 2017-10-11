#ifndef Analysis_Core_TreeBase_h
#define Analysis_Core_TreeBase_h 1

// -*- C++ -*-
//
// Package:    Analysis/Core
// Class:      TreeBase
// 
/**\class TreeBase TreeBase.cc Analysis/Core/src/TreeBase.cc

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
// 
// user include files

#include "TTree.h"
#include "TChain.h"
// #include "Analysis/Core/interface/Jet.h"
// #include "Analysis/Core/interface/MET.h"
// #include "Analysis/Core/interface/Muon.h"
// #include "Analysis/Core/interface/Vertex.h"

//
// class declaration
//

namespace analysis {
   namespace core {

//       template <typename Object>
      class TreeBase : public TChain{
         public:
            TreeBase();
            TreeBase(TChain * tree, const std::string & name);
           ~TreeBase();
           
           void event(const int & event);
           TChain * tree();
           
            // ----------member data ---------------------------
         protected:
            TChain * tree_;
            std::string className_;
            std::string inputTag_;
            
            std::vector<std::string> branches_;
            
            std::string name_;
            
         private:

      };
         
   }
}

#endif  // Analysis_Core_TreeBase_h
