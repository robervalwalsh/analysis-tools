#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment

import os
import sys

correction_level = [("nothing", "false"),
                    ("only_smearing", "false"),
                    ("smearing_btag", "false"),
                    ("smearing_btag_regression", "false"),
                    ("smearing_btag_regression_fsr", "false"),
                    ("smearing_btag_regression_fsr", "true")
]
mass_points = [{'mass': "120",
                'highx': 400,
                'bins': 20,
                'filename': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-120_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145111/0000/nano_1-1.root"},
               {'mass': "350",
                'highx': 800,
                'bins': 50,
                'filename': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-350_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180712_150216/0000/nano_1.root"},
               {'mass': "1200",
                'highx': 1600,
                'bins': 50,
                'filename': "/pnfs/desy.de/cms/tier2/store/user/rwalsh/Analysis/Ntuples/MC/Fall17/nano_94X_mc_2017_fall17-v1/SUSYGluGluToBBHToBB_NarrowWidth_M-1200_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/180730_145131/0000/nano_1-1.root"}
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
    for params in mass_points:
        for cl in correction_level:
            out_text = template.render(cl=cl[0], trig=cl[1], **params)
            filename = "_".join([params['mass'], cl[0], cl[1]]) + ".cc"
            f = open(os.path.join("_tmp/src", filename), "w")
            f.write(out_text)
            f.close()
