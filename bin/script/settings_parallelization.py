# -*- coding:utf-8 -*-


correction_level_signal = [("nothing", "false"),
                    ("only_smearing", "false"),
                    ("smearing_btag", "false"),
                    ("smearing_btag_regression", "false"),
                    ("smearing_btag_regression_fsr", "false"),
                    ("smearing_btag_regression_fsr", "true")
]

correction_level_bkg = [
    ("nothing", "false"),
    ("regression", "false"),
    ("regression_fsr", "false"),
    ("regression_fsr", "true")
]

mass_points_signal = [{'mass': "120",
                'highx': 400,
                'bins': 20,
                'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-120_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/",
                'filenames': ["180730_145111/0000/nano_2.root",
                              "180730_145111/0000/nano_5.root",
                              "180730_145111/0000/nano_1-1.root",
                              "180730_145111/0000/nano_1-2.root",
                              "180730_145111/0000/nano_1-3.root",
                              "180730_145111/0000/nano_1-4.root",
                              "180730_145111/0000/nano_1-5.root",
                              "180730_145111/0000/nano_1-6.root",
                              "180730_145111/0000/nano_1-7.root",
                              "180730_145111/0000/nano_1-8.root",
                ]},
               {'mass': "350",
                'highx': 800,
                'bins': 50,
                'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-350_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/",
                'filenames': ["180730_145049/0000/nano_2.root",
                              "180730_145049/0000/nano_3.root",
                              "180730_145049/0000/nano_4.root"
                              "180730_145049/0000/nano_5.root",
                              "180730_145049/0000/nano_6.root",
                              "180730_145049/0000/nano_7.root",
                              "180730_145049/0000/nano_8.root",
                              "180730_145049/0000/nano_9.root",
                              "180730_145049/0000/nano_10.root",
                              "180730_145049/0000/nano_11.root",
                ]},
               {'mass': "1200",
                'highx': 1600,
                'bins': 50,
                'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-1200_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/",
                'filenames': ["180730_145131/0000/nano_3.root",
                              "180730_145131/0000/nano_4.root",
                              "180730_145131/0000/nano_7.root",
                              "180730_145131/0000/nano_9.root",
                              "180730_145131/0000/nano_10.root",
                              "180730_145131/0000/nano_11.root",
                              "180730_145131/0000/nano_12.root",
                              "180730_145131/0000/nano_13.root",
                              "180730_145131/0000/nano_14.root",
                              "180730_145131/0000/nano_15.root",
                ]}
]



allfiles = ["180731_085953/0000/" + line.rstrip("\n")
            for line in open("script/data_files.txt", "r")]



size = 10
ranges = range(0, len(allfiles)/size)
bkg_files = [{
    'mass': 'bkg',
    'highx': 800,
    'bins': 100,
    'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017C-31Mar2018-v1/",
    'filenames': allfiles[size*i:size*(i+1)],
    } for i in ranges
]
