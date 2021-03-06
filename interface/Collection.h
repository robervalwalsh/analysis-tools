#ifndef Analysis_Tools_Collection_h
#define Analysis_Tools_Collection_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      Collection
// 
/**\class xxx Collection.h Analysis/Tools/interface/Collection.h

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
#include "Analysis/Tools/interface/Candidate.h"
#include "Analysis/Tools/interface/Jet.h"
#include "Analysis/Tools/interface/TriggerObject.h"
#include "Analysis/Tools/interface/GenParticle.h"
#include "Analysis/Tools/interface/GenJet.h"

//
// class declaration
//

namespace analysis {
   namespace tools {
      
      template <class Object>
      class Collection {
         
         typedef std::vector<Object> Objects;

         public:
            Collection();
            Collection(const Objects & objects, const std::string & name_ = "");
           ~Collection();
           
           int size();
           void setSize(const int & size);
           Object & at(const int & index);
           void add(const Object & object);
           
           void matchTo( const std::vector<Candidate>* vectorcandidates, const std::string & name , const float & deltaR = 0.5 );
           void matchTo( const Collection<Candidate> & collection, const float & delta_pT, const float & deltaR);
           void matchTo( const Collection<Jet> & collection, const float & delta_pT, const float & deltaR);
           void matchTo( const Collection<Candidate> & collection, const float & deltaR = 0.5 );
           void matchTo( const Collection<TriggerObject> & collection, const float & deltaR = 0.5 );
           void matchTo( const std::shared_ptr<Collection<TriggerObject> > collection, const float & deltaR = 0.5 );

           void smearTo( const Collection<Jet> & collection, const double & n_sigma = 0 );

           std::vector< std::shared_ptr<Object> > vector();
           std::vector<Candidate>* vectorCandidates() const;
           
           std::string name() const;
           
           void associatePartons(const std::shared_ptr<Collection<GenParticle> > & , const float & deltaR = 0.4, const float & ptMin = 1., const bool & pythia8 = true);
           void addGenJets(const std::shared_ptr<Collection<GenJet> > &);
           void btagAlgo(const std::string &);           
            // ----------member data ---------------------------
         protected:
               
         private:
            Objects objects_;
            mutable std::vector<Candidate> candidates_; // maybe not the best idea but need to make code work
            int size_;
            std::string name_;

      };
      // ===============================================
      // INLINE IMPLEMENTATIONS
      
      
      // Gets
      template <class Object> inline int         Collection<Object>::size()                { return size_; }
      template <class Object> inline Object  &   Collection<Object>::at(const int & index) { return objects_.at(index); }
      template <class Object> inline std::string Collection<Object>::name() const          { return name_; }

      // Sets
      template <class Object> inline void   Collection<Object>::add(const Object & object) { objects_.push_back(object); ++size_;  }
      template <class Object> inline void   Collection<Object>::setSize(const int & size) { size_ = size; }

   }
}

#endif  // Analysis_Tools_Tree_h
