/**\class Jet Jet.cc Analysis/Tools/src/Jet.cc

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
 
#include "Analysis/Tools/interface/Jet.h"


//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

//
// constructors and destructor
//
Jet::Jet() : Candidate() 
{
   extendedFlavour_ = "?";
   btagAlgo_ = "btag_csvivf";
   fsr_ = nullptr;
   muon_ = nullptr;
   uncorrJetp4_ = p4_;
}
Jet::Jet(const float & pt, const float & eta, const float & phi, const float & e) : Candidate(pt,eta,phi,e,0.) 
{
   extendedFlavour_ = "?";
   btagAlgo_ = "btag_csvivf";
   fsr_ = nullptr;
   muon_ = nullptr;
   uncorrJetp4_ = p4_;
   
}

Jet::~Jet()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//
// Gets
bool  Jet::isPuppi()                               const { return isPuppi_;    } 
float Jet::btag()                                  const { return btags_.at(btagAlgo_);    }                   
float Jet::btag(const std::string & algo)          const { return btags_.at(algo);         }                   
int   Jet::flavour()                               const { return flavour_.at("Hadron");   }                   
int   Jet::flavour(const std::string & definition) const { return flavour_.at(definition); }                   
bool  Jet::idLoose()                               const { return idloose_;                }                   
bool  Jet::idTight()                               const { return idtight_;                }         
float Jet::jecUncert()                             const { return jecUnc_;                 }                   
std::vector<int> Jet::flavours()                   const { return flavours_;               }
std::vector< std::shared_ptr<GenParticle> >\
      Jet::partons()                               const { return partons_;        }
std::string Jet::extendedFlavour()                 const { return extendedFlavour_; }
float Jet::JerResolution()                         const { return jerResolution_;}
float Jet::JerSf()                                 const { return jerSF_; }
float Jet::JerSfDown()                             const { return jerSFDown_; }
float Jet::JerSfUp()                               const { return jerSFUp_; }

float Jet::neutralHadronFraction()                 const { return nHadFrac_; }
float Jet::neutralEmFraction()                     const { return nEmFrac_;  }
float Jet::neutralMultiplicity()                   const { return nMult_;    }
float Jet::chargedHadronFraction()                 const { return cHadFrac_; }
float Jet::chargedEmFraction()                     const { return cEmFrac_;  }
float Jet::chargedMultiplicity()                   const { return cMult_;    }
float Jet::muonFraction()                          const { return muFrac_;   }
float Jet::constituents()                          const { return nConst_;   }

float Jet::qgLikelihood()                          const { return qgLikelihood_; }
float Jet::pileupJetIdFullDiscriminant()           const { return puJetIdFullDisc_; }
int   Jet::pileupJetIdFullId()                     const { return puJetIdFullId_; }

float Jet::bRegCorr() const  { return bRegCorr_; }
float Jet::bRegRes()  const  { return bRegRes_; }

double Jet::rho()     const  { return rho_; }

double Jet::btagSFsys(std::shared_ptr<BTagCalibrationReader> reader, const std::string & systype, const std::string & flavalgo) const
{
   double sf = 1;
   
   if ( reader == nullptr ) return sf;
   
   if ( this->flavour(flavalgo) == 5 ) sf = reader->eval_auto_bounds(systype, BTagEntry::FLAV_B,    fabs(this->eta()), this->pt() ); 
   if ( this->flavour(flavalgo) == 4 ) sf = reader->eval_auto_bounds(systype, BTagEntry::FLAV_C,    fabs(this->eta()), this->pt() ); 
   if ( this->flavour(flavalgo) == 0 ) sf = reader->eval_auto_bounds(systype, BTagEntry::FLAV_UDSG, fabs(this->eta()), this->pt() );
   
   return sf;
}

double Jet::btagSF(std::shared_ptr<BTagCalibrationReader> reader, const std::string & flavalgo) const
{
   return this -> btagSFsys(reader,"central",flavalgo);
}
double Jet::btagSFup(std::shared_ptr<BTagCalibrationReader> reader, const std::string & flavalgo) const
{
   return this -> btagSFsys(reader,"up",flavalgo);
}
double Jet::btagSFdown(std::shared_ptr<BTagCalibrationReader> reader, const std::string & flavalgo) const
{
   return this -> btagSFsys(reader,"down",flavalgo);
}


bool  Jet::pileupJetIdFullId(const std::string & wp) const
{ 
   std::string wplow = wp;
   std::transform(wplow.begin(), wplow.end(), wplow.begin(), ::tolower);
   if ( puJetIdFullId_ < 0 )
   {
      std::cout << "analysis:tools::Jet *W* Pileup Jet ID FullId is negative; the collection may not have this information." << std::endl;
      std::cout << "                        All jets are accepted." << std::endl;
      return true;
   }
   if ( wplow == "loose"  && (puJetIdFullId_ & (1 << 2)) ) return true;
   if ( wplow == "medium" && (puJetIdFullId_ & (1 << 1)) ) return true;
   if ( wplow == "tight"  && (puJetIdFullId_ & (1 << 0)) ) return true;
   return false;
}



// Sets                                                             
void Jet::isPuppi  (const bool & ispuppi)                             { isPuppi_  = ispuppi; } 
void Jet::btag     (const float & btag)                               { btag_    = btag; } 
void Jet::btag     (const std::string & algo, const float & btag)     { btags_[algo]  = btag; } 
void Jet::flavour  (const int   & flav)                               { flavour_["Hadron"] = flav; } 
void Jet::flavour  (const std::string & definition, const int & flav) { flavour_[definition] = flav; } 
void Jet::idLoose  (const bool  & loos)                               { idloose_ = loos; } 
void Jet::idTight  (const bool  & tigh)                               { idtight_ = tigh; } 
void Jet::jecUncert(const float & ju)                                 { jecUnc_  = ju; } 
void Jet::addParton(const std::shared_ptr<GenParticle> & parton)      { partons_.push_back(parton);
                                                                        flavours_.push_back(parton->pdgId());  }
void Jet::btagAlgo (const std::string & algo )                        { btagAlgo_ = algo; }                                                                        
void Jet::JerResolution(const float & jerResolution)                  { jerResolution_ = jerResolution; }
void Jet::JerSf(const float & jerSf)                                  { jerSF_ = jerSf; }
void Jet::JerSfDown(const float & jerSfDown)                          { jerSFDown_ = jerSfDown; }
void Jet::JerSfUp(const float & jerSfUp)                              { jerSFUp_ = jerSfUp; }

void Jet::qgLikelihood(const float & discr)                           { qgLikelihood_ = discr; }
void Jet::pileupJetIdFullDiscriminant(const float & discr)            { puJetIdFullDisc_ = discr; }
void Jet::pileupJetIdFullId(const int & id)                           { puJetIdFullId_ = id; }

void Jet::bRegCorr(const float & bRegCorr)                            { bRegCorr_ = bRegCorr; }
void Jet::bRegRes(const float & bRegRes)                              { bRegRes_  = bRegRes; }

void Jet::rho(const double & rho)                                      { rho_  = rho; }


int Jet::removeParton(const int & i)
{
   if ( partons_.size() == 1 )
   {
      partons_.clear();
      flavours_.clear();
   }
   else
   {
      partons_.erase(partons_.begin()+i);
      flavours_.erase(flavours_.begin()+i);
   }
   
   // re-do the extendedFlavour
   int flavour = abs(this->flavour());
   int flavCounter = 0;
   
   for ( auto & flav : flavours_ )
      if ( abs(flav) == flavour ) ++flavCounter;
   
//    if (( flavour == 4 || flavour == 5 ) && flavCounter == 0 )
//    {
//       std::cout << "ERROR!!! Jet::removeParton => Original flavour has changed!? " << flavour << "->" << extendedFlavour_ << std::endl;
//       return -1;
//    }

   if ( flavour == 4 && flavCounter > 1 ) extendedFlavour_ == "cc";
   if ( flavour == 5 && flavCounter > 1 ) extendedFlavour_ == "bb";
   
   return 0;
   
}

// ------------ methods  -----------
void Jet::addFSR(Jet* j)
{
   if ( j == nullptr ) return;
   if ( fsr_ != nullptr )
   {
      std::cout << "FSR jet will not be added because FSR already exists" << std::endl;
      return;
   }
   fsr_ = j;
   p4_ = p4_ + fsr_->p4();
}

void Jet::rmFSR()
{
   p4_ = uncorrJetp4_;
   fsr_ = nullptr;
}

Jet * Jet::fsrJet()
{
   return fsr_;
}

void Jet::addMuon(Muon* m)
{
   if ( m == nullptr ) return;
   if ( muon_ != nullptr )
   {
      std::cout << "A muon is already associated to this jet" << std::endl;
      return;
   }
   muon_ = m;
}

void Jet::rmMuon()
{
   muon_ = nullptr;
}

Muon * Jet::muon()
{
   return muon_;
}



void Jet::associatePartons(const std::vector< std::shared_ptr<GenParticle> > & particles, const float & dRmax, const float & ptMin,  const bool & pythia8 )
{
   int flavour = abs(this->flavour());
   extendedFlavour_ = "udsg";
   if ( flavour == 5 ) extendedFlavour_ = "b";
   if ( flavour == 4 ) extendedFlavour_ = "c";
   
   int flavCounter = 0;
   for ( auto & particle : particles )
   {
      int pdg = particle->pdgId();
      int status = particle->status();
      float pt = particle->pt();
      if ( pt < ptMin ) continue;
      if ( pythia8 )
      {
         if ( status != 71 && status != 72 ) continue;
      }
      else
      {
         if ( status != 3 ) continue;
      }
      if ( abs(pdg) > 5 && pdg != 21 ) continue;
      if ( this->p4().DeltaR(particle->p4()) > dRmax ) continue;
      
      addParton (particle);
      
      if ( abs(pdg) == flavour ) ++flavCounter;
      
   }
   
   // Need to check for ambiguities!!!
   
   // extendedFlavour re-definition
   if ( flavour == 4 && flavCounter > 1 ) extendedFlavour_ = "cc"; 
   if ( flavour == 5 && flavCounter > 1 ) extendedFlavour_ = "bb";
   
}
                                                                        
                                                                        
                                                                        
void Jet::id      (const float & nHadFrac,
                   const float & nEmFrac ,
                   const float & nMult   ,
                   const float & cHadFrac,
                   const float & cEmFrac ,
                   const float & cMult   ,
                   const float & muFrac  )
{
   float nM;
   float cM;
   float numConst;
   if ( isPuppi_ )
   {
      nM = nMult;
      cM = cMult;
      numConst = nM + cM;
   }
   else
   {
      nM = round(nMult);
      cM = round(cMult);
      numConst = round(nM + cM);
   }
   nHadFrac_ = nHadFrac;
   nEmFrac_  = nEmFrac;
   nMult_    = nM;
   cHadFrac_ = cHadFrac;
   cEmFrac_  = cEmFrac;
   cMult_    = cM;
   muFrac_   = muFrac;
   nConst_   = numConst;

   // Jet ID 2017 - only for AK4CHS (slimmedJets)
   // https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVRun2017?rev=6
   if ( fabs(p4_.Eta()) <= 2.7 )
   {
      idloose_ = false;
      idtight_ = ((nHadFrac<0.90 && nEmFrac<0.90 && numConst>1) && ((abs(p4_.Eta())<=2.4 && cHadFrac>0 && cM>0 ) || fabs(p4_.Eta())>2.4) && fabs(p4_.Eta())<=2.7);
   }
   else if ( fabs(p4_.Eta()) > 2.7 && fabs(p4_.Eta()) <= 3. )
   {
      idloose_ = false;
      if ( isPuppi_ )
      {
         idtight_ = (nHadFrac<0.99);
      }
      else
      {
         idtight_ = (nEmFrac>0.02 && nEmFrac<0.90 && nM>2);
      }
   }
   else
   {
      idloose_ = false;
      if ( isPuppi_ )
      {
         idtight_ = (nHadFrac>0.02 && nEmFrac<0.90 && nM>10);
      }
      else
      {
         idtight_ = (nHadFrac>0.02 && nEmFrac<0.90 && nM>2 && nM<15);
      }
   }
   
   
//    // Jet ID 2016
//    // https://twiki.cern.ch/twiki/bin/view/CMS/JetID?rev=95#Recommendations_for_13_TeV_data
//    if ( fabs(p4_.Eta()) <= 2.7 )
//    {
//       idloose_ = ((nHadFrac<0.99 && nEmFrac<0.99 && numConst>1) && ((abs(p4_.Eta())<=2.4 && cHadFrac>0 && cM>0 && cEmFrac<0.99) || fabs(p4_.Eta())>2.4) && fabs(p4_.Eta())<=2.7);
//       idtight_ = ((nHadFrac<0.90 && nEmFrac<0.90 && numConst>1) && ((abs(p4_.Eta())<=2.4 && cHadFrac>0 && cM>0 && cEmFrac<0.99) || fabs(p4_.Eta())>2.4) && fabs(p4_.Eta())<=2.7);
//    }
//    else if ( fabs(p4_.Eta()) > 2.7 && fabs(p4_.Eta()) <= 3. )
//    {
//       idloose_ = (nEmFrac<0.90 && nM>2);
//       idtight_ = (nEmFrac<0.90 && nM>2);
//    }
//    else
//    {
//       idloose_ = (nEmFrac<0.90 && nM>10);
//       idtight_ = (nEmFrac<0.90 && nM>10);
//    }
   
//    if ( tag_ == "JetIdOld" )
//    {
//       if ( fabs(p4_.Eta()) <= 3.0 )
//       {
//          idloose_ = ((nHadFrac<0.99 && nEmFrac<0.99 && numConst>1) && ((abs(p4_.Eta())<=2.4 && cHadFrac>0 && cM>0 && cEmFrac<0.99) || fabs(p4_.Eta())>2.4) && fabs(p4_.Eta())<=3.0);
//          idtight_ = ((nHadFrac<0.90 && nEmFrac<0.90 && numConst>1) && ((abs(p4_.Eta())<=2.4 && cHadFrac>0 && cM>0 && cEmFrac<0.99) || fabs(p4_.Eta())>2.4) && fabs(p4_.Eta())<=3.0);
//       }
//       else
//       {
//          idloose_ = (nEmFrac<0.90 && nM>10);
//          idtight_ = (nEmFrac<0.90 && nM>10);
//       }
//    }
   
}
