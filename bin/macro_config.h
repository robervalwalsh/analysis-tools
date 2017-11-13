#ifndef Analysis_Tools_macro_config_h_
#define Analysis_Tools_macro_config_h_

#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

int macro_config(int argc, char * argv[]);

std::string cfg_;

int nevtmax_;
int nlumis_;
bool isMC_;
bool signalregion_;
std::string inputlist_;
std::string outputRoot_;
std::string json_;

//
bool matchonoff_;
float matchonoffdrmax_;
bool matchonoffref_;
bool psweight_;
bool trigemul_;

// jets
int nl1jetsmin_;
std::vector<float> l1jetsptmin_;
std::vector<float> l1jetsetamax_;

int nl2jetsmin_;
std::vector<float> l2jetsptmin_;
std::vector<float> l2jetsetamax_;

int nl3jetsmin_;
std::vector<float> l3jetsptmin_;
std::vector<float> l3jetsetamax_;

int njetsmin_;
int nbjetsmin_;
std::vector<float> jetsptmin_;
std::vector<float> jetsptmax_;
std::vector<float> jetsetamax_;
std::vector<float> jetsbtagmin_;
std::string jetsid_;

// muons
int nl1muonsmin_;
std::vector<float> l1muonsptmin_;
std::vector<float> l1muonsetamax_;

int nl2muonsmin_;
std::vector<float> l2muonsptmin_;
std::vector<float> l2muonsetamax_;

int nl3muonsmin_;
std::vector<float> l3muonsptmin_;
std::vector<float> l3muonsetamax_;

int nmuonsmin_;
std::vector<float> muonsptmin_;
std::vector<float> muonsptmax_;
std::vector<float> muonsetamax_;
std::string muonsid_;

// additional cuts of unidentified objects or for extra selections
int nmin_;
std::vector<float> ptmin_;
std::vector<float> etamax_;


float drmin_;
float drmax_;
float detamax_;
float dphimin_;

float btagwploose_;
float btagwpmedium_;
float btagwptight_;
float btagwp_;
float nonbtagwp_;


std::string hltPath_;
std::string l1Seed_;
std::string hltPathRef_;
std::vector<std::string> triggerObjects_;
std::vector<std::string> hltPaths_;
std::string hltPathsLogic_;
std::vector<std::string> hltPaths2_;
std::string hltPathsLogic2_;
std::vector<std::string> triggerObjectsJets_;
std::vector<std::string> triggerObjectsMuons_;

// ntuples collections
std::string treePath_;
std::string jetsCol_; 
std::string muonsCol_; 
std::string triggerCol_;
std::string triggerObjDir_;

int macro_config(int argc, char * argv[])
{
   try
   {
      namespace po = boost::program_options;
      po::options_description desc("Options");
      desc.add_options()
         ("help,h","Show help messages")
         ("config,c",po::value<std::string>(&cfg_),"Configuration file name");
      
      po::options_description config("Configuration");
      config.add_options()
         ("ntuplesList",po::value <std::string> (&inputlist_)->default_value("rootFileList.txt"),"File with list of ntuples")
         ("nEventsMax",po::value <int> (&nevtmax_)->default_value(-1), "Maximum number of events")
         ("nLumiSections",po::value <int> (&nlumis_)->default_value(-1), "Number of lumi sections processed")
         ("output",po::value <std::string> (&outputRoot_)->default_value("histograms.root"),"Output root file")
         ("json",po::value <std::string> (&json_)->default_value("no_json.txt"),"JSON file for data")
//      
         ("nL1JetsMin",po::value <int> (&nl1jetsmin_)->default_value(0),"Minimum number of L1 trigger jets")
         ("l1JetsPtMin", po::value<std::vector<float> >(&l1jetsptmin_)->multitoken(),"Mimium pt of the L1 trigger jets")
         ("l1JetsEtaMax", po::value<std::vector<float> >(&l1jetsetamax_)->multitoken(),"Maximum |eta| of the L1 trigger jets")
         
         ("nL2JetsMin",po::value <int> (&nl2jetsmin_)->default_value(0),"Minimum number of L2 trigger jets")
         ("l2JetsPtMin", po::value<std::vector<float> >(&l2jetsptmin_)->multitoken(),"Mimium pt of the L2 trigger jets")
         ("l2JetsEtaMax", po::value<std::vector<float> >(&l2jetsetamax_)->multitoken(),"Maximum |eta| of the L2 trigger jets")

         ("nL3JetsMin",po::value <int> (&nl3jetsmin_)->default_value(0),"Minimum number of L3 trigger jets")
         ("l3JetsPtMin", po::value<std::vector<float> >(&l3jetsptmin_)->multitoken(),"Mimium pt of the L3 trigger jets")
         ("l3JetsEtaMax", po::value<std::vector<float> >(&l3jetsetamax_)->multitoken(),"Maximum |eta| of the L3 trigger jets")
         
         ("nJetsMin",po::value <int> (&njetsmin_)->default_value(0),"Minimum number of jets")
         ("nBJetsMin",po::value <int> (&nbjetsmin_)->default_value(0),"Minimum number of btgaged jets")
         ("jetsPtMin", po::value<std::vector<float> >(&jetsptmin_)->multitoken(),"Mimium pt of the jets")
         ("jetsPtMax", po::value<std::vector<float> >(&jetsptmax_)->multitoken(),"Maximum pt of the jets")
         ("jetsEtaMax", po::value<std::vector<float> >(&jetsetamax_)->multitoken(),"Maximum |eta| of the jets")
         ("jetsBtagMin", po::value<std::vector<float> >(&jetsbtagmin_)->multitoken(),"Minimum btag of the jets; if < 0 -> reverse btag")
         ("jetsId",po::value <std::string> (&jetsid_)->default_value("LOOSE"),"Jets id criteria for all jets")
//      
         ("nL1MuonsMin",po::value <int> (&nl1muonsmin_)->default_value(0),"Minimum number of L1 trigger muons")
         ("l1MuonsPtMin", po::value<std::vector<float> >(&l1muonsptmin_)->multitoken(),"Mimium pt of the L1 trigger muons")
         ("l1MuonsEtaMax", po::value<std::vector<float> >(&l1muonsetamax_)->multitoken(),"Maximum |eta| of the L1 trigger muons")
         
         ("nL2MuonsMin",po::value <int> (&nl2muonsmin_)->default_value(0),"Minimum number of L2 trigger muons")
         ("l2MuonsPtMin", po::value<std::vector<float> >(&l2muonsptmin_)->multitoken(),"Mimium pt of the L2 trigger muons")
         ("l2MuonsEtaMax", po::value<std::vector<float> >(&l2muonsetamax_)->multitoken(),"Maximum |eta| of the L2 trigger muons")

         ("nL3MuonsMin",po::value <int> (&nl3muonsmin_)->default_value(0),"Minimum number of L3 trigger muons")
         ("l3MuonsPtMin", po::value<std::vector<float> >(&l3muonsptmin_)->multitoken(),"Mimium pt of the L3 trigger muons")
         ("l3MuonsEtaMax", po::value<std::vector<float> >(&l3muonsetamax_)->multitoken(),"Maximum |eta| of the L3 trigger muons")
         
         ("nMuonsMin",po::value <int> (&nmuonsmin_)->default_value(0),"Minimum number of muons")
         ("muonsPtMin", po::value<std::vector<float> >(&muonsptmin_)->multitoken(),"Mimium pt of the muons")
         ("muonsPtMax", po::value<std::vector<float> >(&muonsptmax_)->multitoken(),"Maximum pt of the muons")
         ("muonsEtaMax", po::value<std::vector<float> >(&muonsetamax_)->multitoken(),"Maximum |eta| of the muons")
         ("muonsId",po::value <std::string> (&muonsid_)->default_value("LOOSE"),"muons id criteria for all muons")
//
         ("nMin",po::value <int> (&nmin_)->default_value(0),"Minimum number objects")
         ("ptMin", po::value<std::vector<float> >(&ptmin_)->multitoken(),"Mimium pt of an object")
         ("etaMax", po::value<std::vector<float> >(&etamax_)->multitoken(),"Maximum |eta| of an object")
//
         ("dRMin",po::value <float> (&drmin_)->default_value(0.),"Minimum delta R between candidates")
         ("dRMax",po::value <float> (&drmax_)->default_value(0.),"Maximum delta R between candidates")
         ("dEtaMax",po::value <float> (&detamax_)->default_value(10.),"Maximum delta eta between candidates")
         ("dPhiMin",po::value <float> (&dphimin_)->default_value(0.),"Minimum delta phi between candidates")
//      
         ("isMC",po::value <bool> (&isMC_)->default_value(true),"Flag for MC dataset")
         ("signalRegion",po::value <bool> (&signalregion_)->default_value(true),"Flag for signal region")
         ("hltPath",po::value <std::string> (&hltPath_),"HLT path name")
         ("l1Seed",po::value <std::string> (&l1Seed_),"L1 seed name")
         ("hltPathReference",po::value <std::string> (&hltPathRef_),"HLT path name for reference trigger for trigger efficiency")
         ("triggerObjects", po::value<std::vector<std::string> >(&triggerObjects_)->multitoken(),"Trigger objects")
         ("hltPathsList", po::value<std::vector<std::string> >(&hltPaths_)->multitoken(),"HLT paths list")
         ("hltPathsLogic",po::value <std::string> (&hltPathsLogic_)->default_value("OR"),"HLT paths logic (OR/AND)")
         ("hltPathsList2", po::value<std::vector<std::string> >(&hltPaths2_)->multitoken(),"HLT paths second list")
         ("hltPathsLogic2",po::value <std::string> (&hltPathsLogic2_)->default_value("OR"),"HLT paths logic (OR/AND) for second list")
         ("triggerObjectsJets", po::value<std::vector<std::string> >(&triggerObjectsJets_)->multitoken(),"Trigger objects for jets")
         ("triggerObjectsMuons", po::value<std::vector<std::string> >(&triggerObjectsMuons_)->multitoken(),"Trigger objects for muons")
//      
         ("btagWPLoose",po::value <float> (&btagwploose_)->default_value(0.46),"BTag working point LOOSE")
         ("btagWPMedium",po::value <float> (&btagwpmedium_)->default_value(0.84),"BTag working point MEDIUM")
         ("btagWPTight",po::value <float> (&btagwptight_)->default_value(0.92),"BTag working point TIGHT")
//         
         ("btagWP",po::value <float> (&btagwp_)->default_value(0.8484),"Btag working point")
         ("nonbtagWP",po::value <float> (&nonbtagwp_)->default_value(0.46),"non-Btag working point")
         
//
         ("matchOnlineOffline",po::value <bool> (&matchonoff_)->default_value(true),"Flag for doing matching online offline objects")
         ("matchOnlineOfflineDeltaRMax",po::value <float> (&matchonoffdrmax_)->default_value(0.4),"DeltaR max for matching online-offline")
         ("matchOnlineOfflineReference",po::value <bool> (&matchonoffref_)->default_value(true),"Flag for doing matching online offline objects when using a reference trigger")
         ("prescaleWeight",po::value <bool> (&psweight_)->default_value(false),"Flag for weighting histograms with prescale")
         ("triggerEmulation",po::value <bool> (&trigemul_)->default_value(false),"Flag for using trigger emulation")
         
         ("Jets",po::value <std::string> (&jetsCol_)->default_value("slimmedJetsPuppi"),"Name of the jets collection")
         ("Muons",po::value <std::string> (&muonsCol_)->default_value("slimmedMuons"),"Name of the muons collection")
         ("TriggerResults",po::value <std::string> (&triggerCol_)->default_value("TriggerResults"),"Name of the trigger results collection")
         ("TriggerObjectsDirectory",po::value <std::string> (&triggerObjDir_)->default_value("slimmedPatTrigger"),"Name of the trigger objects directory")
         ("CollectionsTreePath",po::value <std::string> (&treePath_)->default_value("Events"),"Name of the tree path for the event collections.");

         
      
      po::variables_map vm; 
      try
      {
         po::store(po::parse_command_line(argc, argv, desc), vm); // can throw
         // --help option
         
         if ( vm.count("help") )
         {
            std::cout << "SimpleBjetsAnalysis macro" << std::endl
                      << desc << std::endl
                      << config << std::endl;
            return 1;
         }
         po::notify(vm);
         
         std::ifstream cfg_s(cfg_.c_str());
         po::store(po::parse_config_file(cfg_s, config), vm); // can throw
         if ( vm.count("config") )
         {

         }
         po::notify(vm);
         boost::algorithm::to_upper(jetsid_);
         if ( (int)jetsptmin_.size() != njetsmin_ )
         {
            std::cout << "Config Error *** Jet minimum pt were not defined or the definition does not match the minimum number of jets" <<std::endl;
            return -1;
         }
         if ( (int)jetsptmax_.size() != njetsmin_ && !jetsptmax_.empty() )
         {
            std::cout << "Config Error *** Jet maximum pt has been defined and does not match the minimum number of jets" <<std::endl;
            return -1;
         }
         if ( (int)jetsetamax_.size() != njetsmin_ )
         {
            std::cout << "Config Error *** Jet maximum |eta| were not defined or the definition does not match the minimum number of jets" <<std::endl;
            return -1;
         }
         if ( (int)jetsbtagmin_.size() != nbjetsmin_ && !jetsbtagmin_.empty() )
         {
            std::cout << "Config Error *** Jet minimum btag has been defined and does not match the minimum number of btagged jets" <<std::endl;
            return -1;
         }

         
         if ( (int)muonsptmin_.size() != nmuonsmin_ )
         {
            std::cout << "Config Error *** Muon minimum pt were not defined or the definition does not match the minimum number of muons" <<std::endl;
            return -1;
         }
         if ( (int)muonsptmax_.size() != nmuonsmin_ && !muonsptmax_.empty() )
         {
            std::cout << "Config Error *** Muon maximum pt has been defined and does not match the minimum number of muons" <<std::endl;
            return -1;
         }
         if ( (int)muonsetamax_.size() != nmuonsmin_ )
         {
            std::cout << "Config Error *** Muon maximum |eta| were not defined or the definition does not match the minimum number of muons" <<std::endl;
            return -1;
         }
         
         
      }
      catch(po::error& e)
      { 
         std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
         std::cerr << desc << std::endl; 
         return -1; 
      } 
      
   }
   catch(std::exception& e) 
   { 
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
      return -1; 
   } 
   
   return 0;
}

   
#endif
