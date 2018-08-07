#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment

correction_level = ["nothing_false",
                    "only_smearing_false",
                    "smearing_btag_false",
                    "smearing_btag_regression_false",
                    "smearing_btag_regression_fsr_false",
                    "smearing_btag_regression_fsr_true"
]
correction_level_bkg = [
    "nothing_false",
    "regression_false",
    "regression_fsr_false",
    "regression_fsr_true"
]


mass_points = ["120", "350", "1200"]

template_loader = FileSystemLoader(searchpath='./')
template_env = Environment(loader=template_loader)

names = list()
for mass in mass_points:
    for c in correction_level:
        names.append("_".join([mass, c]))
names_bkg = list()
for c in correction_level_bkg:
    names_bkg.append("_".join(["bkg", c]))

template = template_env.get_template("BuildFile.j2")
out_text = template.render(names=names, names_bkg=names_bkg)
f = open("BuildFile.xml", "w")
f.write(out_text)
f.close()



