.DEFAULT_TARGET=all
.PHONY=all run_all run test


BuildFile.xml: BuildFile.j2 script/create_buildfile.py
	./script/create_buildfile.py

_tmp/src/%.cc: AnalysisJetsSmrBTagRegFSRMatch.j2 BuildFile.xml
	@mkdir -p _tmp/src
	./script/create_all_corrections_sources.py all

test: BuildFile.xml AnalysisJetsSmrBTagRegFSRMatch.j2
	@mkdir -p _tmp/src
	./script/create_all_corrections_sources.py test
	scram b

all: _tmp/src/%.cc
	scram b -k -j5

run_all:
	./script/run_all.py mc

run_bkg:
	./script/run_all.py bkg

merge_bkg:
	./script/run_all.py bkg merge

run_sig:
	./script/run_all.py sig

merge_sig:
	./script/run_all.py sig merge
