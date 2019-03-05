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

// Main constructor
Config::Config(int argc, char ** argv) : opt_cmd_("Options"), opt_cfg_("Configuration")
{
   std::string datapath = Form("%s/src/Analysis/Tools/data",getenv("CMSSW_BASE"));
   argc_ = argc;
   argv_ = argv;
   // read configuration options
   try
   {
      namespace po = boost::program_options;
      opt_cmd_.add_options()
         ("help,h","Show help messages")
         ("config,c",po::value<std::string>(&cfg_),"Configuration file name");
      
      // analysis info
      opt_cfg_.add_options()
         ("ntuplesList",po::value <std::string> (&inputlist_)->default_value("rootFileList.txt"),"File with list of ntuples")
         ("collectionsTreePath",po::value <std::string> (&treePath_)->default_value("Events"),"Name of the tree path for the event collections")
         ("nEventsMax",po::value <int> (&nevtmax_)->default_value(-1), "Maximum number of events")
         ("nLumiSections",po::value <int> (&nlumis_)->default_value(-1), "Number of lumi sections processed")
         ("runMin",po::value <int> (&runmin_)->default_value(-1), "Minimum run number")
         ("runMax",po::value <int> (&runmax_)->default_value(-1), "Maximum run number")
         ("output",po::value <std::string> (&outputRoot_)->default_value("histograms.root"),"Output root file")
         ("json",po::value <std::string> (&json_)->default_value("no_json.txt"),"JSON file for data")
         ("isMC",po::value <bool> (&isMC_)->default_value(true),"Flag for MC dataset")
         ("pythia8",po::value <bool> (&pythia8_)->default_value(true),"Flag for Pythia8 or other recent generators MC")
         ("nlo",po::value <bool> (&nlo_)->default_value(false),"Flag for NLO samples")
         ("workflow",po::value <int> (&workflow_)->default_value(1),"Workflow index defined by user")
         ("blind",po::value <bool> (&blind_)->default_value(true),"Flag for blind analysis")
         ("signalRegion",po::value <bool> (&signalregion_)->default_value(true),"Flag for signal region")
         ("seed",po::value <int> (&seed_)->default_value(-1), "Seed value for random numbers")
         ("seedFile",po::value <std::string> (&seedfile_)->default_value("no_seed.txt"),"File with seed value for random numbers");

      // analysis control
      opt_cfg_.add_options()
         ("override",po::value <bool> (&override_)->default_value(true),"Flag to be used to override procedure, e.g. a selection");
      
      // jets
      opt_cfg_.add_options()
         ("jetsCollection",po::value <std::string> (&jetsCol_)->default_value(""),"Name of the jets collection")
         ("nJetsMin",po::value <int> (&njetsmin_)->default_value(0),"Minimum number of jets")
         ("nJetsMax",po::value <int> (&njetsmax_)->default_value(-1),"Maximum number of jets")
         ("jetsPtMin", po::value<std::vector<float> >(&jetsptmin_)->multitoken(),"Mimium pt of the jets")
         ("jetsPtMax", po::value<std::vector<float> >(&jetsptmax_)->multitoken(),"Maximum pt of the jets")
         ("jetsEtaMax", po::value<std::vector<float> >(&jetsetamax_)->multitoken(),"Maximum |eta| of the jets")
         ("jetsBtagMin", po::value<std::vector<float> >(&jetsbtagmin_)->multitoken(),"Minimum btag of the jets; if < 0 -> reverse btag")
         ("jetsBtagWP", po::value<std::vector<std::string> >(&jetsbtagwp_)->multitoken(),"Jets btag working point")
         ("jetsId",po::value <std::string> (&jetsid_)->default_value("tight"),"Jets id criteria for all jets")
         ("jetsPuId",po::value <std::string> (&jetspuid_)->default_value("loose"),"Jets pileup id criteria for all jets")
         ("jerPtRes",po::value <std::string> (&jerptres_)->default_value(""),"JER pT resolution file")
         ("jerSF",po::value <std::string> (&jersf_)->default_value(""),"JER SF file")
         ("bRegression",po::value <bool> (&bregression_)->default_value(false),"Apply b jet energy regression")
         ("doDijet",po::value <bool> (&dodijet_)->default_value(false),"Combine jets in dijet objects")
         ("useJetsFlavour",po::value <bool> (&usejetsflv_)->default_value(false),"For splitting results accoding to jet flavour")
         ("useJetsExtendedFlavour",po::value <bool> (&usejetsextflv_)->default_value(false),"For splitting results accoding to jet extended flavour")
         ("l1tJetsCollection",po::value <std::string> (&l1tjetsCol_)->default_value("l1tJets"),"Name of the L1T jets collection");

      // btagging
      opt_cfg_.add_options()
         ("nBJetsMin",po::value <int> (&nbjetsmin_)->default_value(0),"Minimum number of btgaged jets")
         ("btagSF",po::value <std::string> (&btagsf_)->default_value(""),"b-tagging scale factor in CSV format")
         ("btagAlgorithm",po::value <std::string> (&btagalgo_)->default_value("csvivf"),"BTag algorithm")
         ("btagWorkingPoint",po::value <std::string> (&btagwp_)->default_value("tight"),"BTag working point")
         ("btagLoose",po::value <float> (&btagwploose_)->default_value(0.46),"BTag working point LOOSE")
         ("btagMedium",po::value <float> (&btagwpmedium_)->default_value(0.84),"BTag working point MEDIUM")
         ("btagTight",po::value <float> (&btagwptight_)->default_value(0.92),"BTag working point TIGHT")
         ("btagXXX",po::value <float> (&btagwpxxx_)->default_value(0.92),"BTag undefined working point XXX")
         ("nonBtagWP",po::value <std::string> (&nonbtagwp_)->default_value(""),"non-Btag working point")
         ("nonBtagJet",po::value <int> (&nonbtagjet_)->default_value(-1),"non-Btag Jet");

      // muons
      opt_cfg_.add_options()
         ("muonsCollection",po::value <std::string> (&muonsCol_)->default_value(""),"Name of the muons collection")      
         ("nMuonsMin",po::value <int> (&nmuonsmin_)->default_value(0) ,"Minimum number of muons")
         ("nMuonsMax",po::value <int> (&nmuonsmax_)->default_value(-1),"Maximum number of muons")
         ("muonsPtMin", po::value<std::vector<float> >(&muonsptmin_)->multitoken(),"Mimium pt of the muons")
         ("muonsPtMax", po::value<std::vector<float> >(&muonsptmax_)->multitoken(),"Maximum pt of the muons")
         ("muonsEtaMax", po::value<std::vector<float> >(&muonsetamax_)->multitoken(),"Maximum |eta| of the muons")
         ("muonsId",po::value <std::string> (&muonsid_)->default_value("LOOSE"),"muons id criteria for all muons")
         ("l1tMuonsCollection",po::value <std::string> (&l1tmuonsCol_)->default_value("l1tMuons"),"Name of the L1T muons collection");

         
      // trigger
      opt_cfg_.add_options()
         ("hltPath",po::value <std::string> (&hltPath_)->default_value(""),"HLT path name")
         ("l1Seed",po::value <std::string> (&l1Seed_)->default_value(""),"L1 seed name")
         ("triggerObjects", po::value<std::vector<std::string> >(&triggerObjects_)->multitoken(),"Trigger objects")
         ("triggerObjectsMatches", po::value<std::vector<int> >(&triggerObjectsMatches_)->multitoken(),"Number of trigger objects matches")
         ("triggerObjectsMatchesRank", po::value<std::vector<int> >(&triggerObjectsMatchesRank_)->multitoken(),"Rank of offline object the trigger objects matches")
         ("triggerObjectsJets", po::value<std::vector<std::string> >(&triggerObjectsJets_)->multitoken(),"Trigger objects for b jets")
         ("triggerObjectsBJets", po::value<std::vector<std::string> >(&triggerObjectsBJets_)->multitoken(),"Trigger objects for btag jets")
         ("triggerObjectsL1Jets", po::value<std::vector<std::string> >(&triggerObjectsL1Jets_)->multitoken(),"Trigger objects for L1 jets")
         ("triggerObjectsJetsMatches", po::value<int> (&triggerObjectsJetsMatches_)->default_value(-1),"Number of matches with trigger objects for jets")
         ("triggerObjectsBJetsMatches", po::value<int> (&triggerObjectsBJetsMatches_)->default_value(-1),"Number of matches with trigger objects for b jets")
         ("triggerObjectsL1JetsMatches", po::value<int> (&triggerObjectsL1JetsMatches_)->default_value(-1),"Number of matches with L1 trigger objects jets")
         ("triggerObjectsMuons", po::value<std::vector<std::string> >(&triggerObjectsMuons_)->multitoken(),"Trigger objects for muons")
         ("triggerObjectsL1Muons", po::value<std::string> (&triggerObjectsL1Muons_)->default_value(""),"Trigger objects for L1 muons")
         ("triggerObjectsL3Muons", po::value<std::string> (&triggerObjectsL3Muons_)->default_value(""),"Trigger objects for L3 muons")
         ("triggerObjectsMuonsMatches", po::value<int> (&triggerObjectsMuonsMatches_)->default_value(-1),"Number of matches with trigger objects for muons")
         ("triggerObjectsL1MuonsMatches", po::value<int> (&triggerObjectsL1MuonsMatches_)->default_value(-1),"Number of matches with L1 trigger objects for muons")
         ("triggerObjectsL3MuonsMatches", po::value<int> (&triggerObjectsL3MuonsMatches_)->default_value(-1),"Number of matches with L3 trigger objects for muons")
         ("triggerMatchDeltaRMax",po::value <float> (&trgmatchdrmax_)->default_value(0.3),"DeltaR max for matching online-offline")
         ("triggerResultsCollection",po::value <std::string> (&triggerCol_)->default_value(""),"Name of the trigger results collection")
         ("triggerObjectsDirectory",po::value <std::string> (&triggerObjDir_)->default_value("slimmedPatTrigger"),"Name of the trigger objects directory");

      // generator level
      opt_cfg_.add_options()
         ("genparticlesCollection",po::value <std::string> (&genpartsCol_)->default_value(""),"Name of the gen particle collection")
         ("genjetsCollection",po::value <std::string> (&genjetsCol_)->default_value(""),"Name of the gen jets collection");
            
         
      // others
      opt_cfg_.add_options()
         ("nMin",po::value <int> (&nmin_)->default_value(0),"Minimum number objects")
         ("nMax",po::value <int> (&nmax_)->default_value(0),"Maximum number objects")
         ("ptMin", po::value<std::vector<float> >(&ptmin_)->multitoken(),"Mimium pt of an object")
         ("ptMax", po::value<std::vector<float> >(&ptmax_)->multitoken(),"Maximum pt of an object")
         ("etaMax", po::value<std::vector<float> >(&etamax_)->multitoken(),"Maximum |eta| of an object")
         ("dRMin",po::value <float> (&drmin_)->default_value(-1.),"Minimum delta R between candidates")
         ("dRMax",po::value <float> (&drmax_)->default_value(-1.),"Maximum delta R between candidates")
         ("dEtaMax",po::value <float> (&detamax_)->default_value(-1.),"Maximum delta eta between candidates")
         ("dEtaMin",po::value <float> (&detamin_)->default_value(-1.),"Minimum delta eta between candidates")
         ("dPhiMin",po::value <float> (&dphimin_)->default_value(-1.),"Minimum delta phi between candidates")
         ("dPhiMax",po::value <float> (&dphimax_)->default_value(-1.),"Maximum delta phi between candidates")
         ("qgMin",po::value <float> (&qgmin_)->default_value(-1),"Minimum value for q-g likelihood")
         ("qgMax",po::value <float> (&qgmax_)->default_value(-1),"Maximum value for q-g likelihood")
         ("ptImbalanceMin",po::value <float> (&ptimbalmin_)->default_value(-1),"Minimum relative imbalance between two candidates")
         ("ptImbalanceMax",po::value <float> (&ptimbalmax_)->default_value(-1),"Maximum relative imbalance between two candidates");
      
         
      // general
      opt_cfg_.add_options()
         ("massMin",po::value <float> (&massmin_)->default_value(-1.),"Cut on a mass, min value")
         ("massMax",po::value <float> (&massmax_)->default_value(-1.),"Cut on a mass, max value");
            
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
         if ( jerptres_ != "" )  jerptres_ = Form("%s/%s", datapath.c_str(), jerptres_.c_str());
         if ( jersf_    != "" )  jersf_    = Form("%s/%s", datapath.c_str(), jersf_.c_str()   );
         if ( btagsf_   != "" )  btagsf_   = Form("%s/%s", datapath.c_str(), btagsf_.c_str()  );
         
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

// analysis info
std::string        Config::ntuplesList()     const { return inputlist_; }
int                Config::nEventsMax()      const { return nevtmax_; }
bool               Config::isMC()            const { return isMC_; }
bool               Config::signalRegion()    const { return signalregion_; }
bool               Config::blind()           const { return blind_; }
bool               Config::nlo()             const { return nlo_; }
int                Config::workflow()        const { return workflow_; }

// analysis control
bool               Config::override()          const { return override_; }

// jets
std::string        Config::jetsCollection()     const { return jetsCol_; }
int                Config::nJetsMin()           const { return njetsmin_; }
int                Config::nJetsMax()           const { return njetsmax_; }
std::vector<float> Config::jetsPtMin()          const { return jetsptmin_; }
std::vector<float> Config::jetsPtMax()          const { return jetsptmax_; }
std::vector<float> Config::jetsEtaMax()         const { return jetsetamax_; }
std::string        Config::jetsId()             const { return jetsid_; }
std::string        Config::jetsPuId()           const { return jetspuid_; }
std::string        Config::jerPtRes()           const { return jerptres_; }
std::string        Config::jerSF()              const { return jersf_; }
std::string        Config::l1tJetsCollection()  const { return l1tjetsCol_; } 
std::vector<std::string> Config::jetsBtagWP()   const { return jetsbtagwp_; }
bool               Config::bRegression()        const { return bregression_; }
std::string        Config::nonBtagWP()          const { return nonbtagwp_; }
int                Config::nonBtagJet()         const { return nonbtagjet_; }
bool               Config::useJetsFlavour()     const { return usejetsflv_; }
bool               Config::useJetsExtendedFlavour() const { return usejetsextflv_; }
bool               Config::doDijet() const            { return dodijet_; }

// muons
std::string        Config::muonsCollection()    const { return muonsCol_; }
int                Config::nMuonsMin()          const { return nmuonsmin_; }
int                Config::nMuonsMax()          const { return nmuonsmax_; }
std::vector<float> Config::muonsPtMin()         const { return muonsptmin_; }
std::vector<float> Config::muonsPtMax()         const { return muonsptmax_; }
std::vector<float> Config::muonsEtaMax()        const { return muonsetamax_; }
std::string        Config::muonsId()            const { return muonsid_; }
std::string        Config::l1tMuonsCollection() const { return l1tmuonsCol_; } 

// trigger
std::string        Config::triggerResults()     const { return triggerCol_; }

// generator level
std::string        Config::genJetsCollection()       const { return genjetsCol_; }
std::string        Config::genParticlesCollection()  const { return genpartsCol_; }

// seed 
std::string        Config::seedFile()           const { return seedfile_; }
int                Config::seed()               const { return seed_;     }

bool               Config::pythia8()            const { return pythia8_;  }

// btag
float              Config::btagWP(const std::string & wp) const
{
   if ( wp == "loose"  ) return btagwploose_ ;
   if ( wp == "medium" ) return btagwpmedium_;
   if ( wp == "tight"  ) return btagwptight_ ;
   if ( wp == "xxx"    ) return btagwpxxx_;
   
   return -100.;
}
 
std::vector<std::string> Config::triggerObjectsJets() const
{
   return triggerObjectsJets_;
}
void Config::triggerObjectsJets(const std::string & label, const int & index)
{
   triggerObjectsJets_.at(index) = label;
}
// General stuff            
float Config::massMin() const { return massmin_; }
float Config::massMax() const { return massmax_; }
