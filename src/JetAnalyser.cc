/**\class JetAnalyser JetAnalyser.cc Analysis/Tools/src/JetAnalyser.cc

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
#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
// 
// user include files
#include "TString.h" 
#include "Analysis/Tools/interface/JetAnalyser.h"

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

JetAnalyser::JetAnalyser()
{
}


JetAnalyser::JetAnalyser(int argc, char * argv[]) : BaseAnalyser(argc,argv)
{
   // Physics objects
   // Jets
   jetsanalysis_  = ( analysis_->addTree<Jet> ("Jets",config_->jetsCollection()) != nullptr );
   
   if ( config_->triggerObjDir_ != "" )
   {
      for ( auto & obj : config_->triggerObjectsJets_ )  analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
      for ( auto & obj : config_->triggerObjectsBJets_ ) analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
   }
}

JetAnalyser::~JetAnalyser()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//
// ------------ method called for each event  ------------

void JetAnalyser::jets(const std::string & col)
{
   analysis_->addTree<Jet> ("Jets",col);
}

void JetAnalyser::histograms(const std::string & obj, const int & n)
{
   if ( obj == "jet" )
   {
      for ( int j = 0; j < n; ++j )
      {
         h1_[Form("pt_%s%d"  , obj.c_str(),j+1)]    = new TH1F(Form("pt_%s%d"  , obj.c_str(),j+1)   , "" ,100 , 0   , 1000  );
         h1_[Form("eta_%s%d" , obj.c_str(),j+1)]    = new TH1F(Form("eta_%s%d" , obj.c_str(),j+1)   , "" , 60 , -3, 3 );
         h1_[Form("btag_%s%d", obj.c_str(),j+1)]    = new TH1F(Form("btag_%s%d", obj.c_str(),j+1)   , "" , 100 , 0, 1 );
         for ( int k = j+1; k < n && j < n; ++k )
         {
            h1_[Form("dr_%s%d%d"  , obj.c_str(),j+1,k+1)]     = new TH1F(Form("dr_%s%d%d"  , obj.c_str(),j+1,k+1)   , "" , 50 , 0, 5 );
            h1_[Form("deta_%s%d%d", obj.c_str(),j+1,k+1)]     = new TH1F(Form("deta_%s%d%d", obj.c_str(),j+1,k+1)   , "" ,100 , 0,10 );
            h1_[Form("m_%s%d%d"   , obj.c_str(),j+1,k+1)]     = new TH1F(Form("m_%s%d%d   ", obj.c_str(),j+1,k+1)   , "" ,300 , 0,3000 );
         }
      }
      
   }
}


float JetAnalyser::btag(const Jet & jet, const std::string & algo)
{
   float btag;
   if ( config_->btagalgo_ == "csvivf" || config_->btagalgo_ == "csv" )   {      btag = jet.btag("btag_csvivf");   }
   else if ( config_->btagalgo_ == "deepcsv" )                            {      btag = jet.btag("btag_deepb") + jet.btag("btag_deepbb");   }
   else if ( config_->btagalgo_ == "deepbvsall" )                         {      btag = jet.btag("btag_deepbvsall");   }
   else                                                                   {      btag = -9999;   }
   
   return btag;
}

bool JetAnalyser::selectionJet()
{
   bool isgood = true;
   
   // jet kinematics
   std::map<std::string,bool> isOk;
   for ( int j = 0; j < config_->nJetsMin() ; ++j )
   {
      if ( ! selectionJet(j+1) ) return false;
   }
   return isgood;
}

bool JetAnalyser::selectionJet(const int & r)
{
   ++cutflow_;
   
   bool isgood = true;
   int j = r-1;
   
   if ( selectedJets_.size() == 0 ) isgood = (isgood && selectionJetId());
   if ( !isgood || (int)selectedJets_.size() < r ) return false;
   
   // kinematic selection
   if ( selectedJets_[j] -> pt() < config_->jetsPtMin()[j]           && !(config_->jetsPtMin()[j] < 0) )   return false;
   if ( fabs(selectedJets_[j] -> eta()) > config_->jetsEtaMax()[j]   && !(config_->jetsEtaMax()[j] < 0) )  return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: pt > %5.1f and |eta| < %3.1f",r,config_->jetsPtMin()[j], config_->jetsEtaMax()[j] ));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return isgood;
}


bool JetAnalyser::selectionJetDeta(const int & j1, const int & j2, const float & delta)
{
   ++cutflow_;
   
   if ( (int)selectedJets_.size() < j1 || (int)selectedJets_.size() < j2 )
   {
      std::cout << "-errr- JetAnalyser::selectionJetDeta(): you dont have enough selected jets. Will return false" << std::endl;
      return false;
   }
   if ( delta > 0 )
   {
      if ( fabs(selectedJets_[j1-1]->eta() - selectedJets_[j2-1]->eta()) > fabs(delta) ) return false;
   }
   else
   {
      if ( fabs(selectedJets_[j1-1]->eta() - selectedJets_[j2-1]->eta()) < fabs(delta) ) return false;
   }

   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( delta > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Deta(jet %d, jet %d) < %4.2f",j1,j2,fabs(delta)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Deta(jet %d, jet %d) > %4.2f",j1,j2,fabs(delta)));
   }
   
        
   h1_["cutflow"] -> Fill(cutflow_);
    
   return true;
   
}
bool JetAnalyser::selectionJetDeta(const int & j1, const int & j2)
{
   bool ok = true;
   if (config_->detamax_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetDeta(j1,j2,config_->detamax_);
   }
   
   if (config_->detamin_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetDeta(j1,j2,-1*config_->detamin_);
   }
   return ok;
   
}


bool JetAnalyser::selectionJetDr(const int & j1, const int & j2, const float & delta)
{
   ++cutflow_;
   
   if ( (int)selectedJets_.size() < j1 || (int)selectedJets_.size() < j2 )
   {
      std::cout << "-e- JetAnalyser::selectionJetDr(): at least one of the jets does not exist" << std::endl;
      return false;
   }
   
   if ( delta > 0 )
   {
      if ( selectedJets_[j1-1]->deltaR(*selectedJets_[j2-1]) > fabs(delta) ) return false;
   }
   else
   {
      if ( selectedJets_[j1-1]->deltaR(*selectedJets_[j2-1]) < fabs(delta) ) return false;
   }

      
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( delta > 0 ) 
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("DR(jet %d, jet %d) < %4.2f",j1,j2,fabs(delta)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("DR(jet %d, jet %d) > %4.2f",j1,j2,fabs(delta)));
   }
        
   h1_["cutflow"] -> Fill(cutflow_);
    
   return true;
   
}

bool JetAnalyser::selectionJetDr(const int & j1, const int & j2)
{
   bool ok = true;
   if (config_->drmax_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetDr(j1,j2,config_->drmax_);
   }
   
   if (config_->drmin_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetDr(j1,j2,-1*config_->drmin_);
   }
   return ok;
}

bool JetAnalyser::analysisWithJets()
{
   jets_.clear();
   selectedJets_.clear();
   if ( ! jetsanalysis_ ) return false;
   
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsJets_,0.3);
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsBJets_,0.3);

   auto jets = analysis_->collection<Jet>("Jets");
   for ( int j = 0 ; j < jets->size() ; ++j )  jets_.push_back(&jets->at(j));
   
   selectedJets_ = jets_;
   
   return true;
}

std::vector<Jet*> JetAnalyser::jets()
{
   return jets_;
}

bool JetAnalyser::selectionJetId()
{
   ++cutflow_;
   
   if ( ! jetsanalysis_ ) return false;
   
   auto jet = std::begin(selectedJets_);
   while ( jet != std::end(selectedJets_) )
   {
      if ( ! (*jet)->id(config_->jetsId() ) )
         jet = selectedJets_.erase(jet);
      else
         ++jet;
   }
   if ( selectedJets_.size() == 0 ) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("JetId: %s",config_->jetsId().c_str()));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}

bool JetAnalyser::selectionJetPileupId()
{
   ++cutflow_;
   
   if ( ! jetsanalysis_ ) return false;
   
   auto jet = std::begin(selectedJets_);
   
   while ( jet != std::end(selectedJets_) )
   {
      if ( ! (*jet)->pileupJetIdFullId(config_->jetsPuId()) )
         jet = selectedJets_.erase(jet);
      else
         ++jet;
   }
   
   if ( selectedJets_.size() == 0 ) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("JetPileupId: %s",config_->jetsPuId().c_str()));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;

}

bool JetAnalyser::selectionNJets()
{
   ++cutflow_;
   
   if  ((int)selectedJets_.size() < config_->nJetsMin()) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("NJets >= %d",config_->nJetsMin()));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
   
}


bool JetAnalyser::selectionBJet()
{
   bool isgood = true;
   // jet btag
   for ( int j = 0; j < config_->nbjetsmin_ ; ++j )
   {
      if ( ! selectionBJet(j+1) ) return false;
   }
   return isgood;
}

bool JetAnalyser::selectionBJet(const int & r )
{
   int j = r-1;
   if ( config_->jetsbtagmin_[j] < 0 ) return true; // there is not selection here, so will not update the cutflow
   
   ++ cutflow_;
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJet(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
   if ( btag(*selectedJets_[j],config_->btagalgo_) < config_->jetsbtagmin_[j] ) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: btag > %6.4f",r,config_->jetsbtagmin_[j]));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}

bool JetAnalyser::selectionNonBJet(const int & r )
{
   int j = r-1;
   if ( config_->nonbtagwp_ < 0 || config_->jetsbtagmin_[j] < 0  ) return true; // there is not selection here, so will not update the cutflow
   
   ++ cutflow_;
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJet(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  non btag
   if ( btag(*selectedJets_[j],config_->btagalgo_) > config_->nonbtagwp_ ) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: btag < %6.4f",r,config_->nonbtagwp_));
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}


bool JetAnalyser::onlineJetMatching()
{
   if ( config_->triggerObjectsJetsMatches_ < 0 ) return true;
   
   for ( int j = 0; j < config_->triggerObjectsJetsMatches_; ++j )
   {
      if ( ! onlineJetMatching(j+1) ) return false; 
   }
   
   return true;
}

bool JetAnalyser::onlineBJetMatching()
{
   if ( config_->triggerObjectsBJetsMatches_ < 0 ) return true;
   
   for ( int j = 0; j < config_->triggerObjectsBJetsMatches_; ++j )
   {
      if ( ! onlineBJetMatching(j+1) ) return false; 
   }
   
   return true;
}



bool JetAnalyser::onlineJetMatching(const int & r)
{
   int j = r-1;
   if ( config_->triggerObjectsJets_.size() == 0 ) return true;
   
   ++cutflow_;
   
   if ( r > config_->nJetsMin() )
   {
      std::cout << "*Warning* JetAnalyser::onlineJetMatching(): asking for matching of unselected jet. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( selectedJets_.size() == 0 )
   {
      std::cout << "*Warning* JetAnalyser::onlineJetMatching(): selectedJets is empty. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   
   Jet * jet = selectedJets_[j];
   for ( size_t io = 0; io < config_->triggerObjectsJets_.size() ; ++io )
   {       
      if ( ! jet->matched(config_->triggerObjectsJets_[io]) ) return false;
   }

   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("JetTriggerMatch_%d",r));
      
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}


bool JetAnalyser::onlineBJetMatching(const int & r)
{
   int j = r-1;
   if ( config_->triggerObjectsBJets_.size() == 0 ) return true;
   
   ++cutflow_;
   
   if ( r > config_->nJetsMin() )
   {
      std::cout << "*Warning* JetAnalyser::onlineBJetMatching(): asking for matching of unselected jet. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   if ( selectedJets_.size() == 0 )
   {
      std::cout << "*Warning* JetAnalyser::onlineBJetMatching(): selectedJets is empty. You must run selectionJetId() before. Returning false!" << std::endl;
      return false;  // asking for a match beyond the selection, that's wrong, therefore false
   }
   
   Jet * jet = selectedJets_[j];
   for ( size_t io = 0; io < config_->triggerObjectsBJets_.size() ; ++io )
   {       
      if ( ! jet->matched(config_->triggerObjectsBJets_[io]) ) return false;
   }
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("BTagTriggerMatch_%d",r));
   
   
   h1_["cutflow"] -> Fill(cutflow_);
   
   return true;
}

void JetAnalyser::fillJetHistograms()
{
   int n = config_->nJetsMin();
   
   for ( int j = 0; j < n; ++j )
   {
      h1_[Form("pt_jet%d",j+1)] -> Fill(selectedJets_[j]->pt());
      h1_[Form("eta_jet%d",j+1)] -> Fill(selectedJets_[j]->eta());
      h1_[Form("btag_jet%d",j+1)] -> Fill(btag(*selectedJets_[j],config_->btagalgo_));
      for ( int k = j+1; k < n && j < n; ++k )
      {
         float deltaR = selectedJets_[j]->deltaR(*selectedJets_[k]);
         h1_[Form("dr_jet%d%d",j+1,k+1)]    -> Fill(deltaR);
         float deltaEta = fabs(selectedJets_[j]->eta() - selectedJets_[k]->eta());
         h1_[Form("deta_jet%d%d",j+1,k+1)]  -> Fill(deltaEta);
         float m = (selectedJets_[j]->p4()+selectedJets_[k]->p4()).M();
         if ( !config_->signalRegion() )
         {
            h1_[Form("m_jet%d%d",j+1,k+1)]  -> Fill(m);
         }
      }
   }
   
   
}

