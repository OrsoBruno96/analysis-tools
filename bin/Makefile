.DEFAULT_GOAL=all
.PHONY=all run_mc run_bkg run_sig merge_sig merge_bkg test


BuildFile.xml: BuildFile.j2 script/create_buildfile.py script/settings_parallelization.py
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

run_mc:
	./script/run_all.py mc

run_bkg:
	./script/run_all.py bkg

merge_bkg:
	./script/run_all.py bkg merge

run_sig:
	./script/run_all.py sig

merge_sig:
	./script/run_all.py sig merge
