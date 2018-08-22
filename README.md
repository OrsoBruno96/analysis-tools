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
