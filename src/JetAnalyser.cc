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
#include "Analysis/Tools/interface/Composite.h"
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
   
   if ( config_->btagsf_ != "" )
   {
      bsf_reader_["loose"]  = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "loose"); 
      bsf_reader_["medium"] = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "medium");
      bsf_reader_["tight"]  = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "tight");
   }
   
   if ( config_->triggerObjDir_ != "" )
   {
      for ( auto & obj : config_->triggerObjectsJets_ )  analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
      for ( auto & obj : config_->triggerObjectsBJets_ ) analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
   }
   
//   histograms("jet",config_->nJetsMin());
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

bool JetAnalyser::analysisWithJets()
{
   jets_.clear();
   selectedJets_.clear();
   if ( ! jetsanalysis_ ) return false;
   
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsJets_,0.3);
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsBJets_,0.3);

   auto jets = analysis_->collection<Jet>("Jets");
   for ( int j = 0 ; j < jets->size() ; ++j )  jets_.push_back(std::make_shared<Jet>(jets->at(j)));
   
   selectedJets_ = jets_;
   
   return true;
}


void JetAnalyser::jets(const std::string & col)
{
   analysis_->addTree<Jet> ("Jets",col);
}

void JetAnalyser::jetHistograms( const int & n, const std::string & label )
{
   n_hjets_ = n;
   
   h1_[Form("jet_hist_weight_%s",label.c_str())] = std::make_shared<TH1F>(Form("jet_hist_weight_%s",label.c_str()) , "" ,1 , 0. , 1. );
   
   for ( int j = 0; j < n; ++j )
   {
      h1_[Form("pt_jet%d_%s"  , j+1,label.c_str())]    = std::make_shared<TH1F>(Form("pt_jet%d_%s"  , j+1,label.c_str())   , "" ,100 , 0   , 1000  );
      h1_[Form("eta_jet%d_%s" , j+1,label.c_str())]    = std::make_shared<TH1F>(Form("eta_jet%d_%s" , j+1,label.c_str())   , "" , 60 , -3, 3 );
      h1_[Form("phi_jet%d_%s" , j+1,label.c_str())]    = std::make_shared<TH1F>(Form("phi_jet%d_%s" , j+1,label.c_str())   , "" , 360 , -180, 180 );
      h1_[Form("btag_jet%d_%s", j+1,label.c_str())]    = std::make_shared<TH1F>(Form("btag_jet%d_%s", j+1,label.c_str())   , "" , 100 , 0, 1 );
      for ( int k = j+1; k < n && j < n; ++k )
      {
         h1_[Form("dr_jet%d%d_%s"  , j+1,k+1,label.c_str())]     = std::make_shared<TH1F>(Form("dr_jet%d%d_%s"  , j+1,k+1,label.c_str())   , "" , 50 , 0, 5 );
         h1_[Form("deta_jet%d%d_%s", j+1,k+1,label.c_str())]     = std::make_shared<TH1F>(Form("deta_jet%d%d_%s", j+1,k+1,label.c_str())   , "" ,100 , 0,10 );
         h1_[Form("pt_jet%d%d_%s"  , j+1,k+1,label.c_str())]     = std::make_shared<TH1F>(Form("pt_jet%d%d_%s"  , j+1,k+1,label.c_str())   , "" ,300 , 0,3000 );
         h1_[Form("eta_jet%d%d_%s" , j+1,k+1,label.c_str())]     = std::make_shared<TH1F>(Form("eta_jet%d%d_%s" , j+1,k+1,label.c_str())   , "" ,200 , -10,10 );
         h1_[Form("phi_jet%d%d_%s" , j+1,k+1,label.c_str())]     = std::make_shared<TH1F>(Form("phi_jet%d%d_%s" , j+1,k+1,label.c_str())   , "" ,360 , -180,180 );
         h1_[Form("m_jet%d%d_%s"   , j+1,k+1,label.c_str())]     = std::make_shared<TH1F>(Form("m_jet%d%d_%s"   , j+1,k+1,label.c_str())   , "" ,300 , 0,3000 );
      }
   }
}


float JetAnalyser::btag(const Jet & jet, const std::string & algo)
{
   float btag;
   if ( config_->btagalgo_ == "csvivf" || config_->btagalgo_ == "csv" )   {      btag = jet.btag("btag_csvivf");   }
   else if ( config_->btagalgo_ == "deepcsv" )                            {      btag = jet.btag("btag_deepb") + jet.btag("btag_deepbb");   }
   else if ( config_->btagalgo_ == "deepbvsall" )                         {      btag = jet.btag("btag_deepbvsall");   }
   else if ( config_->btagalgo_ == "deepflavour" )                        {      btag = jet.btag("btag_dfb") + jet.btag("btag_dfbb") + jet.btag("btag_dflepb");   }
   else                                                                   {      btag = -9999;   }
   
   return btag;
}

bool JetAnalyser::selectionJet(const int & r)
{
   ++cutflow_;
   
   bool isgood = true;
   int j = r-1;
   
   if ( selectedJets_.size() == 0 ) isgood = (isgood && selectionJetId());
   if ( !isgood || (int)selectedJets_.size() < r ) return false;
   
   // kinematic selection
   if ( selectedJets_[j] -> pt() < config_->jetsPtMin()[j]           && !(config_->jetsPtMin()[j] < 0) ) return false;
   if ( fabs(selectedJets_[j] -> eta()) > config_->jetsEtaMax()[j]   && !(config_->jetsEtaMax()[j] < 0) ) return false;
   if ( config_->jetsPtMax().size() > 0 )
   {
      if ( selectedJets_[j] -> pt() > config_->jetsPtMax()[j] && !(config_->jetsPtMax()[j] < config_->jetsPtMin()[j]) )   return false;
   }
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( config_->jetsPtMax().size() > 0 && config_->jetsPtMax()[j] > config_->jetsPtMin()[j] )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: pt > %5.1f and pt < %5.1f and |eta| < %3.1f",r,config_->jetsPtMin()[j], config_->jetsPtMax()[j],config_->jetsEtaMax()[j] ));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: pt > %5.1f and |eta| < %3.1f",r,config_->jetsPtMin()[j], config_->jetsEtaMax()[j] ));
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
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
   
        
   h1_["cutflow"] -> Fill(cutflow_,weight_);
    
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

bool JetAnalyser::selectionJetDphi(const int & j1, const int & j2, const float & delta)
{
   ++cutflow_;
   
   if ( (int)selectedJets_.size() < j1 || (int)selectedJets_.size() < j2 )
   {
      std::cout << "-errr- JetAnalyser::selectionJetDeta(): you dont have enough selected jets. Will return false" << std::endl;
      return false;
   }
   if ( delta > 0 )
   {
      if ( fabs(selectedJets_[j1-1]->deltaPhi(*selectedJets_[j2-1])) > fabs(delta) ) return false;
   }
   else
   {
      if ( fabs(selectedJets_[j1-1]->deltaPhi(*selectedJets_[j2-1])) < fabs(delta) ) return false;
   }

   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( delta > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Dphi(jet %d, jet %d) < %4.2f",j1,j2,fabs(delta)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Dphi(jet %d, jet %d) > %4.2f",j1,j2,fabs(delta)));
   }
   
        
   h1_["cutflow"] -> Fill(cutflow_,weight_);
    
   return true;
   
}
bool JetAnalyser::selectionJetDphi(const int & j1, const int & j2)
{
   bool ok = true;
   if (config_->dphimax_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetDphi(j1,j2,config_->dphimax_);
   }
   
   if (config_->dphimin_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetDphi(j1,j2,-1*config_->dphimin_);
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
        
   h1_["cutflow"] -> Fill(cutflow_,weight_);
    
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


std::vector< std::shared_ptr<Jet> > JetAnalyser::jets()
{
   return jets_;
}

std::vector< std::shared_ptr<Jet> > JetAnalyser::selectedJets()
{
   return selectedJets_;
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
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
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
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;

}

bool JetAnalyser::selectionNJets()
{
   ++cutflow_;
   
   if  ((int)selectedJets_.size() < config_->nJetsMin()) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("NJets >= %d",config_->nJetsMin()));
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
   
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
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
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
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
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
   
   std::shared_ptr<Jet> jet = selectedJets_[j];
   for ( size_t io = 0; io < config_->triggerObjectsJets_.size() ; ++io )
   {       
      if ( ! jet->matched(config_->triggerObjectsJets_[io]) ) return false;
   }

   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("JetTriggerMatch_%d",r));
      
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
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
   
   std::shared_ptr<Jet> jet = selectedJets_[j];
   for ( size_t io = 0; io < config_->triggerObjectsBJets_.size() ; ++io )
   {       
      if ( ! jet->matched(config_->triggerObjectsBJets_[io]) ) return false;
   }
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("BTagTriggerMatch_%d",r));
   
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

void JetAnalyser::fillJetHistograms(const std::string & label)
{
   int n = n_hjets_;
   
   if ( n > config_->nJetsMin() ) n = config_->nJetsMin();
   
   h1_[Form("jet_hist_weight_%s",label.c_str())] -> Fill(0.,weight_);
   
   for ( int j = 0; j < n; ++j )
   {
      h1_[Form("pt_jet%d_%s",j+1,label.c_str())] -> Fill(selectedJets_[j]->pt(),weight_);
      h1_[Form("eta_jet%d_%s",j+1,label.c_str())] -> Fill(selectedJets_[j]->eta(),weight_);
      h1_[Form("phi_jet%d_%s",j+1,label.c_str())] -> Fill(selectedJets_[j]->phi()*180./acos(-1.),weight_);
      h1_[Form("btag_jet%d_%s",j+1,label.c_str())] -> Fill(btag(*selectedJets_[j],config_->btagalgo_),weight_);
      for ( int k = j+1; k < n && j < n; ++k )
      {
         Composite<Jet,Jet> c_ij(*(selectedJets_[j]),*(selectedJets_[k]));
         
         h1_[Form("dr_jet%d%d_%s",j+1,k+1,label.c_str())]    -> Fill(c_ij.deltaR(),weight_);
         h1_[Form("deta_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(c_ij.deltaEta(),weight_);
         
         h1_[Form("pt_jet%d%d_%s",j+1,k+1,label.c_str())]   -> Fill(c_ij.pt(),weight_);
         h1_[Form("eta_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(c_ij.eta(),weight_);
         h1_[Form("phi_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(c_ij.phi()*180./acos(-1.),weight_);
         if ( config_->blind() )
         {
            h1_[Form("m_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(0.,weight_);
         }
         else
         {
            h1_[Form("m_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(c_ij.m(),weight_);
         }
      }
   }
   
   
}

ScaleFactors JetAnalyser::btagSF(const int & r, const std::string & wp)
{
   ScaleFactors sf;
   int j = r-1;
   sf.nominal = selectedJets_[j]->btagSF(bsf_reader_[wp]);
   sf.up      = selectedJets_[j]->btagSFup(bsf_reader_[wp]);
   sf.down    = selectedJets_[j]->btagSFdown(bsf_reader_[wp]);
   
   return sf;
}
