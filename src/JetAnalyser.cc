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
   genjetsanalysis_ = false;
   if ( config_->isMC() )  genjetsanalysis_  = ( analysis_->addTree<GenJet> ("GenJets",config_->genJetsCollection()) != nullptr );
   
   applyjer_ = false;
   
   if ( config_->btagsf_ != "" )
   {
      bsf_reader_["loose"]  = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "loose"); 
      bsf_reader_["medium"] = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "medium");
      bsf_reader_["tight"]  = analysis_->btagCalibration(config_->btagalgo_, config_->btagsf_, "tight");
   }
   
   if ( config_->jerPtRes() != "" && config_->jerSF() != "" && genjetsanalysis_ ) // FIXME: check if files exist
   {
      jerinfo_ = analysis_->jetResolutionInfo(config_->jerPtRes(),config_->jerSF());
      applyjer_ = ( jerinfo_ != nullptr );
   }
   
   if ( config_->isMC() )
   {
      flavours_ = {"udsg","c","b"};
      if ( config_->useJetsExtendedFlavour() )
      {
         flavours_.push_back("cc");
         flavours_.push_back("bb");
      }
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
   
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsL1Jets(),0.3);
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsCaloJets(),0.3);
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsPFJets(),0.3);
   analysis_->match<Jet,TriggerObject>("Jets",config_->triggerObjectsBJets(),0.3);

   // std::shared_ptr< Collection<Jet> >
   auto jets = analysis_->collection<Jet>("Jets");
   
   if ( this->genParticlesAnalysis() && config_ -> useJetsExtendedFlavour() )
   {
      auto particles = analysis_->collection<GenParticle>("GenParticles");
      jets->associatePartons(particles,0.4,1.,config_->pythia8());
   }
   
   if ( genjetsanalysis_ )
   {
      auto genjets = analysis_->collection<GenJet>("GenJets");
      jets->addGenJets(genjets);
   }
   
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
   
   // btag binning
   float min1 = 0.0;
   float max1 = 0.1;
   float size1 = 0.0001;
   int nbins1 = int((max1-min1)/size1);
   float min2 = 0.1;
   float max2 = 0.9;
   float size2 = 0.001;
   int nbins2 = int((max2-min2)/size2);
   float min3 = 0.9;
   float max3 = 1.0;
   float size3 = 0.0001;
   int nbins3 = int((max3-min3)/size3);
   int nbins_btag = nbins1+nbins2+nbins3;
   
   std::vector<float> bins_btag;
   int counter = 0; 
   for ( int i = 0; i<nbins1; ++i)   {  bins_btag.push_back(min1 + size1*i);  ++counter; }
   for ( int i = 0; i<nbins2; ++i)   {  bins_btag.push_back(min2 + size2*i);  ++counter; }
   for ( int i = 0; i<nbins3+1; ++i) {  bins_btag.push_back(min3 + size3*i);  ++counter; }
   
   
   for ( int j = 0; j < n; ++j ) // loop over jets
   {
      // 1D histograms
      h1_[Form("pt_jet%d_%s"  , j+1,label.c_str())]  = std::make_shared<TH1F>(Form("pt_jet%d"  , j+1) , Form("pt_jet%d_%s"  , j+1,label.c_str()) ,1500 , 0   , 1500  );
      h1_[Form("eta_jet%d_%s" , j+1,label.c_str())]  = std::make_shared<TH1F>(Form("eta_jet%d" , j+1) , Form("eta_jet%d_%s" , j+1,label.c_str()) , 600 , -3, 3 );
      h1_[Form("phi_jet%d_%s" , j+1,label.c_str())]  = std::make_shared<TH1F>(Form("phi_jet%d" , j+1) , Form("phi_jet%d_%s" , j+1,label.c_str()) , 360 , -180, 180 );
      h1_[Form("btag_jet%d_%s", j+1,label.c_str())]  = std::make_shared<TH1F>(Form("btag_jet%d", j+1) , Form("btag_jet%d_%s", j+1,label.c_str()) , nbins_btag, &bins_btag[0] );
      h1_[Form("btaglog_jet%d_%s", j+1,label.c_str())]  = std::make_shared<TH1F>(Form("btaglog_jet%d", j+1) , Form("btaglog_jet%d_%s", j+1,label.c_str()) , 200 , 1.e-6, 10 );
      h1_[Form("qglikelihood_jet%d_%s", j+1,label.c_str())]  = std::make_shared<TH1F>(Form("qglikelihood_jet%d", j+1) , Form("qglikelihood_jet%d_%s", j+1,label.c_str()) , 200 , 0, 1 );
      h1_[Form("nconstituents_jet%d_%s", j+1,label.c_str())]  = std::make_shared<TH1F>(Form("nconstituents_jet%d", j+1) , Form("nconstituents_jet%d_%s", j+1,label.c_str()) , 200 , 0, 200 );
      
      h1_[Form("pt_jet%d_%s"  , j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d p_{T} [GeV]",j+1));
      h1_[Form("eta_jet%d_%s" , j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d  #eta",j+1));
      h1_[Form("phi_jet%d_%s" , j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d  #phi",j+1));
      h1_[Form("btag_jet%d_%s", j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d btag discriminator",j+1)); 
      h1_[Form("btaglog_jet%d_%s", j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d -ln(1-btag discriminator)",j+1)); 
      h1_[Form("qglikelihood_jet%d_%s", j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d q-g likelihood",j+1)); 
      h1_[Form("nconstituents_jet%d_%s", j+1,label.c_str())]-> GetXaxis() -> SetTitle(Form("Jet %d n constituents",j+1)); 
      
      if ( config_->btagalgo_ == "deepcsv")
      {
         h1_[Form("btag_light_jet%d_%s", j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d", j+1) , Form("btag_light_jet%d_%s", j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_c_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d"    , j+1) , Form("btag_c_jet%d_%s"    , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_b_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d"    , j+1) , Form("btag_b_jet%d_%s"    , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_bb_jet%d_%s"   , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d"   , j+1) , Form("btag_bb_jet%d_%s"   , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_cc_jet%d_%s"   , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_cc_jet%d"   , j+1) , Form("btag_cc_jet%d_%s"   , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
      }
      if ( config_->btagalgo_ == "deepflavour")
      {
         h1_[Form("btag_light_jet%d_%s", j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d", j+1) , Form("btag_light_jet%d_%s", j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_g_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_g_jet%d"    , j+1) , Form("btag_g_jet%d_%s"    , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_c_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d"    , j+1) , Form("btag_c_jet%d_%s"    , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_b_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d"    , j+1) , Form("btag_b_jet%d_%s"    , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_bb_jet%d_%s"   , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d"   , j+1) , Form("btag_bb_jet%d_%s"   , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
         h1_[Form("btag_lepb_jet%d_%s" , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btag_lepb_jet%d" , j+1) , Form("btag_lepb_jet%d_%s" , j+1,label.c_str()) , nbins_btag, &bins_btag[0] );         
//          h1_[Form("btaglog_light_jet%d_%s", j+1,label.c_str())] = std::make_shared<TH1F>(Form("btaglog_light_jet%d", j+1) , Form("btaglog_light_jet%d_%s", j+1,label.c_str()), 200 , 1.e-6, 10  );         
//          h1_[Form("btaglog_g_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btaglog_g_jet%d"    , j+1) , Form("btaglog_g_jet%d_%s"    , j+1,label.c_str()), 200 , 1.e-6, 10  );         
//          h1_[Form("btaglog_c_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btaglog_c_jet%d"    , j+1) , Form("btaglog_c_jet%d_%s"    , j+1,label.c_str()), 200 , 1.e-6, 10  );         
//          h1_[Form("btaglog_b_jet%d_%s"    , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btaglog_b_jet%d"    , j+1) , Form("btaglog_b_jet%d_%s"    , j+1,label.c_str()), 200 , 1.e-6, 10  );         
//          h1_[Form("btaglog_bb_jet%d_%s"   , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btaglog_bb_jet%d"   , j+1) , Form("btaglog_bb_jet%d_%s"   , j+1,label.c_str()), 200 , 1.e-6, 10  );         
//          h1_[Form("btaglog_lepb_jet%d_%s" , j+1,label.c_str())] = std::make_shared<TH1F>(Form("btaglog_lepb_jet%d" , j+1) , Form("btaglog_lepb_jet%d_%s" , j+1,label.c_str()), 200 , 1.e-6, 10  );         
      }
      
      // 2D histograms
      h2_[Form("pt_eta_jet%d_%s"  , j+1,label.c_str())]  = std::make_shared<TH2F>(Form("pt_eta_jet%d"  , j+1) , Form("pt_eta_jet%d_%s"  , j+1,label.c_str()) ,1500 , 0   , 1500, 600, -3, 3  );
      h2_[Form("pt_eta_jet%d_%s"  , j+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d p_{T} [GeV]",j+1));
      h2_[Form("pt_eta_jet%d_%s"  , j+1,label.c_str())] -> GetYaxis() -> SetTitle(Form("Jet %d  #eta",j+1));
      
      if ( config_->isMC() && ( config_->useJetsFlavour() || config_->useJetsExtendedFlavour() ) )
      {
         for ( auto & flv : flavours_ ) // flavour dependent histograms
         {
            // 1D histograms
            h1_[Form("pt_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str())]  = std::make_shared<TH1F>(Form("pt_jet%d_%s"   , j+1, flv.c_str()) , Form("pt_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str()) ,1500 , 0   , 1500  );
            h1_[Form("eta_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())]  = std::make_shared<TH1F>(Form("eta_jet%d_%s"  , j+1, flv.c_str()) , Form("eta_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str()) , 600 , -3, 3 );
            h1_[Form("phi_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())]  = std::make_shared<TH1F>(Form("phi_jet%d_%s"  , j+1, flv.c_str()) , Form("phi_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str()) , 360 , -180, 180 );
            h1_[Form("btag_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())]  = std::make_shared<TH1F>(Form("btag_jet%d_%s" , j+1, flv.c_str()) , Form("btag_jet%d_%s_%s", j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );
            h1_[Form("qglikelihood_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())]  = std::make_shared<TH1F>(Form("qglikelihood_jet%d_%s" , j+1, flv.c_str()) , Form("qglikelihood_jet%d_%s_%s", j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );
            h1_[Form("nconstituents_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())]  = std::make_shared<TH1F>(Form("nconstituents_jet%d_%s" , j+1, flv.c_str()) , Form("nconstituents_jet%d_%s_%s", j+1,label.c_str(),flv.c_str()) , 200 , 0, 200 );
            
            h1_[Form("pt_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d (%s) p_{T} [GeV]"        , j+1, flv.c_str()));
            h1_[Form("eta_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d (%s)  #eta"              , j+1, flv.c_str()));
            h1_[Form("phi_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d (%s)  #phi"              , j+1, flv.c_str()));
            h1_[Form("btag_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d (%s) btag discriminator" , j+1, flv.c_str())); 
            h1_[Form("qglikelihood_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d (%s) q-g likelihood" , j+1, flv.c_str())); 
            h1_[Form("nconstituents_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d (%s) n constituents" , j+1, flv.c_str())); 
            
            if ( config_->btagalgo_ == "deepcsv")
            {
               h1_[Form("btag_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d_%s", j+1,flv.c_str()) , Form("btag_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d_%s"    , j+1,flv.c_str()) , Form("btag_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d_%s"    , j+1,flv.c_str()) , Form("btag_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d_%s"   , j+1,flv.c_str()) , Form("btag_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_cc_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_cc_jet%d_%s"   , j+1,flv.c_str()) , Form("btag_cc_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
            }
            if ( config_->btagalgo_ == "deepflavour")
            {
               h1_[Form("btag_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d_%s", j+1,flv.c_str()) , Form("btag_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_g_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_g_jet%d_%s"    , j+1,flv.c_str()) , Form("btag_g_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d_%s"    , j+1,flv.c_str()) , Form("btag_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d_%s"    , j+1,flv.c_str()) , Form("btag_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d_%s"   , j+1,flv.c_str()) , Form("btag_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
               h1_[Form("btag_lepb_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btag_lepb_jet%d_%s" , j+1,flv.c_str()) , Form("btag_lepb_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str()) , nbins_btag, &bins_btag[0] );         
//                h1_[Form("btaglog_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btaglog_light_jet%d_%s", j+1,flv.c_str()) , Form("btaglog_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str()) , 200 , 1.e-6, 10  );         
//                h1_[Form("btaglog_g_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btaglog_g_jet%d_%s"    , j+1,flv.c_str()) , Form("btaglog_g_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , 200 , 1.e-6, 10  );         
//                h1_[Form("btaglog_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btaglog_c_jet%d_%s"    , j+1,flv.c_str()) , Form("btaglog_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , 200 , 1.e-6, 10  );         
//                h1_[Form("btaglog_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btaglog_b_jet%d_%s"    , j+1,flv.c_str()) , Form("btaglog_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str()) , 200 , 1.e-6, 10  );         
//                h1_[Form("btaglog_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btaglog_bb_jet%d_%s"   , j+1,flv.c_str()) , Form("btaglog_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str()) , 200 , 1.e-6, 10  );         
//                h1_[Form("btaglog_lepb_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())] = std::make_shared<TH1F>(Form("btaglog_lepb_jet%d_%s" , j+1,flv.c_str()) , Form("btaglog_lepb_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str()) , 200 , 1.e-6, 10  );         
            }
            // 2D histograms
            h2_[Form("pt_eta_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str())]  = std::make_shared<TH2F>(Form("pt_eta_jet%d_%s" , j+1,flv.c_str()) , Form("pt_eta_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str()) ,1500 , 0   , 1500, 600, -3, 3  );
            h2_[Form("pt_eta_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d (%s) p_{T} [GeV]" , j+1,flv.c_str()));
            h2_[Form("pt_eta_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str())] -> GetYaxis() -> SetTitle(Form("Jet %d (%s)  #eta" ,j+1,flv.c_str()));

         }
      }
      if ( config_->doDijet() || config_->doDijetFlavour() )  // dijet histograms
      {
         for ( int k = j+1; k < n && j < n; ++k )
         {
            h1_[Form("dptrel_jet%d%d_%s" , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("dptrel_jet%d%d" , j+1,k+1) , Form("dptrel_jet%d%d_%s" , j+1,k+1,label.c_str()) ,1000 , 0,1 );
            h1_[Form("dpt_jet%d%d_%s"    , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("dpt_jet%d%d" , j+1,k+1)    , Form("dpt_jet%d%d_%s"    , j+1,k+1,label.c_str()) ,1000 , 0,1000 );
            h1_[Form("dr_jet%d%d_%s"     , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("dr_jet%d%d"  , j+1,k+1)    , Form("dr_jet%d%d_%s"     , j+1,k+1,label.c_str()) , 100 , 0, 5 );
            h1_[Form("deta_jet%d%d_%s"   , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("deta_jet%d%d", j+1,k+1)    , Form("deta_jet%d%d_%s"   , j+1,k+1,label.c_str()) , 100 , 0,10 );
            h1_[Form("dphi_jet%d%d_%s"   , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("dphi_jet%d%d", j+1,k+1)    , Form("dphi_jet%d%d_%s"   , j+1,k+1,label.c_str()) , 315 , 0, 3.15 );
            h1_[Form("pt_jet%d%d_%s"     , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("pt_jet%d%d"  , j+1,k+1)    , Form("pt_jet%d%d_%s"     , j+1,k+1,label.c_str()) , 300 , 0,3000 );
            h1_[Form("eta_jet%d%d_%s"    , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("eta_jet%d%d" , j+1,k+1)    , Form("eta_jet%d%d_%s"    , j+1,k+1,label.c_str()) , 200 , -10,10 );
            h1_[Form("phi_jet%d%d_%s"    , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("phi_jet%d%d" , j+1,k+1)    , Form("phi_jet%d%d_%s"    , j+1,k+1,label.c_str()) , 360 , -180,180 );
            h1_[Form("m_jet%d%d_%s"      , j+1,k+1,label.c_str())]  = std::make_shared<TH1F>(Form("m_jet%d%d"   , j+1,k+1)    , Form("m_jet%d%d_%s"      , j+1,k+1,label.c_str()) ,3000 , 0,3000 );
            
            h1_[Form("dptrel_jet%d%d_%s" , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#DeltaP_{T}(Jet %d, Jet %d)/Jet %d p_{T}",j+1,k+1,j+1));
            h1_[Form("dpt_jet%d%d_%s"    , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#Delta p_{T}(Jet %d, Jet %d) [GeV]",j+1,k+1));
            h1_[Form("dr_jet%d%d_%s"     , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#DeltaR(Jet %d, Jet %d)",j+1,k+1));
            h1_[Form("deta_jet%d%d_%s"   , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#Delta#eta(Jet %d, Jet %d)",j+1,k+1));
            h1_[Form("dphi_jet%d%d_%s"   , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("#Delta#phi(Jet %d, Jet %d)",j+1,k+1));
            h1_[Form("pt_jet%d%d_%s"     , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d + Jet %d p_{T} [GeV]",j+1,k+1));
            h1_[Form("eta_jet%d%d_%s"    , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d + Jet %d  #eta",j+1,k+1));
            h1_[Form("phi_jet%d%d_%s"    , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("Jet %d + Jet %d  #phi",j+1,k+1));
            h1_[Form("m_jet%d%d_%s"      , j+1,k+1,label.c_str())] -> GetXaxis() -> SetTitle(Form("M_{%d%d} [GeV]",j+1,k+1));
            
            if ( config_->isMC() && config_->doDijetFlavour() )
            {
               for ( auto & flv1 : flavours_ )
               {
                  for ( auto & flv2 : flavours_ )
                  {
                     h1_[Form("dptrel_jet%d%d_%s_%s_%s" , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("dptrel_jet%d%d_%s_%s" , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("dptrel_jet%d%d_%s_%s_%s" , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 100 , 0,1 );
                     h1_[Form("dpt_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("dpt_jet%d%d_%s_%s"    , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("dpt_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 100 , 0,1000 );
                     h1_[Form("dr_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("dr_jet%d%d_%s_%s"     , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("dr_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 100 , 0, 5 );
                     h1_[Form("deta_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("deta_jet%d%d_%s_%s"   , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("deta_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 100 , 0,10 );
                     h1_[Form("dphi_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("dphi_jet%d%d_%s_%s"   , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("dphi_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 315 , 0, 3.15 );
                     h1_[Form("pt_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("pt_jet%d%d_%s_%s"     , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("pt_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 300 , 0,300 );
                     h1_[Form("eta_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("eta_jet%d%d_%s_%s"    , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("eta_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 200 , -10,10 );
                     h1_[Form("phi_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("phi_jet%d%d_%s_%s"    , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("phi_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) , 360 , -180,180 );
                     h1_[Form("m_jet%d%d_%s_%s_%s"      , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  = std::make_shared<TH1F>(Form("m_jet%d%d_%s_%s"      , j+1,k+1,flv1.c_str(),flv2.c_str()) , Form("m_jet%d%d_%s_%s_%s"      , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str()) ,3000 , 0,300 );
                     
                     h1_[Form("dptrel_jet%d%d_%s_%s_%s" , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("#DeltaP_{T}(Jet %d (%s), Jet %d (%s))/Jet %d p_{T}",j+1,flv1.c_str(),k+1,flv2.c_str(),j+1));
                     h1_[Form("dpt_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("#Delta p_{T}(Jet %d (%s), Jet %d (%s)) [GeV]",j+1,flv1.c_str(),k+1,flv2.c_str()));
                     h1_[Form("dr_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("#DeltaR(Jet %d (%s), Jet %d (%s))",j+1,flv1.c_str(),k+1,flv2.c_str()));
                     h1_[Form("deta_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("#Delta#eta(Jet %d (%s), Jet %d (%s))",j+1,flv1.c_str(),k+1,flv2.c_str()));
                     h1_[Form("dphi_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("#Delta#phi(Jet %d (%s), Jet %d (%s))",j+1,flv1.c_str(),k+1,flv2.c_str()));
                     h1_[Form("pt_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("Jet %d (%s) + Jet %d (%s) p_{T} [GeV]",j+1,flv1.c_str(),k+1,flv2.c_str()));
                     h1_[Form("eta_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("Jet %d (%s) + Jet %d (%s)  #eta",j+1,flv1.c_str(),k+1,flv2.c_str()));
                     h1_[Form("phi_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("Jet %d (%s) + Jet %d (%s)  #phi",j+1,flv1.c_str(),k+1,flv2.c_str()));
                     h1_[Form("m_jet%d%d_%s_%s_%s"      , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> GetXaxis() -> SetTitle(Form("M_{%d%d} (%s)(%s) [GeV]",j+1,k+1,flv1.c_str(),flv2.c_str()));                                          
                  }
               }
            }
         }
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
   if ( config_->triggerObjectsL1Jets() == "" && config_->triggerObjectsCaloJets() == "" && config_->triggerObjectsPFJets() == "") return true;
   
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
   if ( ! jet->matched(config_->triggerObjectsL1Jets()   ) ) return false;
   if ( ! jet->matched(config_->triggerObjectsCaloJets() ) ) return false;
   if ( ! jet->matched(config_->triggerObjectsPFJets()   ) ) return false;

   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}


bool JetAnalyser::onlineBJetMatching(const int & r)
{
   int j = r-1;
   if ( config_->triggerObjectsBJets() == "" ) return true;
   
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
   if ( ! jet->matched(config_->triggerObjectsBJets()) ) return false;
   
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
      // 1D histograms
      h1_[Form("pt_jet%d_%s",j+1,label.c_str())]   -> Fill(selectedJets_[j]->pt(),weight_);
      h1_[Form("eta_jet%d_%s",j+1,label.c_str())]  -> Fill(selectedJets_[j]->eta(),weight_);
      h1_[Form("phi_jet%d_%s",j+1,label.c_str())]  -> Fill(selectedJets_[j]->phi()*180./acos(-1.),weight_);
      float mybtag = btag(*selectedJets_[j],config_->btagalgo_);
      float mybtaglog = 1.e-7;
      if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
      h1_[Form("btag_jet%d_%s",j+1,label.c_str())]    -> Fill(mybtag,weight_);
      h1_[Form("btaglog_jet%d_%s",j+1,label.c_str())] -> Fill(mybtaglog,weight_);
      h1_[Form("qglikelihood_jet%d_%s", j+1,label.c_str())] -> Fill(selectedJets_[j]->qgLikelihood(),weight_);
      h1_[Form("nconstituents_jet%d_%s", j+1,label.c_str())] -> Fill(selectedJets_[j]->constituents(),weight_);
      
      if ( config_->btagalgo_ == "deepcsv")
      {
         h1_[Form("btag_light_jet%d_%s", j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deeplight"),weight_);
         h1_[Form("btag_c_jet%d_%s"    , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepc"),weight_); 
         h1_[Form("btag_b_jet%d_%s"    , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepb"),weight_); 
         h1_[Form("btag_bb_jet%d_%s"   , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepbb"),weight_); 
         h1_[Form("btag_cc_jet%d_%s"   , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepcc"),weight_); 
      }
      if ( config_->btagalgo_ == "deepflavour")
      {
         h1_[Form("btag_light_jet%d_%s", j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dflight"),weight_);
         h1_[Form("btag_g_jet%d_%s"    , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfg")    ,weight_); 
         h1_[Form("btag_c_jet%d_%s"    , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfc")    ,weight_); 
         h1_[Form("btag_b_jet%d_%s"    , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfb")    ,weight_); 
         h1_[Form("btag_bb_jet%d_%s"   , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfbb")   ,weight_); 
         h1_[Form("btag_lepb_jet%d_%s" , j+1,label.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dflepb") ,weight_); 
         
//          mybtag = selectedJets_[j]->btag("btag_dflight");
//          mybtaglog = 1.e-7;
//          if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//          h1_[Form("btaglog_light_jet%d_%s", j+1,label.c_str())]  -> Fill(mybtaglog ,weight_);
//          mybtag = selectedJets_[j]->btag("btag_dfg");
//          mybtaglog = 1.e-7;
//          if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//          h1_[Form("btaglog_g_jet%d_%s"    , j+1,label.c_str())]  -> Fill(mybtaglog ,weight_); 
//          mybtag = selectedJets_[j]->btag("btag_dfc");
//          mybtaglog = 1.e-7;
//          if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//          h1_[Form("btaglog_c_jet%d_%s"    , j+1,label.c_str())]  -> Fill(mybtaglog ,weight_); 
//          mybtag = selectedJets_[j]->btag("btag_dfb");
//          mybtaglog = 1.e-7;
//          if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//          h1_[Form("btaglog_b_jet%d_%s"    , j+1,label.c_str())]  -> Fill(mybtaglog ,weight_); 
//          mybtag = selectedJets_[j]->btag("btag_dfbb");
//          mybtaglog = 1.e-7;
//          if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//          h1_[Form("btaglog_bb_jet%d_%s"   , j+1,label.c_str())]  -> Fill(mybtaglog ,weight_); 
//          mybtag = selectedJets_[j]->btag("btag_dflepb");
//          mybtaglog = 1.e-7;
//          if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//          h1_[Form("btaglog_lepb_jet%d_%s" , j+1,label.c_str())]  -> Fill(mybtaglog ,weight_); 

      }
      // 2D histograms
      h2_[Form("pt_eta_jet%d_%s"  , j+1,label.c_str())] -> Fill(selectedJets_[j]->pt(), selectedJets_[j]->eta(), weight_);
      
      if ( config_->isMC() && ( config_ -> useJetsFlavour() || config_ -> useJetsExtendedFlavour() ))
      {
         std::string flv = "udsg";
         if ( config_ -> useJetsFlavour() )
         {
            if ( abs(selectedJets_[j]->flavour()) == 4 ) flv = "c"; 
            if ( abs(selectedJets_[j]->flavour()) == 5 ) flv = "b"; 
         }
         if ( config_ -> useJetsExtendedFlavour() ) flv = selectedJets_[j]->extendedFlavour();
         // 1D histograms
         h1_[Form("pt_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->pt(),weight_);
         h1_[Form("eta_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->eta(),weight_);
         h1_[Form("phi_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->phi()*180./acos(-1.),weight_);
         h1_[Form("btag_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())]  -> Fill(btag(*selectedJets_[j],config_->btagalgo_),weight_);
         h1_[Form("qglikelihood_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] -> Fill(selectedJets_[j]->qgLikelihood(),weight_);
         h1_[Form("nconstituents_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())] -> Fill(selectedJets_[j]->constituents(),weight_);
         if ( config_->btagalgo_ == "deepcsv")
         {
            h1_[Form("btag_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deeplight"),weight_);
            h1_[Form("btag_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepc"),weight_); 
            h1_[Form("btag_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepb"),weight_); 
            h1_[Form("btag_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepbb"),weight_); 
            h1_[Form("btag_cc_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_deepcc"),weight_); 
         }
         if ( config_->btagalgo_ == "deepflavour")
         {
            h1_[Form("btag_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dflight"),weight_);
            h1_[Form("btag_g_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfg")    ,weight_); 
            h1_[Form("btag_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfc")    ,weight_); 
            h1_[Form("btag_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfb")    ,weight_); 
            h1_[Form("btag_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dfbb")   ,weight_); 
            h1_[Form("btag_lepb_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())]  -> Fill(selectedJets_[j]->btag("btag_dflepb") ,weight_); 
            
//             mybtag = selectedJets_[j]->btag("btag_dflight");
//             mybtaglog = 1.e-7;
//             if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//             h1_[Form("btag_light_jet%d_%s_%s", j+1,label.c_str(),flv.c_str())]  -> Fill(mybtaglog ,weight_);
//             mybtag = selectedJets_[j]->btag("btag_dfg");
//             mybtaglog = 1.e-7;
//             if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//             h1_[Form("btag_g_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(mybtaglog ,weight_); 
//             mybtag = selectedJets_[j]->btag("btag_dfc");
//             mybtaglog = 1.e-7;
//             if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//             h1_[Form("btag_c_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(mybtaglog ,weight_); 
//             mybtag = selectedJets_[j]->btag("btag_dfb");
//             mybtaglog = 1.e-7;
//             if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//             h1_[Form("btag_b_jet%d_%s_%s"    , j+1,label.c_str(),flv.c_str())]  -> Fill(mybtaglog ,weight_); 
//             mybtag = selectedJets_[j]->btag("btag_dfbb");
//             mybtaglog = 1.e-7;
//             if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//             h1_[Form("btag_bb_jet%d_%s_%s"   , j+1,label.c_str(),flv.c_str())]  -> Fill(mybtaglog ,weight_); 
//             mybtag = selectedJets_[j]->btag("btag_dflepb");
//             mybtaglog = 1.e-7;
//             if ( mybtag > 0 ) mybtaglog = -log(1.-mybtag);
//             h1_[Form("btag_lepb_jet%d_%s_%s" , j+1,label.c_str(),flv.c_str())]  -> Fill(mybtaglog ,weight_); 
            
         }
         // 2D histograms
         h2_[Form("pt_eta_jet%d_%s_%s"  , j+1,label.c_str(),flv.c_str())] -> Fill(selectedJets_[j]->pt(), selectedJets_[j]->eta(), weight_);
      }
      
      if ( config_ -> doDijet() || config_->doDijetFlavour() )
      {
         for ( int k = j+1; k < n && j < n; ++k )
         {
            Composite<Jet,Jet> c_ij(*(selectedJets_[j]),*(selectedJets_[k]));
            
            h1_[Form("dptrel_jet%d%d_%s" , j+1,k+1,label.c_str())] -> Fill(fabs(selectedJets_[j]->pt()-selectedJets_[k]->pt())/selectedJets_[j]->pt(),weight_);
            h1_[Form("dpt_jet%d%d_%s" , j+1,k+1,label.c_str())]    -> Fill(fabs(selectedJets_[j]->pt()-selectedJets_[k]->pt()),weight_);
            h1_[Form("dr_jet%d%d_%s",j+1,k+1,label.c_str())]       -> Fill(c_ij.deltaR(),weight_);
            h1_[Form("deta_jet%d%d_%s",j+1,k+1,label.c_str())]     -> Fill(c_ij.deltaEta(),weight_);
            h1_[Form("dphi_jet%d%d_%s",j+1,k+1,label.c_str())]     -> Fill(fabs(selectedJets_[j]->deltaPhi(*selectedJets_[k])));
            h1_[Form("pt_jet%d%d_%s",j+1,k+1,label.c_str())]       -> Fill(c_ij.pt(),weight_);
            h1_[Form("eta_jet%d%d_%s",j+1,k+1,label.c_str())]      -> Fill(c_ij.eta(),weight_);
            h1_[Form("phi_jet%d%d_%s",j+1,k+1,label.c_str())]      -> Fill(c_ij.phi()*180./acos(-1.),weight_);
            if ( config_->isMC() || !config_->signalRegion() )
            {
               h1_[Form("m_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(c_ij.m(),weight_);
            }
            else  // blind
            {
               h1_[Form("m_jet%d%d_%s",j+1,k+1,label.c_str())]  -> Fill(0.,weight_);
            }
            if ( config_->isMC() && config_->doDijetFlavour() )
            {
               std::string flv1 = "udsg";
               std::string flv2 = "udsg";
               if ( config_ -> useJetsFlavour() )
               {
                  if ( abs(selectedJets_[j]->flavour()) == 4 ) flv1 = "c"; 
                  if ( abs(selectedJets_[j]->flavour()) == 5 ) flv1 = "b"; 
                  if ( abs(selectedJets_[k]->flavour()) == 4 ) flv2 = "c"; 
                  if ( abs(selectedJets_[k]->flavour()) == 5 ) flv2 = "b"; 
               }
               if ( config_ -> useJetsExtendedFlavour() )
               {
                  flv1 = selectedJets_[j]->extendedFlavour();
                  flv2 = selectedJets_[k]->extendedFlavour();
               }
               h1_[Form("dptrel_jet%d%d_%s_%s_%s" , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(fabs(selectedJets_[j]->pt()-selectedJets_[k]->pt())/selectedJets_[j]->pt(),weight_);
               h1_[Form("dpt_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(fabs(selectedJets_[j]->pt()-selectedJets_[k]->pt()),weight_);
               h1_[Form("dr_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(c_ij.deltaR(),weight_);
               h1_[Form("deta_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(c_ij.deltaEta(),weight_);
               h1_[Form("dphi_jet%d%d_%s_%s_%s"   , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(fabs(selectedJets_[j]->deltaPhi(*selectedJets_[k])));
               h1_[Form("pt_jet%d%d_%s_%s_%s"     , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(c_ij.pt(),weight_);
               h1_[Form("eta_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(c_ij.eta(),weight_);
               h1_[Form("phi_jet%d%d_%s_%s_%s"    , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(c_ij.phi()*180./acos(-1.),weight_);
               h1_[Form("m_jet%d%d_%s_%s_%s"      , j+1,k+1,label.c_str(),flv1.c_str(),flv2.c_str())]  -> Fill(c_ij.m(),weight_);

            }
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
   if ( ! applyjer_ )  return;  // will not apply btag SF
   
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
      h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,"*** Apply JER smearing");
   
   
   for ( auto & j : selectedJets_ )
   {
      j -> applyJER(*jerinfo_,0.2);
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
   {
      if ( config_->jetsBtagWP()[j] == "xxx" )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: btag SF = 1 applied (%s %s WP)",r,config_->btagalgo_.c_str(),config_->jetsBtagWP()[j].c_str()));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: btag SF applied (%s %s WP)",r,config_->btagalgo_.c_str(),config_->jetsBtagWP()[j].c_str()));
   }
   
   float sf = 1.;
   if ( config_->jetsBtagWP()[j] != "xxx" )  sf = this->btagSF(r,config_->jetsBtagWP()[j]).nominal;
   
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



bool JetAnalyser::selectionJetPtImbalance(const int & j1, const int & j2, const float & delta)
{
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( delta > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("DpT(jet %d, jet %d)/jet %d pT < %4.2f",j1,j2,j1,fabs(delta)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("DpT(jet %d, jet %d)/jet %d pT > %4.2f",j1,j2,j1,fabs(delta)));
   }
   
   
   if ( (int)selectedJets_.size() < j1 || (int)selectedJets_.size() < j2 )
   {
      std::cout << "-errr- JetAnalyser::selectionJetDptrel(): you dont have enough selected jets. Will return false" << std::endl;
      return false;
   }
   if ( delta > 0 )
   {
      if ( fabs(selectedJets_[j1-1]->pt() - selectedJets_[j2-1]->pt())/selectedJets_[j1-1]->pt() > fabs(delta) ) return false;
   }
   else
   {
      if ( fabs(selectedJets_[j1-1]->pt() - selectedJets_[j2-1]->pt())/selectedJets_[j1-1]->pt() < fabs(delta) ) return false;
   }

        
   h1_["cutflow"] -> Fill(cutflow_,weight_);
    
   return true;
   
}
bool JetAnalyser::selectionJetPtImbalance(const int & j1, const int & j2)
{
   bool ok = true;
   if (config_->ptimbalmax_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetPtImbalance(j1,j2,config_->ptimbalmax_);
   }
   
   if (config_->ptimbalmin_ < 0 )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetPtImbalance(j1,j2,-1*config_->ptimbalmin_);
   }
   return ok;
   
}

bool JetAnalyser::selectionJetQGlikelihood(const int & r, const float & cut)
{
   int j = r-1;
   
   ++cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" )
   {
      if ( cut > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d Q-G likelihood < %4.2f",r,fabs(cut)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d Q-G likelihood > %4.2f",r,fabs(cut)));
   }
   
   if ( cut > 0 )
   {
      if ( selectedJets_[j]->qgLikelihood() > fabs(cut) ) return false;
   }
   else
   {
      if ( selectedJets_[j]->qgLikelihood() < fabs(cut) ) return false;
   }
        
   h1_["cutflow"] -> Fill(cutflow_,weight_);
    
   return true;
   
}

bool JetAnalyser::selectionJetQGlikelihood(const int & r)
{
   int j = r-1;
   bool ok = true;
   if ( config_->qgmax_.size() == 0 || config_->qgmax_[j] < 0 || (int)config_->qgmax_.size() < r )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetQGlikelihood(r,config_->qgmax_[j]);
   }
   
   if (config_->qgmin_.size() == 0 || config_->qgmin_[j] < 0 || (int)config_->qgmin_.size() < r )
   {
      ok = ok && true;
   }
   else
   {
      ok = ok && selectionJetQGlikelihood(r,-1*config_->qgmin_[j] );
   }
   return ok;
   
}


bool JetAnalyser::selectionBJetProbB(const int & r )
{
   if ( config_->jetsBtagProbB().size() == 0 ) return true;
   int j = r-1;
   float wp = config_->jetsBtagProbB()[j];
   std::string algo = config_->btagalgo_;
   if ( fabs(wp) > 1 ) return true; // there is no selection here, so will not update the cutflow
   
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
   {
      if ( wp > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob b < %6.4f",r,algo.c_str(),fabs(wp)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob b > %6.4f",r,algo.c_str(),fabs(wp)));
   }
         
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbB(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
   if ( algo == "deepcsv" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_deepb") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_deepb") < fabs(wp) ) return false;
   }
   if ( algo == "deepflavour" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_dfb") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_dfb") < fabs(wp) ) return false;
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}




bool JetAnalyser::selectionBJetProbBB(const int & r )
{
   if ( config_->jetsBtagProbBB().size() == 0 ) return true;
   int j = r-1;
   float wp = config_->jetsBtagProbBB()[j];
   std::string algo = config_->btagalgo_;
   if ( fabs(wp) > 1 ) return true; // there is no selection here, so will not update the cutflow
   
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
   {
      if ( wp > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob bb < %6.4f",r,algo.c_str(),fabs(wp)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob bb > %6.4f",r,algo.c_str(),fabs(wp)));
   }
         
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbBB(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
   if ( algo == "deepcsv" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_deepbb") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_deepbb") < fabs(wp) ) return false;
   }
   if ( algo == "deepflavour" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_dfbb") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_dfbb") < fabs(wp) ) return false;
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

bool JetAnalyser::selectionBJetProbLepB(const int & r )
{
   if ( config_->jetsBtagProbLepB().size() == 0 ) return true;
   int j = r-1;
   float wp = config_->jetsBtagProbLepB()[j];
   std::string algo = config_->btagalgo_;
   if ( fabs(wp) > 1 || algo == "deepcsv" ) return true; // there is no selection here, so will not update the cutflow
   
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
   {
      if ( wp > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob lepb < %6.4f",r,algo.c_str(),fabs(wp)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob lepb > %6.4f",r,algo.c_str(),fabs(wp)));
   }
         
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbLepB(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
   if ( algo == "deepflavour" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_dflepb") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_dflepb") < fabs(wp) ) return false;
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}



bool JetAnalyser::selectionBJetProbC(const int & r )
{
   if ( config_->jetsBtagProbC().size() == 0 ) return true;
   int j = r-1;
   float wp = config_->jetsBtagProbC()[j];
   std::string algo = config_->btagalgo_;
   if ( fabs(wp) > 1 ) return true; // there is no selection here, so will not update the cutflow
   
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
   {
      if ( wp > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob c < %6.4f",r,algo.c_str(),fabs(wp)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob c > %6.4f",r,algo.c_str(),fabs(wp)));
   }
         
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbC(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
   if ( algo == "deepcsv" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_deepc") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_deepc") < fabs(wp) ) return false;
   }
   if ( algo == "deepflavour" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_dfc") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_dfc") < fabs(wp) ) return false;
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

bool JetAnalyser::selectionBJetProbG(const int & r )
{
   if ( config_->jetsBtagProbG().size() == 0 ) return true;
   int j = r-1;
   float wp = config_->jetsBtagProbG()[j];
   std::string algo = config_->btagalgo_;
   if ( fabs(wp) > 1 || algo == "deepcsv" ) return true; // there is no selection here, so will not update the cutflow
   
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
   {
      if ( wp > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob g < %6.4f",r,algo.c_str(),fabs(wp)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob g > %6.4f",r,algo.c_str(),fabs(wp)));
   }
         
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbG(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
   if ( algo == "deepflavour" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_dfg") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_dfg") < fabs(wp) ) return false;
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}

bool JetAnalyser::selectionBJetProbLight(const int & r )
{
   if ( config_->jetsBtagProbLight().size() == 0 ) return true;
   int j = r-1;
   float wp = config_->jetsBtagProbLight()[j];
   std::string algo = config_->btagalgo_;
   if ( fabs(wp) > 1 ) return true; // there is no selection here, so will not update the cutflow
   
   ++ cutflow_;
   if ( std::string(h1_["cutflow"] -> GetXaxis()-> GetBinLabel(cutflow_+1)) == "" ) 
   {
      if ( wp > 0 )
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob light < %6.4f",r,algo.c_str(),fabs(wp)));
      else
         h1_["cutflow"] -> GetXaxis()-> SetBinLabel(cutflow_+1,Form("Jet %d: %s btag prob light > %6.4f",r,algo.c_str(),fabs(wp)));
   }
         
   
   if ( r > config_->nbjetsmin_ ) 
   {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbLight(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   
   // jet  btag
   if ( algo == "deepcsv" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_deeplight") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_deeplight") < fabs(wp) ) return false;
   }
   if ( algo == "deepflavour" )
   {
      if ( wp > 0 && selectedJets_[j]->btag("btag_dflight") > fabs(wp) ) return false;
      if ( wp < 0 && selectedJets_[j]->btag("btag_dflight") < fabs(wp) ) return false;
   }
   
   h1_["cutflow"] -> Fill(cutflow_,weight_);
   
   return true;
}


