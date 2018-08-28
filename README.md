# analysis-tools
Core codes for the analysis framework

## Compile
Works with CMSSW_9_4_9.

```bash
export SCRAM_ARCH=slc6_amd64_gcc630
cmsrel CMSSW_9_4_9
cd CMSSW_9_4_9/src
git clone https://github.com/OrsoBruno96/analysis-tools.git Analysis/Tools
cd Analysis/Tools/bin
make all
cd ..
scram b -k -j4
```

## Workflow

All my analysis is done with the template file `AnalysisJetsSmrBTagRegFSRMatch.j2`. With the commands in the `Makefile` you can produce all the binaries for running the analysis on both MC, signal and background.

To fit everything now the useful file is `bin/script/fit_script.py`, all the others are deprecated. This will produce some bash scripts that call the program `FitBackground`. Actually, the name is misleading, it will fit everything. The program `FitBackgroundRoofit` is deprecated.

To produce files for the CombineTool you can run the script `bin/script/merge_files_for_combine.py`.

The programs `AddNormalizationToFile`, `AnalysisQGLBtag`, `MoveRoohisto`, `PlotStackStyle`, `PlotWithStyle`, `RatioPlot`, `SplitTree` are utils file written by me. The names should suggest their use.

Please set the variable `base_dir` in the file `bin/script/settings_parallelization.py` before running stuff. It's the only absolute path that really needs to be set manually.
