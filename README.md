# Analysis/Tools

**Core codes for the DESY CMS MSSM Higgs -> bb analysis framework**

* [Compilation](#compilation)

## Compilation

The codes as well as the ntuples are independent of CMSSW. However, in order to compile it uses `scram`.
So the latest version in any architecture should be fine. 

```bash
cmsrel CMSSW_X_Y_Z
cd CMSSW_X_Y_Z/src
cmsenv
git clone https://github.com/desy-cms/analysis-tools.git Analysis/Tools 
scram b -j4 USER_CXXFLAGS="-Wno-misleading-indentation"
```

:zzz: The `USER_CXXFLAGS="-Wno-misleading-indentation"` prevents a large number of warnings
from misleading identation in modules of the boost library. User can also define an
environment variable, of course, before compiling
```bash
export USER_CXXFLAGS="-Wno-misleading-indentation"
```
