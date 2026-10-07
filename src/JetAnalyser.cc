#include "Analysis/Tools/interface/JetAnalyser.h"

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

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

bool btag_ordering( const std::shared_ptr<Jet> & j1, const std::shared_ptr<Jet> & j2){ return ( j1->btag() > j2->btag() );}
bool jetpt_ordering( const std::shared_ptr<Jet> & j1, const std::shared_ptr<Jet> & j2){ return ( j1->pt() > j2->pt() );}


JetAnalyser::JetAnalyser() {
   //
}

JetAnalyser::JetAnalyser(int argc, char *argv[]) : BaseAnalyser(argc, argv) {
   // Physics objects
   // Jets
   jets_analysis_ = (analysis_->addTree<Jet>("Jets", config_->jetsCollection()) != nullptr);
   apply_jet_resolution_ = false;
   apply_jet_corrections_ = false;
   if (config_->btagScaleFactors() != "") {
      btag_scale_factor_reader_["loose"]  = analysis_->btagCalibration(config_->btagAlgorithm(), config_->btagScaleFactors(), "loose");
      btag_scale_factor_reader_["medium"] = analysis_->btagCalibration(config_->btagAlgorithm(), config_->btagScaleFactors(), "medium");
      btag_scale_factor_reader_["tight"]  = analysis_->btagCalibration(config_->btagAlgorithm(), config_->btagScaleFactors(), "tight");
   }
   for (int mb = 0; mb < 4; ++mb) {
      if (config_->btagEfficiencies(mb + 1) != "") {
         btag_efficiencies_[mb] = BTagEfficiencies(config_->btagEfficiencies(mb + 1));
      }
   }
   if (config_->jerPtRes() != "" && config_->jerSF() != "" && generator_jets_analysis_) { // FIXME: check if files exist
      jet_resolution_info_ = analysis_->jetResolutionInfo(config_->jerPtRes(), config_->jerSF());
      apply_jet_resolution_ = (jet_resolution_info_ != nullptr && jets_analysis_);
   }
   // Jet energy scale corrections are applied when producing the ntuples.
   // Here we apply only systematic variations
   apply_jet_corrections_ = (config_->jecSystematics() != 0);
   if (config_->isMC()) {
      flavours_ = {"udsg", "c", "b"};
      if (config_->useJetsExtendedFlavour()) {
         flavours_.push_back("cc");
         flavours_.push_back("bb");
      }
      if(config_->onlinejetSF() != "" )
         jet_trigger_efficiency_          = std::make_unique<JetTriggerEfficiencies>(config_->onlinejetSF());
      if(config_->onlinebtagSF() != "" )
         btag_trigger_efficiency_         = std::make_unique<BtagTriggerEfficiencies>(config_->onlinebtagSF());
      if(config_->onlinebtagMuonJetSF() != "" )
         btag_muonjet_trigger_efficiency_ = std::make_unique<BtagTriggerEfficiencies>(config_->onlinebtagMuonJetSF());

   }
   //   histograms("jet",config_->nJetsMin());

}

JetAnalyser::~JetAnalyser() {
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//
// ------------ method called for each event  ------------

bool JetAnalyser::analysisWithJets() {
   jets_.clear();
   selected_jets_.clear();
   // trigger emulation
   // L1 jets
   std::string triggerObjectsL1Jets;
   if (config_->triggerObjectsL1Jets() != "") {
      triggerObjectsL1Jets = config_->triggerObjectsL1Jets();
      if (config_->triggerEmulateL1Jets() != "" && config_->triggerEmulateL1JetsNMin() > 0)
      {
         int nmin = config_->triggerEmulateL1JetsNMin();
         float ptmin = config_->triggerEmulateL1JetsPtMin();
         float etamax = config_->triggerEmulateL1JetsEtaMax();
         std::string newL1Jets = config_->triggerEmulateL1Jets();
         triggerEmulation(triggerObjectsL1Jets, nmin, ptmin, etamax, newL1Jets);
         triggerObjectsL1Jets = newL1Jets;
      }
   }
   // Calo jets
   std::string triggerObjectsCaloJets;
   if (config_->triggerObjectsCaloJets() != "") {
      triggerObjectsCaloJets = config_->triggerObjectsCaloJets();
      if (config_->triggerEmulateCaloJets() != "" && config_->triggerEmulateCaloJetsNMin() > 0) {
         int nmin = config_->triggerEmulateCaloJetsNMin();
         float ptmin = config_->triggerEmulateCaloJetsPtMin();
         float etamax = config_->triggerEmulateCaloJetsEtaMax();
         std::string newCaloJets = config_->triggerEmulateCaloJets();
         triggerEmulation(triggerObjectsCaloJets, nmin, ptmin, etamax, newCaloJets);
         triggerObjectsCaloJets = newCaloJets;
      }
   }
   // PF jets
   std::string triggerObjectsPFJets;
   if (config_->triggerObjectsPFJets() != "") {
      triggerObjectsPFJets = config_->triggerObjectsPFJets();
      if (config_->triggerEmulatePFJets() != "" && config_->triggerEmulatePFJetsNMin() > 0) {
         int nmin = config_->triggerEmulatePFJetsNMin();
         float ptmin = config_->triggerEmulatePFJetsPtMin();
         float etamax = config_->triggerEmulatePFJetsEtaMax();
         std::string newPFJets = config_->triggerEmulatePFJets();
         triggerEmulation(triggerObjectsPFJets, nmin, ptmin, etamax, newPFJets);
         triggerObjectsPFJets = newPFJets;
      }
   }
   if (!jets_analysis_)
      return false;
   cutflow(Form("Using Jet collection: %s", (config_->jetsCollection()).c_str()));
   if (config_->triggerObjectsL1Jets() != "")
      analysis_->match<Jet, TriggerObject>("Jets", triggerObjectsL1Jets, config_->triggerMatchL1JetsDrMax());
   if (config_->triggerObjectsCaloJets() != "")
      analysis_->match<Jet, TriggerObject>("Jets", triggerObjectsCaloJets, config_->triggerMatchCaloJetsDrMax());
   if (config_->triggerObjectsPFJets() != "")
      analysis_->match<Jet, TriggerObject>("Jets", triggerObjectsPFJets, config_->triggerMatchPFJetsDrMax());
   analysis_->match<Jet, TriggerObject>("Jets", config_->triggerObjectsBJets(), config_->triggerMatchCaloBJetsDrMax());
   // std::shared_ptr< Collection<Jet> >
   auto jets = analysis_->collection<Jet>("Jets");
   if (generator_particles_analysis_ && config_->useJetsExtendedFlavour()) {
      auto particles = analysis_->collection<GenParticle>("GenParticles");
      jets->associatePartons(particles, 0.4, 1., config_->pythia8());
   }
   if (generator_jets_analysis_) {
      auto genjets = analysis_->collection<GenJet>("GenJets");
      jets->addGenJets(genjets);
   }
   for (int j = 0; j < jets->size(); ++j) {
      jets_.push_back(std::make_shared<Jet>(jets->at(j)));
      jets_.back()->btag(btag(*jets_.back(), config_->btagAlgorithm())); // give to each jet its btag value
   }
   selected_jets_ = jets_;
   return true;
}

void JetAnalyser::jets(const std::string &collection) {
   analysis_->addTree<Jet>("Jets", collection);
}

void JetAnalyser::jetHistograms(const std::string &label) {
   this->jetHistograms(config_->nJetsMin(), label);
}
void JetAnalyser::jetHistograms(const std::string &label, const int &n) {
   this->jetHistograms(label, config_->nJetsMin());
}
void JetAnalyser::jetHistograms(const int &n, const std::string &label) {
   if (label == "") {
      std::cout << "-warning- JetAnalyser::jetHistograms - no label given for histograms (root directory)" << std::endl;
      return;
   }

   output_rootfile_->cd();
   if (!output_rootfile_->FindObjectAny(label.c_str())) {
      output_rootfile_->mkdir(label.c_str());
      output_rootfile_->cd(label.c_str());
   } else {
      if (h1_.find(Form("jet_hist_weight_%s", label.c_str())) != h1_.end()) // the jet histograms already exist
         return;
   }
   num_histograms_jets_ = n;
   this->add1DHistogram(label,"jet_hist_weight","",1,0.,1);
   output_rootfile_->cd(label.c_str()); // TODO need to find a way to remove this line
   // btag variable binning
   float min1 = 0.0;
   float max1 = 0.1;
   float size1 = 0.0001;
   int nbins1 = int((max1 - min1) / size1);
   float min2 = 0.1;
   float max2 = 0.9;
   float size2 = 0.001;
   int nbins2 = int((max2 - min2) / size2);
   float min3 = 0.9;
   float max3 = 1.0;
   float size3 = 0.0001;
   int nbins3 = int((max3 - min3) / size3);
   int nbins_btag = nbins1 + nbins2 + nbins3;
   std::vector<float> bins_btag;
   bins_btag.clear();
   int counter = 0;
   for (int i = 0; i < nbins1; ++i) {
      bins_btag.push_back(min1 + size1 * i);
      ++counter;
   }
   for (int i = 0; i < nbins2; ++i) {
      bins_btag.push_back(min2 + size2 * i);
      ++counter;
   }
   for (int i = 0; i < nbins3 + 1; ++i) {
      bins_btag.push_back(min3 + size3 * i);
      ++counter;
   }
   // uniform binning for btag (comment it out if you want the variable binning above)
   float size = 0.0002;
   nbins_btag = int(1. / size);
   bins_btag.clear();
   for (int i = 0; i < nbins_btag + 1; ++i) {
      bins_btag.push_back(size * i);
      ++counter;
   }
   for (int j = 0; j < n; ++j) { // loop over jets
      // 1D histograms
      h1_[Form("pt_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d", j + 1), Form("pt_jet%d_%s", j + 1, label.c_str()), 1500, 0, 1500);
      h1_[Form("eta_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("eta_jet%d", j + 1), Form("eta_jet%d_%s", j + 1, label.c_str()), 600, -3, 3);
      h1_[Form("phi_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("phi_jet%d", j + 1), Form("phi_jet%d_%s", j + 1, label.c_str()), 360, -180, 180);
      h1_[Form("qglikelihood_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("qglikelihood_jet%d", j + 1), Form("qglikelihood_jet%d_%s", j + 1, label.c_str()), 200, 0, 1);
      h1_[Form("nconstituents_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("nconstituents_jet%d", j + 1), Form("nconstituents_jet%d_%s", j + 1, label.c_str()), 200, 0, 200);
      if (config_->isMC()) {
         h1_[Form("flavour_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("flavour_jet%d", j + 1), Form("flavour_jet%d_%s", j + 1, label.c_str()), 6, 0, 6);
         if (config_->useJetsExtendedFlavour()) {
            h1_[Form("extendedflavour_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("extendedflavour_jet%d", j + 1), Form("extendedflavour_jet%d_%s", j + 1, label.c_str()), 16, 0, 16);
         }
      }
      // pt distributions separate for barrel and endcap, minus and plus sides and overlaps
      if (config_->histogramJetsRegionSplit()) {
         h1_[Form("pt_jet%d_me_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d_me", j + 1), Form("pt_jet%d_me_%s", j + 1, label.c_str()), 1500, 0, 1500);
         h1_[Form("pt_jet%d_pe_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d_pe", j + 1), Form("pt_jet%d_pe_%s", j + 1, label.c_str()), 1500, 0, 1500);
         h1_[Form("pt_jet%d_mb_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d_mb", j + 1), Form("pt_jet%d_mb_%s", j + 1, label.c_str()), 1500, 0, 1500);
         h1_[Form("pt_jet%d_pb_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d_pb", j + 1), Form("pt_jet%d_pb_%s", j + 1, label.c_str()), 1500, 0, 1500);
         h1_[Form("pt_jet%d_mbe_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d_mbe", j + 1), Form("pt_jet%d_mbe_%s", j + 1, label.c_str()), 1500, 0, 1500);
         h1_[Form("pt_jet%d_pbe_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d_pbe", j + 1), Form("pt_jet%d_pbe_%s", j + 1, label.c_str()), 1500, 0, 1500);
      }

      h1_[Form("pt_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d p_{T} [GeV]", j + 1));
      h1_[Form("eta_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d  #eta", j + 1));
      h1_[Form("phi_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d  #phi", j + 1));
      h1_[Form("qglikelihood_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d q-g likelihood", j + 1));
      h1_[Form("nconstituents_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d n constituents", j + 1));

      if (config_->btagAlgorithm() != "") {
         h1_[Form("btag_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_jet%d", j + 1), Form("btag_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
         h1_[Form("btaglog_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btaglog_jet%d", j + 1), Form("btaglog_jet%d_%s", j + 1, label.c_str()), 200, 1.e-6, 10);
         h1_[Form("btag_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d btag discriminator", j + 1));
         h1_[Form("btaglog_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d -ln(1-btag discriminator)", j + 1));

         if (config_->btagAlgorithm() == "deepcsv") {
            h1_[Form("btag_light_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d", j + 1), Form("btag_light_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_c_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d", j + 1), Form("btag_c_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_b_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d", j + 1), Form("btag_b_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_bb_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d", j + 1), Form("btag_bb_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_cc_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_cc_jet%d", j + 1), Form("btag_cc_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
         }
         if (config_->btagAlgorithm() == "deepflavour" || config_->btagAlgorithm() == "deepjet") {
            h1_[Form("btag_light_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d", j + 1), Form("btag_light_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_g_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_g_jet%d", j + 1), Form("btag_g_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_c_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d", j + 1), Form("btag_c_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_b_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d", j + 1), Form("btag_b_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_bb_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d", j + 1), Form("btag_bb_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("btag_lepb_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("btag_lepb_jet%d", j + 1), Form("btag_lepb_jet%d_%s", j + 1, label.c_str()), nbins_btag, &bins_btag[0]);
         }
      }
      // 2D histograms
      h2_[Form("pt_eta_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH2F>(Form("pt_eta_jet%d", j + 1), Form("pt_eta_jet%d_%s", j + 1, label.c_str()), 1500, 0, 1500, 600, -3, 3);
      h2_[Form("pt_eta_jet%d_%s", j + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d p_{T} [GeV]", j + 1));
      h2_[Form("pt_eta_jet%d_%s", j + 1, label.c_str())]->GetYaxis()->SetTitle(Form("Jet %d  #eta", j + 1));

      if (config_->isMC() && config_->histogramJetsPerFlavour()) {
         for (auto &flv : flavours_) {// flavour dependent histograms
            // 1D histograms
            h1_[Form("pt_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d_%s", j + 1, flv.c_str()), Form("pt_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), 1500, 0, 1500);
            h1_[Form("eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("eta_jet%d_%s", j + 1, flv.c_str()), Form("eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), 600, -3, 3);
            h1_[Form("phi_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("phi_jet%d_%s", j + 1, flv.c_str()), Form("phi_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), 360, -180, 180);
            h1_[Form("qglikelihood_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("qglikelihood_jet%d_%s", j + 1, flv.c_str()), Form("qglikelihood_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
            h1_[Form("nconstituents_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("nconstituents_jet%d_%s", j + 1, flv.c_str()), Form("nconstituents_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), 200, 0, 200);

            h1_[Form("pt_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) p_{T} [GeV]", j + 1, flv.c_str()));
            h1_[Form("eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s)  #eta", j + 1, flv.c_str()));
            h1_[Form("phi_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s)  #phi", j + 1, flv.c_str()));
            h1_[Form("qglikelihood_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) q-g likelihood", j + 1, flv.c_str()));
            h1_[Form("nconstituents_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) n constituents", j + 1, flv.c_str()));

            if (config_->btagAlgorithm() != "") {
               h1_[Form("btag_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_jet%d_%s", j + 1, flv.c_str()), Form("btag_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
               h1_[Form("btag_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) btag discriminator", j + 1, flv.c_str()));
               if (config_->btagAlgorithm() == "deepcsv") {
                  h1_[Form("btag_light_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d_%s", j + 1, flv.c_str()), Form("btag_light_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_c_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d_%s", j + 1, flv.c_str()), Form("btag_c_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_b_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d_%s", j + 1, flv.c_str()), Form("btag_b_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_bb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d_%s", j + 1, flv.c_str()), Form("btag_bb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_cc_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_cc_jet%d_%s", j + 1, flv.c_str()), Form("btag_cc_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
               }
               if (config_->btagAlgorithm() == "deepflavour" || config_->btagAlgorithm() == "deepjet") {
                  h1_[Form("btag_light_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_light_jet%d_%s", j + 1, flv.c_str()), Form("btag_light_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_g_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_g_jet%d_%s", j + 1, flv.c_str()), Form("btag_g_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_c_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_c_jet%d_%s", j + 1, flv.c_str()), Form("btag_c_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_b_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_b_jet%d_%s", j + 1, flv.c_str()), Form("btag_b_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_bb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_bb_jet%d_%s", j + 1, flv.c_str()), Form("btag_bb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
                  h1_[Form("btag_lepb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH1F>(Form("btag_lepb_jet%d_%s", j + 1, flv.c_str()), Form("btag_lepb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), nbins_btag, &bins_btag[0]);
               }
            }

            // 2D histograms
            h2_[Form("pt_eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())] = std::make_shared<TH2F>(Form("pt_eta_jet%d_%s", j + 1, flv.c_str()), Form("pt_eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str()), 1500, 0, 1500, 600, -3, 3);
            h2_[Form("pt_eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) p_{T} [GeV]", j + 1, flv.c_str()));
            h2_[Form("pt_eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->GetYaxis()->SetTitle(Form("Jet %d (%s)  #eta", j + 1, flv.c_str()));
         }
      }
      if (config_->doDijet()) {// dijet histograms
         for (int k = j + 1; k < n && j < n; ++k) {
            h1_[Form("dptrel_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("dptrel_jet%d%d", j + 1, k + 1), Form("dptrel_jet%d%d_%s", j + 1, k + 1, label.c_str()), 1000, 0, 1);
            h1_[Form("dpt_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("dpt_jet%d%d", j + 1, k + 1), Form("dpt_jet%d%d_%s", j + 1, k + 1, label.c_str()), 1000, 0, 1000);
            h1_[Form("dr_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("dr_jet%d%d", j + 1, k + 1), Form("dr_jet%d%d_%s", j + 1, k + 1, label.c_str()), 100, 0, 5);
            h1_[Form("deta_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("deta_jet%d%d", j + 1, k + 1), Form("deta_jet%d%d_%s", j + 1, k + 1, label.c_str()), 100, 0, 10);
            h1_[Form("dphi_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("dphi_jet%d%d", j + 1, k + 1), Form("dphi_jet%d%d_%s", j + 1, k + 1, label.c_str()), 315, 0, 3.15);
            h1_[Form("pt_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d%d", j + 1, k + 1), Form("pt_jet%d%d_%s", j + 1, k + 1, label.c_str()), 300, 0, 3000);
            h1_[Form("eta_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("eta_jet%d%d", j + 1, k + 1), Form("eta_jet%d%d_%s", j + 1, k + 1, label.c_str()), 200, -10, 10);
            h1_[Form("phi_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("phi_jet%d%d", j + 1, k + 1), Form("phi_jet%d%d_%s", j + 1, k + 1, label.c_str()), 360, -180, 180);
            h1_[Form("m_jet%d%d_%s", j + 1, k + 1, label.c_str())] = std::make_shared<TH1F>(Form("m_jet%d%d", j + 1, k + 1), Form("m_jet%d%d_%s", j + 1, k + 1, label.c_str()), 3000, 0, 3000);

            h1_[Form("dptrel_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("#DeltaP_{T}(Jet %d, Jet %d)/Jet %d p_{T}", j + 1, k + 1, j + 1));
            h1_[Form("dpt_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("#Delta p_{T}(Jet %d, Jet %d) [GeV]", j + 1, k + 1));
            h1_[Form("dr_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("#DeltaR(Jet %d, Jet %d)", j + 1, k + 1));
            h1_[Form("deta_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("#Delta#eta(Jet %d, Jet %d)", j + 1, k + 1));
            h1_[Form("dphi_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("#Delta#phi(Jet %d, Jet %d)", j + 1, k + 1));
            h1_[Form("pt_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d + Jet %d p_{T} [GeV]", j + 1, k + 1));
            h1_[Form("eta_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d + Jet %d  #eta", j + 1, k + 1));
            h1_[Form("phi_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("Jet %d + Jet %d  #phi", j + 1, k + 1));
            h1_[Form("m_jet%d%d_%s", j + 1, k + 1, label.c_str())]->GetXaxis()->SetTitle(Form("M_{%d%d} [GeV]", j + 1, k + 1));

            if (config_->isMC() && config_->histogramJetsPerFlavour()) {
               for (auto &flv1 : flavours_) {
                  for (auto &flv2 : flavours_) {
                     h1_[Form("dptrel_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("dptrel_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("dptrel_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 100, 0, 1);
                     h1_[Form("dpt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("dpt_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("dpt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 100, 0, 1000);
                     h1_[Form("dr_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("dr_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("dr_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 100, 0, 5);
                     h1_[Form("deta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("deta_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("deta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 100, 0, 10);
                     h1_[Form("dphi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("dphi_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("dphi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 315, 0, 3.15);
                     h1_[Form("pt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("pt_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("pt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 300, 0, 300);
                     h1_[Form("eta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("eta_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("eta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 200, -10, 10);
                     h1_[Form("phi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("phi_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("phi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 360, -180, 180);
                     h1_[Form("m_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())] = std::make_shared<TH1F>(Form("m_jet%d%d_%s_%s", j + 1, k + 1, flv1.c_str(), flv2.c_str()), Form("m_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str()), 3000, 0, 300);

                     h1_[Form("dptrel_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("#DeltaP_{T}(Jet %d (%s), Jet %d (%s))/Jet %d p_{T}", j + 1, flv1.c_str(), k + 1, flv2.c_str(), j + 1));
                     h1_[Form("dpt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("#Delta p_{T}(Jet %d (%s), Jet %d (%s)) [GeV]", j + 1, flv1.c_str(), k + 1, flv2.c_str()));
                     h1_[Form("dr_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("#DeltaR(Jet %d (%s), Jet %d (%s))", j + 1, flv1.c_str(), k + 1, flv2.c_str()));
                     h1_[Form("deta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("#Delta#eta(Jet %d (%s), Jet %d (%s))", j + 1, flv1.c_str(), k + 1, flv2.c_str()));
                     h1_[Form("dphi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("#Delta#phi(Jet %d (%s), Jet %d (%s))", j + 1, flv1.c_str(), k + 1, flv2.c_str()));
                     h1_[Form("pt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) + Jet %d (%s) p_{T} [GeV]", j + 1, flv1.c_str(), k + 1, flv2.c_str()));
                     h1_[Form("eta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) + Jet %d (%s)  #eta", j + 1, flv1.c_str(), k + 1, flv2.c_str()));
                     h1_[Form("phi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("Jet %d (%s) + Jet %d (%s)  #phi", j + 1, flv1.c_str(), k + 1, flv2.c_str()));
                     h1_[Form("m_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->GetXaxis()->SetTitle(Form("M_{%d%d} (%s)(%s) [GeV]", j + 1, k + 1, flv1.c_str(), flv2.c_str()));
                  }
               }
            }
         }
      }
      if (config_->jetsWithMuons()) {
         h1_[Form("muon_pt_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("muon_pt_jet%d", j + 1), Form("muon_pt_jet%d_%s", j + 1, label.c_str()), 1500, 0, 1500);
         h1_[Form("muon_eta_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("muon_eta_jet%d", j + 1), Form("muon_eta_jet%d_%s", j + 1, label.c_str()), 600, -3, 3);
         h1_[Form("muon_phi_jet%d_%s", j + 1, label.c_str())] = std::make_shared<TH1F>(Form("muon_phi_jet%d", j + 1), Form("muon_phi_jet%d_%s", j + 1, label.c_str()), 360, -180, 180);
      }
   }
   output_rootfile_->cd();
}

float JetAnalyser::btag(const Jet &jet, const std::string &algo) { // TODO: verify this function, where, why algo is passed but not used
   float btag;
   if (config_->btagAlgorithm() == "csvivf" || config_->btagAlgorithm() == "csv") {
      btag = jet.btag("btag_csvivf");
   }
   else if (config_->btagAlgorithm() == "deepcsv") {
      btag = jet.btag("btag_deepb") + jet.btag("btag_deepbb");
   }
   else if (config_->btagAlgorithm() == "deepbvsall") {
      btag = jet.btag("btag_deepbvsall");
   }
   else if (config_->btagAlgorithm() == "deepflavour" || config_->btagAlgorithm() == "deepjet") {
      btag = jet.btag("btag_dfb") + jet.btag("btag_dfbb") + jet.btag("btag_dflepb");
   } else {
      btag = -9999;
   }
   return btag;
}

bool JetAnalyser::selectionJet(const int &rank) {
   if (rank > config_->nJetsMin())
      return true;
   int j = rank - 1;
   float pt_min = config_->jetsPtMin()[j];
   float pt_max = -1.;
   float eta_max = config_->jetsEtaMax()[j];
   if (config_->jetsPtMax().size() > 0 && config_->jetsPtMax()[j] > config_->jetsPtMin()[j])
      pt_max = config_->jetsPtMax()[j];
   return this->selectionJet(rank, pt_min, eta_max, pt_max);
}

bool JetAnalyser::selectionJet(const int &rank, const float &pt_min, const float &eta_max, const float &pt_max) {
   if (rank > config_->nJetsMin())
      return true;
   if ((int)selected_jets_.size() < rank)
      return false; // is this correct?
   bool isgood = true;
   std::string label = Form("Jet %d: pt > %5.1f GeV and |eta| < %3.1f", rank, pt_min, eta_max);
   if (pt_max > pt_min)
      label = Form("Jet %d: pt > %5.1f GeV and pt < %5.1f GeV and |eta| < %3.1f", rank, pt_min, pt_max, eta_max);
   int j = rank - 1;
   // kinematic selection
   if (selected_jets_[j]->pt() < pt_min && !(pt_min < 0))
      isgood = false;
   if (fabs(selected_jets_[j]->eta()) > eta_max && !(eta_max < 0))
      isgood = false;
   if (config_->jetsPtMax().size() > 0) {
      if (selected_jets_[j]->pt() > pt_max && !(pt_max < pt_min))
         isgood = false;
   }
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetPt(const int &rank) {
   if (rank > config_->nJetsMin())
      return true;
   int j = rank - 1;
   float pt_min = config_->jetsPtMin()[j];
   float pt_max = -1.;
   if (config_->jetsPtMax().size() > 0 && config_->jetsPtMax()[j] > config_->jetsPtMin()[j])
      pt_max = config_->jetsPtMax()[j];
   return this->selectionJetPt(rank, pt_min, pt_max);
}

bool JetAnalyser::selectionJetPt(const int &rank, const float &pt_min, const float &pt_max) {
   if (rank > config_->nJetsMin())
      return true;
   if ((int)selected_jets_.size() < rank)
      return false; // is this correct?
   bool isgood = true;
   std::string label = Form("Jet %d: pt > %5.1f GeV", rank, pt_min);
   if (pt_max > pt_min)
      label = Form("Jet %d: pt > %5.1f GeV and pt < %5.1f GeV", rank, pt_min, pt_max);
   int j = rank - 1;
   // kinematic selection
   if (selected_jets_[j]->pt() < pt_min && !(pt_min < 0))
      isgood = false;
   if (config_->jetsPtMax().size() > 0) {
      if (selected_jets_[j]->pt() > pt_max && !(pt_max < pt_min))
         isgood = false;
   }
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetEta(const int &rank) {
   if (rank > config_->nJetsMin())
      return true;
   int j = rank - 1;
   float eta_max = config_->jetsEtaMax()[j];
   return this->selectionJetEta(rank, eta_max);
}

bool JetAnalyser::selectionJetEta(const int &rank, const float &eta_max) {
   bool isgood = true;
   if (rank > config_->nJetsMin())
      return isgood;
   if ((int)selected_jets_.size() < rank)
      return isgood; // is this correct?
   std::string label = Form("Jet %d: |eta| < %3.1f", rank, eta_max);
   int j = rank - 1;
   // kinematic selection
   if (fabs(selected_jets_[j]->eta()) > eta_max && !(eta_max < 0))
      isgood = false;
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetDeta(const int &rank1, const int &rank2, const float &delta) {
   bool isgood = true;
   if (rank1 > config_->nJetsMin() || rank2 > config_->nJetsMin() || delta == 0)
      return isgood;
   std::string label = Form("Deta(jet %d, jet %d) < %4.2f", rank1, rank2, fabs(delta));
   if (delta < 0)
      label = Form("Deta(jet %d, jet %d) > %4.2f", rank1, rank2, fabs(delta));
   int j1 = rank1 - 1;
   int j2 = rank2 - 1;
   if (delta > 0)
      isgood = (fabs(selected_jets_[j1]->eta() - selected_jets_[j2]->eta()) < fabs(delta));
   else
      isgood = (fabs(selected_jets_[j1]->eta() - selected_jets_[j2]->eta()) > fabs(delta));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetDeta(const int &rank1, const int &rank2) {
   bool ok = true;
   if (config_->jetsDetaMax() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetDeta(rank1, rank2, config_->jetsDetaMax());
   }
   if (config_->jetsDetaMin() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetDeta(rank1, rank2, -1 * config_->jetsDetaMin());
   }
   return ok;
}

bool JetAnalyser::selectionJetDphi(const int &rank1, const int &rank2, const float &delta) {
   if (rank1 > config_->nJetsMin() || rank2 > config_->nJetsMin() || delta == 0)
      return true;
   bool isgood = true;
   std::string label = Form("Dphi(jet %d, jet %d) < %4.2f", rank1, rank2, fabs(delta));
   if (delta < 0)
      label = Form("Dphi(jet %d, jet %d) > %4.2f", rank1, rank2, fabs(delta));
   int j1 = rank1 - 1;
   int j2 = rank2 - 1;
   if (delta > 0)
      isgood = (fabs(selected_jets_[j1]->deltaPhi(*selected_jets_[j2])) < fabs(delta));
   else
      isgood = (fabs(selected_jets_[j1]->deltaPhi(*selected_jets_[j2])) > fabs(delta));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetDphi(const int &rank1, const int &rank2) {
   bool ok = true;
   if (config_->jetsDphiMax() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetDphi(rank1, rank2, config_->jetsDphiMax());
   }
   if (config_->jetsDphiMin() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetDphi(rank1, rank2, -1 * config_->jetsDphiMin());
   }
   return ok;
}

bool JetAnalyser::selectionJetDr(const int &rank1, const int &rank2, const float &delta) {
   if (rank1 > config_->nJetsMin() || rank2 > config_->nJetsMin() || delta == 0)
      return true;
   bool isgood = true;
   std::string label = Form("DR(jet %d, jet %d) < %4.2f", rank1, rank2, fabs(delta));
   if (delta < 0)
      label = Form("DR(jet %d, jet %d) > %4.2f", rank1, rank2, fabs(delta));
   int j1 = rank1 - 1;
   int j2 = rank2 - 1;
   if (delta > 0)
      isgood = (selected_jets_[j1]->deltaR(*selected_jets_[j2]) < fabs(delta));
   else
      isgood = (selected_jets_[j1]->deltaR(*selected_jets_[j2]) > fabs(delta));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetDr(const int &rank1, const int &rank2) {
   bool ok = true;
   if (config_->jetsDrMax() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetDr(rank1, rank2, config_->jetsDrMax());
   }
   if (config_->jetsDrMin() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetDr(rank1, rank2, -1 * config_->jetsDrMin());
   }
   return ok;
}

bool JetAnalyser::selectionJetPtImbalance(const int &rank1, const int &rank2, const float &delta) {
   if (!jets_analysis_)
      return true;
   if (rank1 > config_->nJetsMin() || rank2 > config_->nJetsMin() || delta == 0)
      return true;
   bool isgood = true;
   std::string label = Form("DpT(jet %d, jet %d)/jet %d pT < %4.2f", rank1, rank2, rank1, fabs(delta));
   if (delta < 0)
      label = Form("DpT(jet %d, jet %d)/jet %d pT > %4.2f", rank1, rank2, rank1, fabs(delta));
   int j1 = rank1 - 1;
   int j2 = rank2 - 1;
   if (delta > 0)
      isgood = (fabs(selected_jets_[j1]->pt() - selected_jets_[j2]->pt()) / selected_jets_[j1]->pt() < fabs(delta));
   else
      isgood = (fabs(selected_jets_[j1]->pt() - selected_jets_[j2]->pt()) / selected_jets_[j1]->pt() > fabs(delta));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetPtImbalance(const int &rank1, const int &rank2) {
   bool ok = true;
   if (config_->jetsPtImbalanceMax() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetPtImbalance(rank1, rank2, config_->jetsPtImbalanceMax());
   }
   if (config_->jetsPtImbalanceMin() < 0) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetPtImbalance(rank1, rank2, -1 * config_->jetsPtImbalanceMin());
   }
   return ok;
}

bool JetAnalyser::selectionJetId() {
   if (!jets_analysis_)
      return true;
   bool isgood = true;
   std::string label = Form("JetId: %s", config_->jetsId().c_str());
   auto jet = std::begin(selected_jets_);
   while (jet != std::end(selected_jets_)) {
      if (!(*jet)->id(config_->jetsId()))
         jet = selected_jets_.erase(jet);
      else
         ++jet;
   }
   isgood = (selected_jets_.size() > 0);
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetPileupId() {
   if (!jets_analysis_)
      return true;
   bool isgood = true;
   std::string label = Form("JetPileupId: %s, jet pT < %.1f GeV", config_->jetsPuId().c_str(),  config_->jetsPtMaxPuId());
   auto jet = std::begin(selected_jets_);
   while (jet != std::end(selected_jets_))   {
      if (!(*jet)->pileupJetIdFullId(config_->jetsPuId(), config_->jetsPtMaxPuId()))
         jet = selected_jets_.erase(jet);
      else
         ++jet;
   }
   isgood = (selected_jets_.size() > 0);
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionNJets() {
   if (config_->nJetsMin() < 0)
      return true;
   bool isgood = true;
   std::string label;
   if (config_->nJetsMax() <= 0) {
      label = Form("NJets >= %d", config_->nJetsMin());
      isgood = ((int)selected_jets_.size() >= config_->nJetsMin());
   } else if (config_->nJets() >= 0) {
      label = Form("NJets = %d", config_->nJets());
      isgood = ((int)selected_jets_.size() == config_->nJets());
   } else {
      if (config_->nJetsMin() == 0) {
         label = Form("NJets <= %d", config_->nJetsMax());
         isgood = ((int)selected_jets_.size() <= config_->nJetsMax());
      } else {
         label = Form("%d <= NJets <= %d", config_->nJetsMin(), config_->nJetsMax());
         isgood = ((int)selected_jets_.size() >= config_->nJetsMin() && (int)selected_jets_.size() <= config_->nJetsMax());
      }
   }
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionBJet(const int &rank) {
   if ( config_->nJetsMin() < config_->nBJetsMin() || config_->nBJetsMin() < 1 || rank > config_->nBJetsMin() ||  (int)(config_->jetsBtagWP()).size() < config_->nBJetsMin() )
      return true;
   if ( ! config_->signalRegion() && ! config_->validationRegion() && rank == config_->revBtagJet() )
      return this->selectionNonBJet(rank);
   if ( ! config_->signalRegion() && config_->validationRegion() && rank == config_->revBtagJet() )
      return this->selectionSemiBJet(rank);
   if (config_->signalRegion() && config_->validationRegion())
   std::cout<<std::endl<<"WARNING, selected signalRegion == TRUE and validationRegion == TRUE. Running on Signal Region"<<std::endl;
   int j = rank-1;
   if ( config_->btagWP(config_->jetsBtagWP()[j]) < 0 ) return true; // there is no selection here, so will not update the cutflow
   bool isgood = true;
   std::string label = Form("Jet %d: %s btag > %6.4f (%s)",rank,config_->btagAlgorithm().c_str(),config_->btagWP(config_->jetsBtagWP()[j]),config_->jetsBtagWP()[j].c_str());
   isgood = ( btag(*selected_jets_[j],config_->btagAlgorithm()) > config_->btagWP(config_->jetsBtagWP()[j]) );
   cutflow(label,isgood);
   return isgood;
}

bool JetAnalyser::selectionNonBJet(const int &rank) {
   if (config_->btagWP(config_->revBtagWP()) < 0)
      return true; // there is no selection here, so will not update the cutflow
   bool isgood = true;
   std::string label = Form("Jet %d: %s btag < %6.4f (%s) [reverse btag]", rank, config_->btagAlgorithm().c_str(), config_->btagWP(config_->revBtagWP()), config_->revBtagWP().c_str());
   int j = rank - 1;
   // jet  non btag
   isgood = (btag(*selected_jets_[j], config_->btagAlgorithm()) < config_->btagWP(config_->revBtagWP()));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionSemiBJet(const int & rank ) {
   if ( config_->btagWP(config_->revBtagWP()) < 0 ) return true; // there is no selection here, so will not update the cutflow
   bool isgood = true;
   int j = rank-1;
   std::string label = Form("Jet %d: %s %6.4f (%s) < btag < %6.4f (%s) [semi btag]",rank,config_->btagAlgorithm().c_str(),config_->btagWP(config_->revBtagWP()),config_->revBtagWP().c_str(),config_->btagWP(config_->jetsBtagWP()[j]),config_->jetsBtagWP()[j].c_str());
   // jet  non btag
   isgood = ( btag(*selected_jets_[j],config_->btagAlgorithm()) > config_->btagWP(config_->revBtagWP()) &&  btag(*selected_jets_[j],config_->btagAlgorithm()) < config_->btagWP(config_->jetsBtagWP()[j]) );
   cutflow(label,isgood);
   return isgood;
}

bool JetAnalyser::onlineJetMatching(const int &rank) {
   if (config_->triggerObjectsL1Jets() == "" && config_->triggerObjectsCaloJets() == "" && config_->triggerObjectsPFJets() == "")
      return true;
   if (config_->nJetsMin() < 0)
      return true;
   bool isgood = true;
   std::string label = Form("Jet %d: online jet match (deltaR: L1 < %4.3f, Calo < %4.3f, PF < %4.3f)", rank, config_->triggerMatchL1JetsDrMax(), config_->triggerMatchCaloJetsDrMax(), config_->triggerMatchPFJetsDrMax());
   int j = rank - 1;
   std::string triggerObjectsL1Jets = config_->triggerObjectsL1Jets();
   if (config_->triggerEmulateL1Jets() != "" && config_->triggerEmulateL1JetsNMin() > 0)
      triggerObjectsL1Jets = config_->triggerEmulateL1Jets();
   std::string triggerObjectsCaloJets = config_->triggerObjectsCaloJets();
   if (config_->triggerEmulateCaloJets() != "" && config_->triggerEmulateCaloJetsNMin() > 0)
      triggerObjectsCaloJets = config_->triggerEmulateCaloJets();
   std::string triggerObjectsPFJets = config_->triggerObjectsPFJets();
   if (config_->triggerEmulatePFJets() != "" && config_->triggerEmulatePFJetsNMin() > 0)
      triggerObjectsPFJets = config_->triggerEmulatePFJets();
   std::shared_ptr<Jet> jet = selected_jets_[j];
   isgood = (jet->matched(triggerObjectsL1Jets));
   isgood = (isgood && jet->matched(triggerObjectsCaloJets));
   isgood = (isgood && jet->matched(triggerObjectsPFJets));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::onlineBJetMatching(const int &rank) {
   if (config_->triggerObjectsBJets() == "")
      return true;
   bool isgood = true;
   std::string label = Form("Jet %d: online b jet match (deltaR < %4.3f)", rank, config_->triggerMatchCaloBJetsDrMax());
   int j = rank - 1;
   std::shared_ptr<Jet> jet = selected_jets_[j];
   isgood = (jet->matched(config_->triggerObjectsBJets()));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::onlineBJetMatching(const std::vector<int> & ranks, const int &nmin) {
   if (config_->triggerObjectsBJets() == "")
      return true;
   std::string rank_list = "";
   for ( auto & rank : ranks )
      rank_list += std::to_string(rank)+", ";
   rank_list.pop_back();
   rank_list.pop_back();
   std::string label = Form("Online b jet match (deltaR < %4.3f), at least %d of jets %s", config_->triggerMatchCaloBJetsDrMax(), nmin,rank_list.c_str());
   auto matched_ranks = this->onlineBJetMatchedJets(ranks);
   bool isgood = (int(matched_ranks.size()) >= nmin);
   cutflow(label, isgood);
   return isgood;
}

std::vector<int> JetAnalyser::onlineBJetMatchedJets(const std::vector<int> & ranks) {
   std::vector<int> matched_ranks;
   if (config_->triggerObjectsBJets() == "")
      return ranks;
   for ( auto & rank : ranks ) {
      int j = rank - 1;
      std::shared_ptr<Jet> jet = selected_jets_[j];
      if ( jet->matched(config_->triggerObjectsBJets()) )
         matched_ranks.push_back(rank);
   }
   return matched_ranks;
}

void JetAnalyser::fillJetHistograms(const std::string &label){
   if (label == "")
      return;
   output_rootfile_->cd();
   output_rootfile_->cd(label.c_str());
   int n = num_histograms_jets_;
   if (n > config_->nJetsMin())
      n = config_->nJetsMin();
   this->fill1DHistogram(label,"jet_hist_weight",0., weight_);
   output_rootfile_->cd(label.c_str()); // TODO need to find a way to remove this line
   for (int j = 0; j < n; ++j) {
      // fill each jet with addtional SF to weight = 1
      this->fillJetHistograms(j + 1, label, 1.);
      if (config_->doDijet()) {
         for (int k = j + 1; k < n && j < n; ++k) {
            Composite<Jet, Jet> c_ij(*(selected_jets_[j]), *(selected_jets_[k]));
            h1_[Form("dptrel_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(fabs(selected_jets_[j]->pt() - selected_jets_[k]->pt()) / selected_jets_[j]->pt(), weight_);
            h1_[Form("dpt_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(fabs(selected_jets_[j]->pt() - selected_jets_[k]->pt()), weight_);
            h1_[Form("dr_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(c_ij.deltaR(), weight_);
            h1_[Form("deta_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(c_ij.deltaEta(), weight_);
            h1_[Form("dphi_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(fabs(selected_jets_[j]->deltaPhi(*selected_jets_[k])));
            h1_[Form("pt_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(c_ij.pt(), weight_);
            h1_[Form("eta_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(c_ij.eta(), weight_);
            h1_[Form("phi_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(c_ij.phi() * 180. / acos(-1.), weight_);
            if (config_->isMC() || !config_->signalRegion() ||  !config_->blind() ) {
               h1_[Form("m_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(c_ij.m(), weight_);
            } else { // blind
               h1_[Form("m_jet%d%d_%s", j + 1, k + 1, label.c_str())]->Fill(0., weight_);
            }
            if (config_->isMC() && config_->histogramJetsPerFlavour()) {
               std::string flv1 = "udsg";
               std::string flv2 = "udsg";
               if (!config_->useJetsExtendedFlavour()) {
                  if (abs(selected_jets_[j]->flavour()) == 4)
                     flv1 = "c";
                  if (abs(selected_jets_[j]->flavour()) == 5)
                     flv1 = "b";
                  if (abs(selected_jets_[k]->flavour()) == 4)
                     flv2 = "c";
                  if (abs(selected_jets_[k]->flavour()) == 5)
                     flv2 = "b";
               } else {
                  flv1 = selected_jets_[j]->extendedFlavour();
                  flv2 = selected_jets_[k]->extendedFlavour();
               }
               h1_[Form("dptrel_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(fabs(selected_jets_[j]->pt() - selected_jets_[k]->pt()) / selected_jets_[j]->pt(), weight_);
               h1_[Form("dpt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(fabs(selected_jets_[j]->pt() - selected_jets_[k]->pt()), weight_);
               h1_[Form("dr_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(c_ij.deltaR(), weight_);
               h1_[Form("deta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(c_ij.deltaEta(), weight_);
               h1_[Form("dphi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(fabs(selected_jets_[j]->deltaPhi(*selected_jets_[k])));
               h1_[Form("pt_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(c_ij.pt(), weight_);
               h1_[Form("eta_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(c_ij.eta(), weight_);
               h1_[Form("phi_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(c_ij.phi() * 180. / acos(-1.), weight_);
               h1_[Form("m_jet%d%d_%s_%s_%s", j + 1, k + 1, label.c_str(), flv1.c_str(), flv2.c_str())]->Fill(c_ij.m(), weight_);
            }
         }
      }
   }
   output_rootfile_->cd();
   cutflow(Form("*** Filling jets histograms - %s", label.c_str()));
}

void JetAnalyser::fillJetHistograms(const int &rank, const std::string &label, const float &scale_factor, const bool &workflow) {
   if (rank < 1 || rank > num_histograms_jets_)
      return;
   if (workflow) {// BE CAREFUL with this
      output_rootfile_->cd();
      ++cutflow_;
      if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "")
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("*** Filling jet # %d histograms - %s", rank, label.c_str()));
      output_rootfile_->cd(label.c_str());
   }
   int j = rank - 1;
   float histogram_weight = weight_ * scale_factor;
   // 1D histograms
   h1_[Form("pt_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
   // barrel and endcap pt distributions
   float eta = selected_jets_[j]->eta();
   if (config_->histogramJetsRegionSplit()) {
      if (eta < -1.4)
         h1_[Form("pt_jet%d_me_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
      if (eta > 1.4)
         h1_[Form("pt_jet%d_pe_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
      if (eta < 0.0 && eta > -1.0)
         h1_[Form("pt_jet%d_mb_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
      if (eta > 0.0 && eta < 1.0)
         h1_[Form("pt_jet%d_pb_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
      if (eta < -1.0 && eta > -1.4)
         h1_[Form("pt_jet%d_mbe_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
      if (eta > 1.0 && eta < 1.4)
         h1_[Form("pt_jet%d_pbe_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
   }
   //
   h1_[Form("eta_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->eta(), histogram_weight);
   h1_[Form("phi_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->phi() * 180. / acos(-1.), histogram_weight);
   h1_[Form("qglikelihood_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->qgLikelihood(), histogram_weight);
   h1_[Form("nconstituents_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->constituents(), histogram_weight);
   //
   if (config_->isMC()) {
      h1_[Form("flavour_jet%d_%s", j + 1, label.c_str())]->Fill(abs(selected_jets_[j]->flavour()), weight_);
      if (config_->useJetsExtendedFlavour()) {
         std::string flv1 = selected_jets_[j]->extendedFlavour();
         int nflv1 = 0;
         if (flv1 == "c")
            nflv1 = 4;
         if (flv1 == "b")
            nflv1 = 5;
         if (flv1 == "cc")
            nflv1 = 14;
         if (flv1 == "bb")
            nflv1 = 15;
         h1_[Form("extendedflavour_jet%d_%s", j + 1, label.c_str())]->Fill(nflv1, weight_);
      }
   }
   if (config_->btagAlgorithm() != "") {
      float mybtag = btag(*selected_jets_[j], config_->btagAlgorithm());
      float mybtaglog = 1.e-7;
      if (mybtag > 0)
         mybtaglog = -log(1. - mybtag);
      h1_[Form("btag_jet%d_%s", j + 1, label.c_str())]->Fill(mybtag, histogram_weight);
      h1_[Form("btaglog_jet%d_%s", j + 1, label.c_str())]->Fill(mybtaglog, histogram_weight);

      if (config_->btagAlgorithm() == "deepcsv") {
         h1_[Form("btag_light_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_deeplight"), histogram_weight);
         h1_[Form("btag_c_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_deepc"), histogram_weight);
         h1_[Form("btag_b_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_deepb"), histogram_weight);
         h1_[Form("btag_bb_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_deepbb"), histogram_weight);
         h1_[Form("btag_cc_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_deepcc"), histogram_weight);
      }
      if (config_->btagAlgorithm() == "deepflavour" || config_->btagAlgorithm() == "deepjet") {
         h1_[Form("btag_light_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_dflight"), histogram_weight);
         h1_[Form("btag_g_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_dfg"), histogram_weight);
         h1_[Form("btag_c_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_dfc"), histogram_weight);
         h1_[Form("btag_b_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_dfb"), histogram_weight);
         h1_[Form("btag_bb_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_dfbb"), histogram_weight);
         h1_[Form("btag_lepb_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->btag("btag_dflepb"), histogram_weight);
      }
   }
   // 2D histograms
   h2_[Form("pt_eta_jet%d_%s", j + 1, label.c_str())]->Fill(selected_jets_[j]->pt(), selected_jets_[j]->eta(), histogram_weight);
   if (config_->isMC() && config_->histogramJetsPerFlavour()) {
      std::string flv = "udsg";
      if (!config_->useJetsExtendedFlavour()) {
         if (abs(selected_jets_[j]->flavour()) == 4)
            flv = "c";
         if (abs(selected_jets_[j]->flavour()) == 5)
            flv = "b";
      } else {
         flv = selected_jets_[j]->extendedFlavour();
      }
      // 1D histograms
      h1_[Form("pt_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->pt(), histogram_weight);
      h1_[Form("eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->eta(), histogram_weight);
      h1_[Form("phi_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->phi() * 180. / acos(-1.), histogram_weight);
      h1_[Form("btag_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(btag(*selected_jets_[j], config_->btagAlgorithm()), histogram_weight);
      h1_[Form("qglikelihood_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->qgLikelihood(), histogram_weight);
      h1_[Form("nconstituents_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->constituents(), histogram_weight);
      if (config_->btagAlgorithm() == "deepcsv") {
         h1_[Form("btag_light_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_deeplight"), histogram_weight);
         h1_[Form("btag_c_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_deepc"), histogram_weight);
         h1_[Form("btag_b_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_deepb"), histogram_weight);
         h1_[Form("btag_bb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_deepbb"), histogram_weight);
         h1_[Form("btag_cc_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_deepcc"), histogram_weight);
      }
      if (config_->btagAlgorithm() == "deepflavour" || config_->btagAlgorithm() == "deepjet") {
         h1_[Form("btag_light_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_dflight"), histogram_weight);
         h1_[Form("btag_g_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_dfg"), histogram_weight);
         h1_[Form("btag_c_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_dfc"), histogram_weight);
         h1_[Form("btag_b_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_dfb"), histogram_weight);
         h1_[Form("btag_bb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_dfbb"), histogram_weight);
         h1_[Form("btag_lepb_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->btag("btag_dflepb"), histogram_weight);
      }
      // 2D histograms
      h2_[Form("pt_eta_jet%d_%s_%s", j + 1, label.c_str(), flv.c_str())]->Fill(selected_jets_[j]->pt(), selected_jets_[j]->eta(), histogram_weight);
   }
   if (config_->jetsWithMuons()) {
      auto jetmu = selected_jets_[j]->muon();
      float mpt = -1.;
      float meta = -10;
      float mphi = -200;
      if (selected_jets_[j]->muon()) {
         mpt = jetmu->pt();
         meta = jetmu->eta();
         mphi = jetmu->phi();
      }
      h1_[Form("muon_pt_jet%d_%s", j + 1, label.c_str())]->Fill(mpt);
      h1_[Form("muon_eta_jet%d_%s", j + 1, label.c_str())]->Fill(meta);
      h1_[Form("muon_phi_jet%d_%s", j + 1, label.c_str())]->Fill(mphi);
   }
   output_rootfile_->cd();
   if (workflow)
      h1_["cutflow"]->Fill(cutflow_, histogram_weight);
}

void JetAnalyser::actionApplyJER() {
   if (!jets_analysis_ || ! is_mc_)
      return;
   std::string label = "WARNING: NO JER smearing (*** missing JER Info and/or GenJet collection ***)";
   if (apply_jet_resolution_) {
      std::string bnpt = basename(config_->jerPtRes());
      std::string bnsf = basename(config_->jerSF());
      label = Form("JER smearing (%s,%s)", bnpt.c_str(), bnsf.c_str());
      if (config_->jerSystematics() != 0)
         label = Form("JER smearing (%s,%s), syst: %+d sig", bnpt.c_str(), bnsf.c_str(), config_->jerSystematics());
      for (auto &j : selected_jets_)
         j->applyJER(*jet_resolution_info_, 0.2, config_->jerSystematics());
   }
   cutflow(label);
}

void JetAnalyser::actionApplyJEC() {
   if (!jets_analysis_ || !apply_jet_corrections_)
      return;
   std::string bnpt = basename(config_->jerPtRes());
   std::string bnsf = basename(config_->jerSF());
   std::string label = Form("JEC systematics: %+d sig", config_->jecSystematics());
   for (auto &j : selected_jets_)
      j->applyJEC(config_->jecSystematics());
   cutflow(label);
}

ScaleFactors JetAnalyser::btagSF(const int &rank, const std::string &working_point, const std::string &systematics_type) {
   ScaleFactors scale_factor;
   int j = rank - 1;
   scale_factor.nominal = selected_jets_[j]->btagSF(btag_scale_factor_reader_[working_point]);
   scale_factor.up = selected_jets_[j]->btagSFup(btag_scale_factor_reader_[working_point],systematics_type);
   scale_factor.down = selected_jets_[j]->btagSFdown(btag_scale_factor_reader_[working_point],systematics_type);
   return scale_factor;
}

float JetAnalyser::actionApplyBtagSF(const int &rank, const bool &global_weight) {
   float scale_factor = 1.;
   if (rank > config_->nBJetsMin())
      return scale_factor;
   if (!config_->isMC() || config_->btagScaleFactors() == "")
      return scale_factor; // will not apply btag SF
   if (!config_->signalRegion() && rank == config_->revBtagJet()) // no scale factors for the reverse btag
      return scale_factor;
   auto syst = config_->btagSystematics();
   auto systype = config_->btagSystematicsType();
   int j = rank - 1;
   std::string label = Form("Jet %d: btag SF applied (%s %s WP)", rank, config_->btagAlgorithm().c_str(), config_->jetsBtagWP()[j].c_str());
   if ( syst != 0 )
      label = Form("Jet %d: btag SF syst (%s, %+d sigma) applied (%s %s WP)", rank, systype.c_str(), syst, config_->btagAlgorithm().c_str(), config_->jetsBtagWP()[j].c_str());
   if (config_->jetsBtagWP()[j] == "xxx")
      label = Form("Jet %d: btag SF = 1 applied (%s %s WP)", rank, config_->btagAlgorithm().c_str(), config_->jetsBtagWP()[j].c_str());
   if (global_weight || config_->jetsBtagWP()[j] != "xxx") {
      float sf_nominal = this->btagSF(rank, config_->jetsBtagWP()[j],systype).nominal;
      float uncert_up = fabs(this->btagSF(rank, config_->jetsBtagWP()[j],systype).up - sf_nominal);
      float uncert_down = fabs(sf_nominal - this->btagSF(rank, config_->jetsBtagWP()[j],systype).down);
      float uncert_factor = (syst == 0) ? 0 : ((syst > 0) ? uncert_up : uncert_down);
      scale_factor = sf_nominal + syst * uncert_factor;
   }
   weight_ *= scale_factor;
   cutflow(label);
   return scale_factor;
}

float JetAnalyser::getBtagSF(const int &rank) {
   float scale_factor = 1.;
   int j = rank - 1;
   if (!config_->isMC() || config_->btagScaleFactors() == "")
      return scale_factor; // will not apply btag SF
   if (!config_->signalRegion() && rank == config_->revBtagJet())
      return scale_factor;
   if (config_->jetsBtagWP()[j] != "xxx")
      scale_factor = this->btagSF(rank, config_->jetsBtagWP()[j],"").nominal;
   return scale_factor;
}

void JetAnalyser::actionApplyBjetRegression() {
   if (!config_->bRegression())
      return;
   for (auto &j : selected_jets_)
      j->applyBjetRegression();
   cutflow("b jet energy regression");
}

void JetAnalyser::actionApplyBjetRegression(const int &rank) {
   if (!config_->bRegression())
      return;
   int j = rank - 1;
   selected_jets_[j]->applyBjetRegression();
   cutflow(Form("Jet %d: b jet energy regression", rank));
}

bool JetAnalyser::selectionDiJetMass(const int &rank1, const int &rank2) {
   float min = config_->massMin();
   float max = config_->massMax();
   if (min < 0. && max < 0.)
      return true;
   ++cutflow_;
   if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "") {
      std::string label = Form("M(Jet %d + Jet %d)", rank1, rank2);
      if (min > 0.) {
         if (max > 0. && max > min)
            label = Form("%5.1f GeV < %s < %5.1f GeV", min, label.c_str(), max);
         else
            label = Form("%s > %5.1f GeV", label.c_str(), min);
      } else {
         label = Form("%s < %5.1f GeV", label.c_str(), max);
      }
      h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, label.c_str());
   }
   int j1 = rank1 - 1;
   int j2 = rank2 - 1;
   Composite<Jet, Jet> c_j1j2(*(selected_jets_[j1]), *(selected_jets_[j2]));
   float mass = c_j1j2.m();
   if (min > 0. && mass < min)
      return false;
   if (max > 0. && mass > max)
      return false;
   h1_["cutflow"]->Fill(cutflow_, weight_);
   return true;
}

void JetAnalyser::jetSwap(const int &rank1, const int &rank2) { // TODO: does a cutflow make sense here?
   if (rank1 == rank2)
      return;
   int j1 = rank1 - 1;
   int j2 = rank2 - 1;
   //    ++ cutflow_;
   //    if ( std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_+1)) == "" )
   //       h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_+1,Form("Jet %d <-->Jet %d: jets ranking was swapped ",rank1,rank2));
   auto jet1 = selected_jets_[j1];
   auto jet2 = selected_jets_[j2];
   selected_jets_[j1] = jet2;
   selected_jets_[j2] = jet1;
   //    h1_["cutflow"]->Fill(cutflow_,weight_);
}

bool JetAnalyser::selectionJetQGlikelihood(const int &rank, const float &cut) {
   bool isgood = true;
   std::string label = Form("Jet %d Q-G likelihood < %4.2f", rank, fabs(cut));
   int j = rank - 1;
   if (cut < 0)
      label = Form("Jet %d Q-G likelihood > %4.2f", rank, fabs(cut));
   if (cut > 0)
      isgood = (selected_jets_[j]->qgLikelihood() < fabs(cut));
   else
      isgood = (selected_jets_[j]->qgLikelihood() > fabs(cut));
   cutflow(label, isgood);
   return isgood;
}

bool JetAnalyser::selectionJetQGlikelihood(const int &rank) {
   int j = rank - 1;
   bool ok = true;
   if (config_->jetsQGmax().size() == 0 || config_->jetsQGmax()[j] < 0 || (int)config_->jetsQGmax().size() < rank) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetQGlikelihood(rank, config_->jetsQGmax()[j]);
   }
   if (config_->jetsQGmin().size() == 0 || config_->jetsQGmin()[j] < 0 || (int)config_->jetsQGmin().size() < rank) {
      ok = ok && true;
   } else {
      ok = ok && selectionJetQGlikelihood(rank, -1 * config_->jetsQGmin()[j]);
   }
   return ok;
}

bool JetAnalyser::selectionBJetProbB(const int &rank) {
   if (config_->jetsBtagProbB().size() == 0)
      return true;
   int j = rank - 1;
   float wp = config_->jetsBtagProbB()[j];
   std::string btag_algorithm = config_->btagAlgorithm();
   if (fabs(wp) > 1)
      return true; // there is no selection here, so will not update the cutflow
   ++cutflow_;
   if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "") {
      if (wp > 0)
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob b < %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
      else
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob b > %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
   }
   if (rank > config_->nBJetsMin()) {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbB(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   // jet  btag
   if (btag_algorithm == "deepcsv") {
      if (wp > 0 && selected_jets_[j]->btag("btag_deepb") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_deepb") < fabs(wp))
         return false;
   }
   if (btag_algorithm == "deepflavour" || btag_algorithm == "deepjet") {
      if (wp > 0 && selected_jets_[j]->btag("btag_dfb") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_dfb") < fabs(wp))
         return false;
   }
   h1_["cutflow"]->Fill(cutflow_, weight_);
   return true;
}

bool JetAnalyser::selectionBJetProbBB(const int &rank) {
   if (config_->jetsBtagProbBB().size() == 0)
      return true;
   int j = rank - 1;
   float wp = config_->jetsBtagProbBB()[j];
   std::string btag_algorithm = config_->btagAlgorithm();
   if (fabs(wp) > 1)
      return true; // there is no selection here, so will not update the cutflow
   ++cutflow_;
   if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "") {
      if (wp > 0)
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob bb < %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
      else
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob bb > %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
   }
   if (rank > config_->nBJetsMin()) {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbBB(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   // jet  btag
   if (btag_algorithm == "deepcsv") {
      if (wp > 0 && selected_jets_[j]->btag("btag_deepbb") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_deepbb") < fabs(wp))
         return false;
   }
   if (btag_algorithm == "deepflavour" || btag_algorithm == "deepjet")  {
      if (wp > 0 && selected_jets_[j]->btag("btag_dfbb") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_dfbb") < fabs(wp))
         return false;
   }
   h1_["cutflow"]->Fill(cutflow_, weight_);
   return true;
}

bool JetAnalyser::selectionBJetProbLepB(const int &rank) {
   if (config_->jetsBtagProbLepB().size() == 0)
      return true;
   int j = rank - 1;
   float wp = config_->jetsBtagProbLepB()[j];
   std::string btag_algorithm = config_->btagAlgorithm();
   if (fabs(wp) > 1 || btag_algorithm == "deepcsv")
      return true; // there is no selection here, so will not update the cutflow
   ++cutflow_;
   if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "") {
      if (wp > 0)
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob lepb < %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
      else
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob lepb > %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
   }
   if (rank > config_->nBJetsMin()) {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbLepB(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   // jet  btag
   if (btag_algorithm == "deepflavour" || btag_algorithm == "deepjet") {
      if (wp > 0 && selected_jets_[j]->btag("btag_dflepb") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_dflepb") < fabs(wp))
         return false;
   }
   h1_["cutflow"]->Fill(cutflow_, weight_);
   return true;
}

bool JetAnalyser::selectionBJetProbC(const int &rank) {
   if (config_->jetsBtagProbC().size() == 0)
      return true;
   int j = rank - 1;
   float wp = config_->jetsBtagProbC()[j];
   std::string btag_algorithm = config_->btagAlgorithm();
   if (fabs(wp) > 1)
      return true; // there is no selection here, so will not update the cutflow
   ++cutflow_;
   if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "") {
      if (wp > 0)
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob c < %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
      else
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob c > %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
   }
   if (rank > config_->nBJetsMin())  {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbC(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   // jet  btag
   if (btag_algorithm == "deepcsv") {
      if (wp > 0 && selected_jets_[j]->btag("btag_deepc") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_deepc") < fabs(wp))
         return false;
   }
   if (btag_algorithm == "deepflavour" || btag_algorithm == "deepjet") {
      if (wp > 0 && selected_jets_[j]->btag("btag_dfc") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_dfc") < fabs(wp))
         return false;
   }
   h1_["cutflow"]->Fill(cutflow_, weight_);
   return true;
}

bool JetAnalyser::selectionBJetProbG(const int &rank) {
   if (config_->jetsBtagProbG().size() == 0)
      return true;
   int j = rank - 1;
   float wp = config_->jetsBtagProbG()[j];
   std::string btag_algorithm = config_->btagAlgorithm();
   if (fabs(wp) > 1 || btag_algorithm == "deepcsv")
      return true; // there is no selection here, so will not update the cutflow
   ++cutflow_;
   if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "") {
      if (wp > 0)
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob g < %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
      else
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob g > %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
   }
   if (rank > config_->nBJetsMin()) {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbG(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   // jet  btag
   if (btag_algorithm == "deepflavour" || btag_algorithm == "deepjet") {
      if (wp > 0 && selected_jets_[j]->btag("btag_dfg") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_dfg") < fabs(wp))
         return false;
   }
   h1_["cutflow"]->Fill(cutflow_, weight_);
   return true;
}

bool JetAnalyser::selectionBJetProbLight(const int &rank) {
   if (config_->jetsBtagProbLight().size() == 0)
      return true;
   int j = rank - 1;
   float wp = config_->jetsBtagProbLight()[j];
   if (fabs(wp) > 1)
      return true; // there is no selection here, so will not update the cutflow
   std::string btag_algorithm = config_->btagAlgorithm();
   ++cutflow_;
   if (std::string(h1_["cutflow"]->GetXaxis()->GetBinLabel(cutflow_ + 1)) == "") {
      if (wp > 0)
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob light < %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
      else
         h1_["cutflow"]->GetXaxis()->SetBinLabel(cutflow_ + 1, Form("Jet %d: %s btag prob light > %6.4f", rank, btag_algorithm.c_str(), fabs(wp)));
   }
   if (rank > config_->nBJetsMin()) {
      std::cout << "* warning * -  JetAnalyser::selectionBJetProbLight(): given jet rank > nbjetsmin. Returning false! " << std::endl;
      return false;
   }
   // jet  btag
   if (btag_algorithm == "deepcsv") {
      if (wp > 0 && selected_jets_[j]->btag("btag_deeplight") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_deeplight") < fabs(wp))
         return false;
   }
   if (btag_algorithm == "deepflavour" || btag_algorithm == "deepjet") {
      if (wp > 0 && selected_jets_[j]->btag("btag_dflight") > fabs(wp))
         return false;
      if (wp < 0 && selected_jets_[j]->btag("btag_dflight") < fabs(wp))
         return false;
   }
   h1_["cutflow"]->Fill(cutflow_, weight_);
   return true;
}

bool JetAnalyser::jetCorrections() {
   // CORRECTIONS
   // Jet energy resolution smearing
   this->actionApplyJER();
   // Jet online trigger scale factor
   this->actionApplyJetOnlineSF();   
   // b energy regression
   this->actionApplyBjetRegression();
   return true;
}

void JetAnalyser::actionApplyBtagEfficiency(const int &rank, const int &model) {
   if (rank > config_->nBJetsMin())
      return;
   if (!jets_analysis_ || ! is_mc_)
      return;
   int j = rank - 1;
   auto jet = selected_jets_[j];
   // Use jet 4-momentum after JER to not use b-regression
   if (config_->useJetsExtendedFlavour())
      weight_ *= btag_efficiencies_[model - 1].efficiency(jet->extendedFlavour(), jet->jerP4().Pt(), jet->jerP4().Eta());
   else
      weight_ *= btag_efficiencies_[model - 1].efficiency(jet->flavour(), jet->jerP4().Pt(), jet->jerP4().Eta());
   // cutflow(label);
}


void JetAnalyser::actionApplyJetOnlineSF() {
// Jet Online Corrections to be applied to MC
   if ( ! jets_analysis_ || ! config_->isMC() || selected_jets_.size() < 2) return; //check and print error message
   std::string label = "WARNING: NO Jet Online Scale factor (*** missing Scale Factor Info and/or GenJet collection ***)";
   if ( config_->onlinejetSF() != "") {
      std::string bnojsf = basename(config_->onlinejetSF());
      label = Form("Jet Online Scale Factor (%s)",bnojsf.c_str());
      if ( config_->onlinejetSystematics() != 0 ) {
         if (fabs(config_->onlinejetSystematics()) == 1 || fabs(config_->onlinejetSystematics()) == 2) {
            label = Form("Jet Online Scale Factor: (%s), syst: %+d sig",bnojsf.c_str(),config_->onlinejetSystematics());
         } else {
            std::string label = Form("WARNING: NO Jet Online Scale factor (*** missing Scale Factor Info for syst = %+d sig ***)",config_->onlinejetSystematics());       
            cutflow(label);
            return;
         }  
      }
      this->applyJetOnlineSF(1); // apply online jet kinematic trigger efficiency scale factor on jet i
      cutflow(Form("Jet 1: %s",label.c_str()));
      this->applyJetOnlineSF(2); 
      cutflow(Form("Jet 2: %s",label.c_str()));
      return;
   }
   cutflow(label);
}     


void JetAnalyser::applyJetOnlineSF(const int & rank) {
// Jet Online Corrections to be applied to MC
   if ( ! jets_analysis_ || ! config_->isMC() || selected_jets_.size() < 2) return;
   if ( config_->onlinejetSF() == "")  return;
   int j = rank-1;
   float scale_factor = 1;
   scale_factor *= jet_trigger_efficiency_->findSF(selected_jets_[j]->eta(),selected_jets_[j]->pt(), config_->onlinejetSystematics());
   weight_ *= scale_factor; //apply scale_factor to event weight]
   return;
}


void JetAnalyser::actionApplyJetOnlineSF(const int & rank) {
// Jet Online Corrections to be applied to MC
   if ( ! jets_analysis_ || ! config_->isMC() )
      return;
   int j = rank-1;
   float scale_factor = 1.;
   int systematic = config_->onlinejetSystematics();
   std::string label = "WARNING: NO jet online scale factor (*** assuming SF = 1 ***)";
   // TODO: check label when no file
   if (config_->onlinejetSF() != "") {
      std::string bnsf = basename(config_->onlinejetSF());
      label = Form("Jet %d: online scale factor (%s)", rank, bnsf.c_str()); // assuming central value
      if ( systematic != 0 )
         label = Form("Jet %d: online scale factor syst = %+d sig (%s)", rank, systematic, bnsf.c_str());
      if ( abs(systematic) > 2 ) 
         std::cout << " *** Error ***: there is no systematic variation > 2 sigma!" << std::endl;
      scale_factor = jet_trigger_efficiency_->findSF(selected_jets_[j]->eta(),selected_jets_[j]->pt(), config_->onlinejetSystematics());
   }
   weight_ *= scale_factor; //apply scale_factor to event weight
   cutflow(label);
}

void JetAnalyser::actionApplyBtagOnlineSF(const std::vector<int> & ranks) {
   if ( ! jets_analysis_ || ! config_->isMC() || config_->onlinebtagSF() == "" )
      return; 
   std::string label = "WARNING: NO Btag Online Scale factor (*** missing Scale Factor Info ***)";
   float scale_factor_jet1 = 1.;
   float scale_factor_jet2 = 1.;
   // Btag Online Corrections to be applied to MC
   auto matched_ranks = this->onlineBJetMatchedJets(ranks);
   auto matched_indices = matched_ranks; // access to the selected jets are by indices, for they are vectors.
   for (int &index : matched_indices)
      index -= 1;
   // dealing with label
   std::string label_scale_factor = basename(config_->onlinebtagSF());
   std::string label_scale_factor_muonjet = basename(config_->onlinebtagSF());
   label = Form("Online Btag Scale Factor: %s",label_scale_factor.c_str());
   if ( config_->onlinebtagSystematics() != 0 )
      label = Form("Online Btag Scale Factor (syst: %+d): %s",config_->onlinebtagSystematics(),label_scale_factor.c_str());
   if ( config_->onlinebtagMuonJetSF() != "" )
      label = Form("%s, %s",label.c_str(), label_scale_factor_muonjet.c_str());
   bool muonjet1 = false;
   bool muonjet2 = false;
   if ( !config_->muonsVeto() && is_muons_analysis_ ) { // for semileptonic case
      muonjet1 = (selected_jets_[matched_indices[0]]->muon() != nullptr); // must use indices or some function that takes ranks
      muonjet2 = (selected_jets_[matched_indices[1]]->muon() != nullptr); // must use indices or some function that takes ranks
   }
   scale_factor_jet1 = this->getBtagOnlineSF(matched_ranks[0],muonjet1);
   scale_factor_jet2 = this->getBtagOnlineSF(matched_ranks[1],muonjet2);
   weight_ *= (scale_factor_jet1 * scale_factor_jet2);
   cutflow(label);
}

float JetAnalyser::getBtagOnlineSF(const int & rank,const bool & muonjet) {
// Jet Online Corrections to be applied to MC
   if ( ! jets_analysis_ || ! config_->isMC() || config_->onlinebtagSF() == "" )
      return -1;
   int j = rank-1;
   float scale_factor = btag_trigger_efficiency_->findSF(selected_jets_[j]->pt(), config_->onlinebtagSystematics()); // TODO: verify that it always return a reasonable value
   if ( config_->onlinebtagMuonJetSF() != "" && muonjet ) {
      scale_factor = btag_muonjet_trigger_efficiency_->findSF(selected_jets_[j]->pt(), config_->onlinebtagSystematics());
   }   
   return scale_factor;
}

std::vector< std::shared_ptr<Jet> > JetAnalyser::removeSelectedJets(const std::vector<int> & ranks) {
   std::vector< std::shared_ptr<Jet> > jets;
   for ( size_t i = 0; i < selected_jets_.size(); ++i) {
      int rank = i+1;
      if ( std::find(ranks.begin(), ranks.end(), rank) != ranks.end() ) continue;
      jets.push_back(selected_jets_[i]);
   }
   return jets;
}

std::vector< std::shared_ptr<Jet> > JetAnalyser::keepSelectedJets(const std::vector<int> & ranks) { 
   std::vector< std::shared_ptr<Jet> > jets;
   for ( size_t i = 0; i < selected_jets_.size(); i++)    {
      int rank = i+1;
      if ( std::find(ranks.begin(), ranks.end(), rank) == ranks.end() )
         continue;
      jets.push_back(selected_jets_[i]);
   }
   return jets;
}

std::vector<std::shared_ptr<Jet>> JetAnalyser::jets() {
   return jets_;
}

std::vector<std::shared_ptr<Jet>> JetAnalyser::selectedJets() {
   return selected_jets_;
}

void JetAnalyser::selectedJets(const std::vector< std::shared_ptr<Jet> > & jets ) {
   selected_jets_ = jets;
}

std::vector< std::shared_ptr<Jet> > JetAnalyser::ptSortedJets( const std::vector< std::shared_ptr<Jet> > & jets ) {
   std::vector< std::shared_ptr<Jet> > sorted_jets = jets;
   std::sort(sorted_jets.begin(),sorted_jets.end(),jetpt_ordering);
   return sorted_jets;
}

std::vector< std::shared_ptr<Jet> > JetAnalyser::btagSortedJets( const std::vector< std::shared_ptr<Jet> > & jets ) {
   std::vector< std::shared_ptr<Jet> > sorted_jets = jets;
   std::sort(sorted_jets.begin(),sorted_jets.end(),btag_ordering);
   return sorted_jets;
}

std::vector< std::shared_ptr<Jet> > JetAnalyser::concatenateJets( const std::vector< std::shared_ptr<Jet> > & jets1 ,const std::vector< std::shared_ptr<Jet> > & jets2) {
   std::vector< std::shared_ptr<Jet> > jets = jets1;
   jets.insert(jets.end(),jets2.begin(),jets2.end());
   return jets;
}

void JetAnalyser::fsrCorrections( const std::vector< std::shared_ptr<Jet> > & main_jets, const std::vector< std::shared_ptr<Jet> > & fsr_candidates, const float & deltaR_max) {
   std::string label = "*** Applying FSR corrections";
   for ( auto & fsr: fsr_candidates ) {
      for ( auto & jet: main_jets ) {
         if ( jet->deltaR(*fsr) < deltaR_max ) {
            jet->addFSR(fsr.get());
            break;
         }
      }
   }
   cutflow(label);
}

void JetAnalyser::HEMCorrection() {
   if (config_->hemCorrection() == true && config_->isMC() && selected_jets_.size() != 0) {
      std::string label = "HEM correction";
      // scale down the jet energy by 20 % for jets with -1.57 <phi< -0.87 and -2.5<eta<-1.3
      for(int i = 0; i < (int)selected_jets_.size(); i++)  {
         auto jet = selected_jets_[i];
         if (jet->eta() > -2.5 && jet->eta() < -1.3 && jet->phi() > -1.57 && jet->phi() < -0.87 && jet->pt() > 15 ) {
            selected_jets_[i]->p4(selected_jets_[i]->p4() *0.8);
         }
      }
      cutflow(label);
   }
   return;
}