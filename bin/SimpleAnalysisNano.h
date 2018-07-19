#ifndef Analysis_Tools_bin_SimpleAnalysisNano_h
#define Analysis_Tools_bin_SimpleAnalysisNano_h 1

#include "TTree.h" 

TTree * tree_;

// event info
unsigned int         run_                    ;
unsigned long long   event_                  ;
unsigned int         lumisection_            ;

// GenJets
unsigned int         ngenjet_               ;
float                genjet_pt_[100]        ;


// Jets
unsigned int njets_   ;
float        jet_pt_  ;
float        jet_eta_ ;
float        jet_phi_ ;
float        jet_mass_;


#endif  // Analysis_Tools_bin_SimpleAnalysisNano_h
