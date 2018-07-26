# analysis-tools
Core codes for the analysis framework

## Detail of the files - Zoratti
- `bin/AnalysisJets2.cc` only cuts on pt, eta and btagging
- `bin/AnalysisJetsRegCorr.cc` corrections with the 2017 algorithm for jets
- `bin/AnalysisjetsCorrSmeared.cc` same as above but smearing MC data to simulate real data.
- `bin/AnalysisJetsSmrBTagRegFSR.cc` first apply smearing to data, then apply scale factors to correct MC for btag, then uses regression algorithm to correct energy and then corrects for final state radiation.
