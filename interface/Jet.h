#ifndef Analysis_Tools_Jet_h
#define Analysis_Tools_Jet_h 1

// -*- C++ -*-
//
// Package:    Analysis/Tools
// Class:      Jet
// 
/**\class Jet Jet.cc Analysis/Tools/src/Jet.cc

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

// 
// user include files
#include "Analysis/Tools/interface/Candidate.h"
#include "Analysis/Tools/interface/Muon.h"
#include "Analysis/Tools/interface/GenParticle.h"
//
// class declaration
//

namespace analysis {
   namespace tools {
      
      class Jet : public Candidate {
         public:
            /// default constructor
            Jet();
            /// constructor from 4-momentum information
            Jet(const float & pt, const float & eta, const float & phi, const float & e);
            /// constructor from TLorentzVector
            Jet(const TLorentzVector & p4);
            /// destructor
           ~Jet();
           
            // Gets
           
            /// returns if jet is Puppi
            bool  isPuppi()                     const;
            /// returns the btag value of btag_csvivf
            float btag()                        const;
            /// returns the btag value of algorithm
            float btag(const std::string & )    const;
            /// returns the flavour with the Hadron definition (=0 for data)
            int   flavour()                     const;
            /// returns the flavour given a definition (=0 for data)
            int   flavour(const std::string & ) const;
            /// returns if jet has id loose working point
            bool  idLoose()                     const;
            /// returns if jet has id tight working point
            bool  idTight()                     const;
            /// returns the jet energy correction uncertainty
            float jecUncert()                   const;
            /// returns the vector containing flavours inside the jet
            std::vector<int> flavours()         const;
            /// returns the extended flavour definition
            std::string extendedFlavour()       const;
            /// returns the vector of pointers to the generated partons
            std::vector< std::shared_ptr<GenParticle> > partons() const;
            /// returns jet energy resolution
            float JerResolution() const;
            /// returns jet energy resolution SF
            float JerSf() const;
            /// returns jet energy resolution SF Down variation
            float JerSfDown() const;
            /// returns jet energy resolution SF Up variation
            float JerSfUp() const;
            
            float neutralHadronFraction()  const ;
            float neutralEmFraction()      const ;
            float neutralMultiplicity()    const ;
            float chargedHadronFraction()  const ;
            float chargedEmFraction()      const ;
            float chargedMultiplicity()    const ;
            float muonFraction()           const ;
            float constituents()           const ;
            
            /// quark-gluon separation
            float qgLikelihood()  const;
            
            /// pile up jet id
            float pileupJetIdFullDiscriminant() const;
            int pileupJetIdFullId() const;
            bool pileupJetIdFullId(const std::string & wp) const;
            
            /// b-jet regression
            float bRegCorr() const;
            float bRegRes()  const;
            
            /// Rho
            double rho() const;
            
            /// pointer to the FSR jet
            Jet * fsrJet();
            
            /// pointer to the muon
            Muon * muon();
               
            // Sets
            /// sets the isPuppi value
            void  isPuppi(const bool &);
            /// sets the btag value
            void  btag(const float &);
            /// sets the btag value for difference algorithms
            void  btag(const std::string &, const float &);
            /// sets the default btag algo
            void  btagAlgo(const std::string & );
            /// sets flavour
            void  flavour(const int &);
            /// sets flavour for a given definition
            void  flavour(const std::string & definition, const int & value);
            /// sets if jet id is loose or not
            void  idLoose(const bool &);
            /// sets if jet id is tight or not
            void  idTight(const bool &);
            /// sets the jet energy correction uncertainty
            void  jecUncert(const float &);
            /// sets jet energy resolution
            void JerResolution(const float & jerResolution);
            /// sets jet energy resolution SF
            void JerSf(const float & jerSf);
            /// sets jet energy resolution SF Up variation
            void JerSfUp(const float & jerSfUp);
            /// sets jet energy resolution SF Down variation
            void JerSfDown(const float & jerSfDown);
            /// add parton that gave rise to jet
            void addParton(const std::shared_ptr<GenParticle> &);
            /// remove parton from jet parton list
            int removeParton(const int &);
            
            void neutralHadronFraction(const float & nHadFrac);
            void neutralEmFraction(const float & nEmFrac);
            void neutralMultiplicity(const float & nMult);
            void chargedHadronFraction(const float & cHadFrac);
            void chargedEmFraction(const float & cEmFrac);
            void chargedMultiplicity(const float & cMult);
            void muonFraction(const float & muFrac);
            
            /// calculates the jet id
            void id(const float & nHadFrac,
                    const float & nEmFrac ,
                    const float & nMult   ,
                    const float & cHadFrac,
                    const float & cEmFrac ,
                    const float & cMult   ,
                    const float & muFrac  );
            
            /// quark-gluon separation
            void qgLikelihood(const float & discr);
            
            /// pile up jet id
            void pileupJetIdFullDiscriminant(const float & discr);
            void pileupJetIdFullId(const int & id);
            
            /// b-jet regression
            void bRegCorr(const float &);
            void bRegRes(const float &);
            
            /// Rho
            void rho(const double &);
            
            
            /// associate partons to the jet
            void associatePartons(const std::vector< std::shared_ptr<GenParticle> > &, const float & dRmax = 0.5, const float & ptMin = 1., const bool & pythi8 = true );
//            using Candidate::set; // in case needed to overload the function set
            
            /// add a final state radiation jet, will modify the 4-momentum
            void addFSR(Jet*);
            /// remove the final state radiation jet, will change back the original 4-momentum
            void rmFSR();
            
            /// associate a muon to the jet
            void addMuon(Muon*);
            /// remove muon association to the jet
            void rmMuon();
            
            
         protected:
            // ----------member data ---------------------------
            //
               
            // Jet type
            bool isPuppi_;
            /// btag value 
            float btag_ ;
            /// btag value for each algo
            std::map<std::string, float> btags_ ;
            /// default btag algo
            std::string btagAlgo_;
            /// map of flavour to a given definition
            std::map<std::string, int> flavour_;
            /// flavours inside the jet
            std::vector<int> flavours_;
            /// extended flavour identification for merged jets
            std::string extendedFlavour_;
            /// vector of pointers to Genparticles from merged jets
            std::vector< std::shared_ptr<GenParticle> > partons_;
            /// jet id loose working point
            bool  idloose_;
            /// jet id tight working point
            bool  idtight_;
            /// jet energy correction uncertainty
            float jecUnc_;
            /// jet energy resolution SF
            float jerSF_;
            /// jet energy resolution SF Up variation
            float jerSFUp_;
            /// jet energy resolution SF Down variation
            float jerSFDown_;
            /// jet energy resolution
            float jerResolution_;
            
            /// jet id
            float nHadFrac_;
            float nEmFrac_;
            float nMult_;
            float cHadFrac_;
            float cEmFrac_;
            float cMult_;
            float muFrac_;
            float nConst_;
            /// quark-gluon separation
            float qgLikelihood_;
            /// pileup jet id
            float puJetIdFullDisc_;
            int   puJetIdFullId_;
            
            /// b-jet regression
            float bRegCorr_;
            float bRegRes_;
            
            /// Fixedgridrho (for JER)
            double rho_;
            
            /// final state radiation
            Jet * fsr_;
            /// 4-momentum before FSR correction
            TLorentzVector uncorrJetp4_;
            
            /// muon in jet
            Muon * muon_;
            
            
         private:
            // ----------member data ---------------------------
            
      };
   }
}

#endif  // Analysis_Tools_Jet_h
