# -*- coding:utf-8 -*-

from os import listdir

import os
import errno

correction_level_signal = [
     ("nothing", "false"),
     ("only_smearing", "false"),
     ("smearing_btag", "false"),
     ("smearing_btag_regression", "false"),
     ("smearing_btag_fsr", "false"),
     ("smearing_btag_regression_fsr", "false"),
     ("nothing", "true"),
     ("only_smearing", "true"),
     ("smearing_btag", "true"),
     ("smearing_btag_regression", "true"),
     ("smearing_btag_fsr", "true"),
     ("smearing_btag_regression_fsr", "true"),
]

correction_level_bkg = [
     ("nothing", "false"),
     ("regression", "false"),
     ("fsr", "false"),
     ("regression_fsr", "false"),
     ("nothing", "true"),
     ("regression", "true"),
     ("fsr", "true"),
     ("regression_fsr", "true"),
]

mass_points_signal = [{'mass': "120",
                'highx': 400,
                'bins': 20,
                'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-120_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145111/0000",
                'filenames': listdir("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-120_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145111/0000")},
               {'mass': "350",
                'highx': 800,
                'bins': 50,
                'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-350_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145049/0000/",
                'filenames': listdir("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-350_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145049/0000/")},
               {'mass': "1200",
                'highx': 1600,
                'bins': 50,
                'basedir':"/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-1200_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145131/0000/",
                'filenames': listdir("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-1200_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145131/0000/")}
]



files_eras = [
    {
        'filenames': listdir("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017C-31Mar2018-v1/180807_163631/0000/"),
        'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017C-31Mar2018-v1/180807_163631/0000/",
        'era': 'C',
    },
    {
        'filenames': listdir("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017D-31Mar2018-v1/180807_163703/0000/"),
        'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017D-31Mar2018-v1/180807_163703/0000/",
        'era': 'D',
    },
    {
        'filenames': listdir("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017E-31Mar2018-v1/180807_163732/0000"),
        'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017E-31Mar2018-v1/180807_163732/0000",
        'era': 'E',
    },
    {
        'filenames': listdir("/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017F-31Mar2018-v1/180807_163804/0000"),
        'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017F-31Mar2018-v1/180807_163804/0000",
        'era': 'F',
    }
]

def split_list(arr, size):
     arrs = []
     while len(arr) > size:
         pice = arr[:size]
         arrs.append(pice)
         arr   = arr[size:]
     arrs.append(arr)
     return arrs



bkg_files = [
    {
        'mass': 'bkg',
        'highx': 800,
        'bins': 100,
        'filenames': params['filenames'],
        'era': params['era'],
        'basedir': params['basedir'],
    }
    for params in files_eras
]


def name_of_lep(l):
    if l:
        return "lep"
    else:
        return "chr"


def open_and_create_dir(filename):
     if not os.path.exists(os.path.dirname(filename)):
          try:
               os.makedirs(os.path.dirname(filename))
          except OSError as exc:
               if exc.errno != errno.EEXIST:
                    raise
     return open(filename, "w")


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise
