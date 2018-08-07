#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment

import os
import sys

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


bkg_files = [{
    'mass': 'bkg',
    'highx': 800,
    'bins': 100,
    'basedir': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/DATA/Run2017/nano_94X_2017_rereco31Mar18-v1/BTagCSV/Run2017C-31Mar2018-v1/",
    'filenames': [
        "180730_153331/0000/nano_10.root",
        "180730_153331/0000/nano_11.root",
        "180730_153331/0000/nano_12.root",
        "180730_153331/0000/nano_13.root",
        "180730_153331/0000/nano_14.root",
        "180730_153331/0000/nano_15.root",
        "180730_153331/0000/nano_16.root",
        "180730_153331/0000/nano_17.root",
        "180730_153331/0000/nano_18.root",
        "180730_153331/0000/nano_19.root",
        "180730_153331/0000/nano_20.root",
        "180730_153331/0000/nano_21.root",
    ],
    },
]



template_loader = FileSystemLoader(searchpath='./')
template_env = Environment(loader=template_loader)
template = template_env.get_template("AnalysisJetsSmrBTagRegFSRMatch.j2")


if sys.argv[1] == "test":
    out_text = template.render(cl=correction_level[4][0],
                               trig=correction_level[4][1], **(mass_points[1]))
    filename = "_".join([mass_points[1]['mass'], correction_level[4][0],
                         correction_level[4][1]]) + ".cc"
    f = open(os.path.join("_tmp/src", filename), "w")
    f.write(out_text)
    f.close()

else:
    for params in mass_points_signal:
        for cl in correction_level_signal:
            out_text = template.render(cl=cl[0], trig=cl[1], bkg=False, **params)
            filename = "_".join([params['mass'], cl[0], cl[1]]) + ".cc"
            f = open(os.path.join("_tmp/src", filename), "w")
            f.write(out_text)
            f.close()
    for params in bkg_files:
        for cl in correction_level_bkg:
            out_text = template.render(cl=cl[0], trig=cl[1], bkg=True, **params)
            filename = "_".join([params['mass'], cl[0], cl[1]]) + ".cc"
            f = open(os.path.join("_tmp/src", filename), "w")
            f.write(out_text)
            f.close()
