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
   genjetsanalysis_  = ( analysis_->addTree<GenJet> ("GenJets",config_->genJetsCollection()) != nullptr && config_->isMC() );
   
   applyjer_ = false;
   
   if ( config_->btagsf_ != "" )
   {
      bsf_reader_["loose"]  = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "loose"); 
      bsf_reader_["medium"] = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "medium");
      bsf_reader_["tight"]  = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "tight");
   }
   
   if ( config_->triggerObjDir_ != "" )
   {
      for ( auto & obj : config_->triggerObjectsJets() )  analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
      for ( auto & obj : config_->triggerObjectsBJets_ ) analysis_->addTree<TriggerObject> (obj,Form("%s/%s",config_->triggerObjDir_.c_str(),obj.c_str()));
   }
   
   if ( config_->jerPtRes() != "" && config_->jerSF() != "" && genjetsanalysis_ ) // FIXME: check if files exist
   {
      jerinfo_ = analysis_->jetResolutionInfo(config_->jerPtRes(),config_->jerSF());
      applyjer_ = ( jerinfo_ != nullptr );
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
   
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsJets(),0.3);
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsBJets_,0.3);

   // std::shared_ptr< Collection<Jet> >
   auto jets = analysis_->collection<Jet>("Jets");
   
   if ( genjetsanalysis_ )
   {
      auto genjets = analysis_->collection<GenJet>("GenJets");
      jets->addGenJets(genjets);
//       for ( int j = 0 ; j < jets->size() ; ++j )
//       {
//          auto jet = std::make_shared<Jet>(jets->at(j));
//          jet -> applyJER(*jerinfo_,0.2);
//          jets_.push_back(jet);
//       }
   }
//    else
//    {
//       for ( int j = 0 ; j < jets->size() ; ++j )  jets_.push_back(std::make_shared<Jet>(jets->at(j)));
//    }
   
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
   this->output()->cd();
   this->output()->mkdir(label.c_str());
   this->output()->cd(label.c_str());
   
   n_hjets_ = n;
   
   h1_[Form("jet_hist_weight_%s",label.c_str())] = std::make_shared<TH1F>(Form("jet_hist_weight_%s",label.c_str()) , Form("jet_hist_weight_%s",label.c_str()) ,1 , 0. , 1. );
   
   for ( int j = 0; j < n; ++j )
   {
      h1_[Form("pt_jet%d_%s"  , j+1,label.c_str())]  = std::make_shared<TH1F>(Form("pt_jet%d"  , j+1) , Form("pt_jet%d_%s"  , j+1,label.c_str()) ,100 , 0   , 1000  );
      h1_[Form("eta_jet%d_%s" , j+1,label.c_str())]  = std::make_shared<TH1F>(Form("eta_jet%d" , j+1) , Form("eta_jet%d_%s" , j+1,label.c_str()) , 60 , -3, 3 );
      h1_[Form("phi_jet%d_%s" , j+1,label.c_str())]  = std::make_shared<TH1F>(Form("phi_jet%d" , j+1) , Form("phi_jet%d_%s" , j+1,label.c_str()) , 360 , -180, 180 );
      h1_[Form("btag_jet%d_%s", j+1,label.c_str())]  = std::make_shared<TH1F>(Form("btag_jet%d", j+1) , Form("btag_jet%d_%s", j+1,label.c_str()) , 100 , 0, 1 );
      h1_[Form("pt_jet%d_%s"  , j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d p_{T} [GeV]",j+1));
      h1_[Form("eta_jet%d_%s" , j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d  #eta",j+1));
      h1_[Form("phi_jet%d_%s" , j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d  #phi",j+1));
      h1_[Form("btag_jet%d_%s", j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d btag discriminator",j+1)); 
      for ( int k = j+1; k < n && j < n; ++k )
      {
         h1_[Form("dr_jet%d%d_%s"  , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("dr_jet%d%d"  , j+1,k+1) , Form("dr_jet%d%d_%s"  , j+1,k+1,label.c_str()) , 50 , 0, 5 );
         h1_[Form("deta_jet%d%d_%s", j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("deta_jet%d%d", j+1,k+1) , Form("deta_jet%d%d_%s", j+1,k+1,label.c_str()) ,100 , 0,10 );
         h1_[Form("dphi_jet%d%d_%s", j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("dphi_jet%d%d", j+1,k+1) , Form("dphi_jet%d%d_%s", j+1,k+1,label.c_str()) ,360 , 0,360 );
         h1_[Form("pt_jet%d%d_%s"  , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("pt_jet%d%d"  , j+1,k+1) , Form("pt_jet%d%d_%s"  , j+1,k+1,label.c_str()) ,300 , 0,3000 );
         h1_[Form("eta_jet%d%d_%s" , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("eta_jet%d%d" , j+1,k+1) , Form("eta_jet%d%d_%s" , j+1,k+1,label.c_str()) ,200 , -10,10 );
         h1_[Form("phi_jet%d%d_%s" , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("phi_jet%d%d" , j+1,k+1) , Form("phi_jet%d%d_%s" , j+1,k+1,label.c_str()) ,360 , -180,180 );
         h1_[Form("m_jet%d%d_%s"   , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("m_jet%d%d"   , j+1,k+1) , Form("m_jet%d%d_%s"   , j+1,k+1,label.c_str()) ,300 , 0,3000 );
         h1_[Form("dr_jet%d%d_%s"  , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#DeltaR(Jet %d, Jet %d)",j+1,k+1));
         h1_[Form("deta_jet%d%d_%s", j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#Delta#eta(Jet %d, Jet %d)",j+1,k+1));
         h1_[Form("dphi_jet%d%d_%s", j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#Delta#phi(Jet %d, Jet %d)",j+1,k+1));
         h1_[Form("pt_jet%d%d_%s"  , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d + Jet %d p_{T} [GeV]",j+1,k+1));
         h1_[Form("eta_jet%d%d_%s" , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d + Jet %d  #eta",j+1,k+1));
         h1_[Form("phi_jet%d%d_%s" , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d + Jet %d  #phi",j+1,k+1));
         h1_[Form("m_jet%d%d_%s"   , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("M_{%d%d} [GeV]",j+1,k+1));
      }
   }
   
   this->output()->cd();

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
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( config_->jetsPtMax().size() > 0 && config_->jetsPtMax()[j] > config_->jetsPtMin()[j] )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: pt > %5.1f GeV and pt < %5.1f GeV and |eta| < %3.1f",r,config_->jetsPtMin()[j], config_->jetsPtMax()[j],config_->jetsEtaMax()[j] ));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: pt > %5.1f GeV and |eta| < %3.1f",r,config_->jetsPtMin()[j], config_->jetsEtaMax()[j] ));
   }
   
//   if ( selectedJets_.size() == 0 ) isgood = (isgood && selectionJetId());
//   if ( !isgood || (int)selectedJets_.size() < r ) return false;
   if ( (int)selectedJets_.size() < r ) return false;
   
   // kinematic selection
   if ( selectedJets_[j] -> pt() < config_->jetsPtMin()[j]           && !(config_->jetsPtMin()[j] < 0) ) return false;
   if ( fabs(selectedJets_[j] -> eta()) > config_->jetsEtaMax()[j]   && !(config_->jetsEtaMax()[j] < 0) ) return false;
   if ( config_->jetsPtMax().size() > 0 )
   {
      if ( selectedJets_[j] -> pt() > config_->jetsPtMax()[j] && !(config_->jetsPtMax()[j] < config_->jetsPtMin()[j]) )   return false;
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return isgood;
}


bool JetAnalyser::selectionJetDeta(const int & j1, const int & j2, const float & delta)
{
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( delta > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Deta(jet %d, jet %d) < %4.2f",j1,j2,fabs(delta)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Deta(jet %d, jet %d) > %4.2f",j1,j2,fabs(delta)));
   }
   
   
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
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( delta > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Dphi(jet %d, jet %d) < %4.2f",j1,j2,fabs(delta)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Dphi(jet %d, jet %d) > %4.2f",j1,j2,fabs(delta)));
   }
   
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
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( delta > 0 ) 
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("DR(jet %d, jet %d) < %4.2f",j1,j2,fabs(delta)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("DR(jet %d, jet %d) > %4.2f",j1,j2,fabs(delta)));
   }
        
   
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
   if ( ! jetsanalysis_ ) return false;
   
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("JetId: %s",config_->jetsId().c_str()));
   
   auto jet = std::begin(selectedJets_);
   while ( jet != std::end(selectedJets_) )
   {
      if ( ! (*jet)->id(config_->jetsId() ) )
         jet = selectedJets_.erase(jet);
      else
         ++jet;
   }
   if ( selectedJets_.size() == 0 ) return false;
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

bool JetAnalyser::selectionJetPileupId()
{
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("JetPileupId: %s",config_->jetsPuId().c_str()));
   
   
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
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;

}

bool JetAnalyser::selectionNJets()
{
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("NJets >= %d",config_->nJetsMin()));
   
   
   if  ((int)selectedJets_.size() < config_->nJetsMin()) return false;
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
   
}


bool JetAnalyser::selectionBJet(const int & r )
{
   if ( ! config_->signalRegion() && r == config_->nonBtagJet() ) return this->selectionNonBJet(r);
      
   int j = r-1;
   if ( config_->btagWP(config_->jetsBtagWP()[j]) < 0 ) return true; // there is no selection here, so will not update the cutflow
   
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag > %6.4f (%s)",r,config_->btagalgo_.c_str(),config_->btagWP(config_->jetsBtagWP()[j]),config_->jetsBtagWP()[j].c_str()));
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJet(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
//   if ( btag(*selectedJets_[j],config_->btagalgo_) < config_->jetsbtagmin_[j] ) return false;
   
   if ( btag(*selectedJets_[j],config_->btagalgo_) < config_->btagWP(config_->jetsBtagWP()[j]) ) return false;
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

bool JetAnalyser::selectionNonBJet(const int & r )
{
   int j = r-1;

   if ( config_->btagWP(config_->nonBtagWP()) < 0 ) return true; // there is no selection here, so will not update the cutflow

   ++ cutflow_;
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJet(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
  
   // jet  non btag
   if ( btag(*selectedJets_[j],config_->btagalgo_) > config_->btagWP(config_->nonBtagWP()) ) return false;
   
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag < %6.4f (%s) [reverse btag]",r,config_->btagalgo_.c_str(),config_->btagWP(config_->nonBtagWP()),config_->nonBtagWP().c_str()));
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}


bool JetAnalyser::onlineJetMatching(const int & r)
{
   int j = r-1;
   if ( config_->triggerObjectsJets().size() == 0 ) return true;
   
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: online jet match",r));
      
   
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
   for ( size_t io = 0; io < config_->triggerObjectsJets().size() ; ++io )
   {       
      if ( ! jet->matched(config_->triggerObjectsJets()[io]) ) return false;
   }

   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}


bool JetAnalyser::onlineBJetMatching(const int & r)
{
   int j = r-1;
   if ( config_->triggerObjectsBJets_.size() == 0 ) return true;
   
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: online b jet match",r));
   
   
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
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

void JetAnalyser::fillJetHistograms(const std::string & label)
{
   this->output()->cd();
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("*** Filling jets histograms - %s",label.c_str()));
   
   this->output()->cd(label.c_str());
   
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
         if ( config_->isMC() || !config_->signalRegion() )
         {
            h1_[Form("m_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(c_ij.m(),weight_);
         }
         else  // blind
         {
            h1_[Form("m_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(0.,weight_);
         }
      }
   }
   this->output()->cd();
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
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


void JetAnalyser::actionApplyJER()
{
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,"*** Apply JER smearing");
   
   
   if ( applyjer_ )
   {
      for ( auto & j : selectedJets_ )
      {
         j -> applyJER(*jerinfo_,0.2);
      }
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
}

void JetAnalyser::actionApplyBtagSF(const int & r)
{
   if ( ! config_-> isMC() || config_->btagsf_ == "" ) return;  // will not apply btag SF
   if ( ! config_->signalRegion() && r == config_->nonBtagJet() ) return;
   
   int j = r-1;
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: btag SF applied (%s %s WP)",r,config_->btagalgo_.c_str(),config_->jetsBtagWP()[j].c_str()));
   
   float sf = this->btagSF(r,config_->jetsBtagWP()[j]).nominal;
   
   weight_ *= sf;
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
}

void JetAnalyser::actionApplyBjetRegression()
{
   if ( ! config_->bRegression() ) return;
   
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,"*** Apply b jet energy regression");
   
   for ( auto & j : selectedJets_ )
   {
      j -> applyBjetRegression();
   }
   h1_["cutflow"] -> Fill(cutflow_,weight_);
}

bool JetAnalyser::selectionDiJetMass(const int & r1, const int & r2)
{
   float min = config_->massMin();
   float max = config_->massMax();
   if ( min < 0. && max < 0. ) return true;
   
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      std::string label = Form("M(Jet %d + Jet %d)",r1,r2);
      if ( min > 0. )
      {   
         if ( max > 0. && max > min ) label = Form("%5.1f GeV < %s < %5.1f GeV",min,label.c_str(),max);
         else                         label = Form("%s > %5.1f GeV",label.c_str(),min);
      }
      else
      {
         label = Form("%s < %5.1f GeV",label.c_str(),max);
      }
      
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,label.c_str());
   }
   
   int j1 = r1-1;
   int j2 = r2-1;
   Composite<Jet,Jet> c_j1j2(*(selectedJets_[j1]),*(selectedJets_[j2]));
   float mass = c_j1j2.m();
   
   if ( min > 0. && mass < min ) return false;
   if ( max > 0. && mass > max ) return false;
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

void JetAnalyser::jetSwap(const int & r1, const int & r2)
{
   if ( r1 == r2 ) return;
   int j1 = r1-1;
   int j2 = r2-1;
//    ++ cutflow_;
//    if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
//       h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d <--> Jet %d: jets ranking was swapped ",r1,r2));
   
   auto jet1 = selectedJets_[j1];
   auto jet2 = selectedJets_[j2];
   
   selectedJets_[j1] = jet2;
   selectedJets_[j2] = jet1;
   
//    h1_["cutflow"] -> Fill(cutflow_,weight_);
   
}
