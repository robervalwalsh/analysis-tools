.DEFAULT_TARGET=all
.PHONY=all run_all run test


BuildFile.xml: AnalysisJetsSmrBTagRegFSRMatch.j2
	./script/create_buildfile.py

# _tmp/src/$(wildcard *.cc):
cusumano: AnalysisJetsSmrBTagRegFSRMatch.j2 BuildFile.xml
	@mkdir -p _tmp/src
	./script/create_all_corrections_sources.py all

test: BuildFile.xml
	@mkdir -p _tmp/src
	./script/create_all_corrections_sources.py test
	scram b

all: cusumano
	scram b -k -j20

run_all:
	./script/run_all_parallel.sh

