/**\class Config Config.cc Analysis/Tools/src/Config.cc

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
#include "Analysis/Tools/interface/Config.h"

//
// class declaration
//

using namespace analysis;
using namespace analysis::tools;

//
// constructors and destructor
Config::Config()
{
}


Config::Config(int argc, char ** argv) : opt_cmd_("Options"), opt_cfg_("Configuration")
{
   argc_ = argc;
   argv_ = argv;
   // read configuration options
   try
   {
      namespace po = boost::program_options;
//      po::options_description desc("Options");
      opt_cmd_.add_options()
         ("help,h","Show help messages")
         ("config,c",po::value<std::string>(&cfg_),"Configuration file name");
      
//      po::options_description config("Configuration");
      opt_cfg_.add_options()
         ("ntuplesList",po::value <std::string> (&inputlist_)->default_value("rootFileList.txt"),"File with list of ntuples")
         ("nEventsMax",po::value <int> (&nevtmax_)->default_value(-1), "Maximum number of events")
         ("nLumiSections",po::value <int> (&nlumis_)->default_value(-1), "Number of lumi sections processed")
         ("runMin",po::value <int> (&runmin_)->default_value(-1), "Minimum run number")
         ("runMax",po::value <int> (&runmax_)->default_value(-1), "Maximum run number")
         ("output",po::value <std::string> (&outputRoot_)->default_value("histograms.root"),"Output root file")
         ("json",po::value <std::string> (&json_)->default_value("no_json.txt"),"JSON file for data")
         ("btagSF",po::value <std::string> (&btagsf_)->default_value("DeepCSV.csv"),"b-tagging scale factor in CSV format")
         ("nJetsMin",po::value <int> (&njetsmin_)->default_value(0),"Minimum number of jets")
         ("nJetsMax",po::value <int> (&njetsmax_)->default_value(-1),"Maximum number of jets")
         ("nBJetsMin",po::value <int> (&nbjetsmin_)->default_value(0),"Minimum number of btgaged jets")
         ("jetsPtMin", po::value<std::vector<float> >(&jetsptmin_)->multitoken(),"Mimium pt of the jets")
         ("jetsPtMax", po::value<std::vector<float> >(&jetsptmax_)->multitoken(),"Maximum pt of the jets")
         ("jetsEtaMax", po::value<std::vector<float> >(&jetsetamax_)->multitoken(),"Maximum |eta| of the jets")
         ("jetsBtagMin", po::value<std::vector<float> >(&jetsbtagmin_)->multitoken(),"Minimum btag of the jets; if < 0 -> reverse btag")
         ("jetsId",po::value <std::string> (&jetsid_)->default_value("tight"),"Jets id criteria for all jets")
         ("jetsPuId",po::value <std::string> (&jetspuid_)->default_value("loose"),"Jets pileup id criteria for all jets")
         ("l1tJetsNMin",po::value <int> (&l1tjetsnmin_)->default_value(0),"Minimum number of L1T jets")
         ("l1tJetsPtMin", po::value<std::vector<float> >(&l1tjetsptmin_)->multitoken(),"Mimium pt of the L1T jets")
         ("l1tJetsEtaMax", po::value<std::vector<float> >(&l1tjetsetamax_)->multitoken(),"Maximum |eta| of the L1T jets")
         ("l1tJetsRefNMin",po::value <int> (&l1tjetsrefnmin_)->default_value(0),"Minimum number of L1T jets for reference trigger")
         ("l1tJetsRefPtMin", po::value<std::vector<float> >(&l1tjetsrefptmin_)->multitoken(),"Mimium pt of the L1T jets for reference trigger")
         ("l1tJetsRefEtaMax", po::value<std::vector<float> >(&l1tjetsrefetamax_)->multitoken(),"Maximum |eta| of the L1T jets for reference trigger")
         ("nMuonsMin",po::value <int> (&nmuonsmin_)->default_value(0),"Minimum number of muons")
         ("muonsPtMin", po::value<std::vector<float> >(&muonsptmin_)->multitoken(),"Mimium pt of the muons")
         ("muonsPtMax", po::value<std::vector<float> >(&muonsptmax_)->multitoken(),"Maximum pt of the muons")
         ("muonsEtaMax", po::value<std::vector<float> >(&muonsetamax_)->multitoken(),"Maximum |eta| of the muons")
         ("muonsId",po::value <std::string> (&muonsid_)->default_value("LOOSE"),"muons id criteria for all muons")
         ("l1tMuonsNMin",po::value <int> (&l1tmuonsnmin_)->default_value(0),"Minimum number of L1T muons")
         ("l1tMuonsPtMin", po::value<std::vector<float> >(&l1tmuonsptmin_)->multitoken(),"Mimium pt of the L1T muons")
         ("l1tMuonsEtaMax", po::value<std::vector<float> >(&l1tmuonsetamax_)->multitoken(),"Maximum |eta| of the L1T muons")
         ("l1tMuonsRefNMin",po::value <int> (&l1tmuonsrefnmin_)->default_value(0),"Minimum number of L1T muons for reference trigger")
         ("l1tMuonsRefPtMin", po::value<std::vector<float> >(&l1tmuonsrefptmin_)->multitoken(),"Mimium pt of the L1T muons for reference trigger")
         ("l1tMuonsRefEtaMax", po::value<std::vector<float> >(&l1tmuonsrefetamax_)->multitoken(),"Maximum |eta| of the L1T muons for reference trigger")
         ("nMin",po::value <int> (&nmin_)->default_value(0),"Minimum number objects")
         ("nMax",po::value <int> (&nmax_)->default_value(0),"Maximum number objects")
         ("ptMin", po::value<std::vector<float> >(&ptmin_)->multitoken(),"Mimium pt of an object")
         ("ptMax", po::value<std::vector<float> >(&ptmax_)->multitoken(),"Maximum pt of an object")
         ("etaMax", po::value<std::vector<float> >(&etamax_)->multitoken(),"Maximum |eta| of an object")
         ("dRMin",po::value <float> (&drmin_)->default_value(0.),"Minimum delta R between candidates")
         ("dRMax",po::value <float> (&drmax_)->default_value(0.),"Maximum delta R between candidates")
         ("dEtaMax",po::value <float> (&detamax_)->default_value(10.),"Maximum delta eta between candidates")
         ("dPhiMin",po::value <float> (&dphimin_)->default_value(0.),"Minimum delta phi between candidates")
         ("ptImbalanceMax",po::value <float> (&ptimbalmax_)->default_value(1000.),"Maximum relative imbalance between two candidates")
         ("isMC",po::value <bool> (&isMC_)->default_value(true),"Flag for MC dataset")
         ("signalRegion",po::value <bool> (&signalregion_)->default_value(true),"Flag for signal region")
         ("hltPath",po::value <std::string> (&hltPath_)->default_value(""),"HLT path name")
         ("l1Seed",po::value <std::string> (&l1Seed_)->default_value(""),"L1 seed name")
         ("hltPathReference",po::value <std::string> (&hltPathRef_),"HLT path name for reference trigger for trigger efficiency")
         ("l1SeedReference",po::value <std::string> (&l1SeedRef_)->default_value(""),"L1 seed name for reference trigger")
         ("triggerObjects", po::value<std::vector<std::string> >(&triggerObjects_)->multitoken(),"Trigger objects")
         ("triggerObjectsMatches", po::value<std::vector<int> >(&triggerObjectsMatches_)->multitoken(),"Number of trigger objects matches")
         ("triggerObjectsMatchesRank", po::value<std::vector<int> >(&triggerObjectsMatchesRank_)->multitoken(),"Rank of offline object the trigger objects matches")
         ("triggerObjectsReference", po::value<std::vector<std::string> >(&triggerObjectsRef_)->multitoken(),"Trigger objects reference trigger")
         ("hltPathsList", po::value<std::vector<std::string> >(&hltPaths_)->multitoken(),"HLT paths list")
         ("hltPathsLogic",po::value <std::string> (&hltPathsLogic_)->default_value("OR"),"HLT paths logic (OR/AND)")
         ("hltPathsList2", po::value<std::vector<std::string> >(&hltPaths2_)->multitoken(),"HLT paths second list")
         ("hltPathsLogic2",po::value <std::string> (&hltPathsLogic2_)->default_value("OR"),"HLT paths logic (OR/AND) for second list")
         ("triggerObjectsJets", po::value<std::vector<std::string> >(&triggerObjectsJets_)->multitoken(),"Trigger objects for b jets")
         ("triggerObjectsBJets", po::value<std::vector<std::string> >(&triggerObjectsBJets_)->multitoken(),"Trigger objects for jets")
         ("triggerObjectsJetsMatches", po::value<int> (&triggerObjectsJetsMatches_)->default_value(-1),"Number of matches with trigger objects for jets")
         ("triggerObjectsBJetsMatches", po::value<int> (&triggerObjectsBJetsMatches_)->default_value(-1),"Number of matches with trigger objects for b jets")
         ("triggerObjectsMuons", po::value<std::vector<std::string> >(&triggerObjectsMuons_)->multitoken(),"Trigger objects for muons")
         ("btagAlgorithm",po::value <std::string> (&btagalgo_)->default_value("csvivf"),"BTag algorithm")
         ("btagWorkingPoint",po::value <std::string> (&btagwp_)->default_value("tight"),"BTag working point")
         ("btagWPLoose",po::value <float> (&btagwploose_)->default_value(0.46),"BTag working point LOOSE")
         ("btagWPMedium",po::value <float> (&btagwpmedium_)->default_value(0.84),"BTag working point MEDIUM")
         ("btagWPTight",po::value <float> (&btagwptight_)->default_value(0.92),"BTag working point TIGHT")
         ("nonbtagWP",po::value <float> (&nonbtagwp_)->default_value(0.46),"non-Btag working point")
         ("nonbtagJet",po::value <int> (&nonbtagjet_)->default_value(-1),"non-Btag Jet")
         ("matchOnlineOffline",po::value <bool> (&matchonoff_)->default_value(true),"Flag for doing matching online offline objects")
         ("matchOnlineOfflineDeltaRMax",po::value <float> (&matchonoffdrmax_)->default_value(0.4),"DeltaR max for matching online-offline")
         ("matchOnlineOfflineReference",po::value <bool> (&matchonoffref_)->default_value(true),"Flag for doing matching online offline objects when using a reference trigger")
         ("prescaleWeight",po::value <bool> (&psweight_)->default_value(false),"Flag for weighting histograms with prescale")
         ("prescaleNormalisation",po::value <float> (&psnorm_)->default_value(1.),"Normalisation factor of prescale weight")
         ("triggerEmulation",po::value <bool> (&trigemul_)->default_value(false),"Flag for using trigger emulation")
         ("jetsCollection",po::value <std::string> (&jetsCol_)->default_value(""),"Name of the jets collection")
         ("muonsCollection",po::value <std::string> (&muonsCol_)->default_value("slimmedMuons"),"Name of the muons collection")
         ("l1tJetsCollection",po::value <std::string> (&l1tjetsCol_)->default_value("l1tJets"),"Name of the L1T jets collection")
         ("l1tMuonsCollection",po::value <std::string> (&l1tmuonsCol_)->default_value("l1tMuons"),"Name of the L1T muons collection")
         ("genParticleCollection",po::value <std::string> (&genParticleCol_)->default_value("prunedGenParticles"),"Name of the gen particle collection")
         ("genjetsCollection",po::value <std::string> (&genjetsCol_)->default_value("slimmedGenJets"),"Name of the gen jets collection")
         ("triggerResultsCollection",po::value <std::string> (&triggerCol_)->default_value("TriggerResults"),"Name of the trigger results collection")
         ("triggerObjectsDirectory",po::value <std::string> (&triggerObjDir_)->default_value("slimmedPatTrigger"),"Name of the trigger objects directory")
         ("collectionsTreePath",po::value <std::string> (&treePath_)->default_value("Events"),"Name of the tree path for the event collections.");
         for ( int i = 0 ; i < 10 ; ++i )
         {
            opt_cfg_.add_options()
               (Form("triggerObject%dNMin",i),po::value <int> (&(tonmin_[i]))->default_value(0),Form("Minimum number of trigger objects #%d",i))
               (Form("triggerObject%dPtMin",i),po::value<std::vector<float> >(&(toptmin_[i]))->multitoken(),Form("Minimum pT of trigger objects #%d",i))
               (Form("triggerObject%dEtaMax",i),po::value<std::vector<float> >(&(toetamax_[i]))->multitoken(),Form("Minimum eta of trigger objects #%d",i))
               (Form("triggerObject%dRefNMin",i),po::value <int> (&(torefnmin_[i]))->default_value(0),Form("Minimum number of trigger objects #%d for reference trigger",i))
               (Form("triggerObject%dRefPtMin",i),po::value<std::vector<float> >(&(torefptmin_[i]))->multitoken(),Form("Minimum pT of trigger objects #%d for reference trigger",i))
               (Form("triggerObject%dRefEtaMax",i),po::value<std::vector<float> >(&(torefetamax_[i]))->multitoken(),Form("Minimum eta of trigger objects #%d for reference trigger",i));
         }
      
      po::variables_map vm; 
      try
      {
         po::store(po::parse_command_line(argc, argv, opt_cmd_), vm); // can throw
         // --help option
         
         if ( vm.count("help") )
         {
            std::cout << "SimpleBjetsAnalysis macro" << std::endl
                      << opt_cmd_ << std::endl
                      << opt_cfg_ << std::endl;
         }
         po::notify(vm);
         
         std::ifstream cfg_s(cfg_.c_str());
         po::store(po::parse_config_file(cfg_s, opt_cfg_), vm); // can throw
         if ( vm.count("config") )
         {

         }
         po::notify(vm);
         boost::algorithm::to_lower(jetsid_);
         std::transform(btagalgo_.begin(), btagalgo_.end(), btagalgo_.begin(), ::tolower);
         std::transform(btagwp_.begin(), btagwp_.end(), btagwp_.begin(), ::tolower);
         
         
      }
      catch(po::error& e)
      { 
         std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
         std::cerr << opt_cmd_ << std::endl;
         throw std::exception();
      } 
      
   }
   catch(std::exception& e) 
   { 
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
      throw std::exception();
   } 
   
}

Config::~Config()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//
// ------------ method called for each event  ------------

po::options_description  & Config::optionsCMD()
{
   return opt_cmd_;
}

po::options_description  & Config::optionsCFG()
{
   return opt_cfg_;
}


void Config::loadOptions()
{
   po::variables_map vm;
   po::store(po::parse_command_line(argc_, argv_, opt_cmd_), vm); 
   po::notify(vm);
   std::ifstream cfg_s(cfg_.c_str());
   po::store(po::parse_config_file(cfg_s, opt_cfg_), vm); // can throw
   po::notify(vm);
   
   
}
