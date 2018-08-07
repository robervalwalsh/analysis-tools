#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment

from settings_parallelization import correction_level_bkg, correction_level_signal, \
    mass_points_signal, ranges, bkg_files

import os
import sys



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
    for params, i in zip(bkg_files, ranges):
        for cl in correction_level_bkg:
            out_text = template.render(cl=cl[0], trig=cl[1], bkg=True, **params)
            filename = "_".join([params['mass'], cl[0], cl[1], str(i)]) + ".cc"
            f = open(os.path.join("_tmp/src", filename), "w")
            f.write(out_text)
            f.close()
