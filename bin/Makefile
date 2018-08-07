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
	scram b -k -j25

run_all:
	./script/run_all_parallel.sh

