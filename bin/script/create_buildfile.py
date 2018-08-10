#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment
from settings_parallelization import correction_level_bkg, correction_level_signal, \
    mass_points_signal, bkg_files


mass_points = ["120", "350", "1200"]

template_loader = FileSystemLoader(searchpath='./')
template_env = Environment(loader=template_loader)

names = list()
for c in correction_level_signal:
    names.append("_".join(["sig", c[0], c[1]]))
names_bkg = list()
for c in correction_level_bkg:
    names_bkg.append("_".join(["bkg", c[0], c[1]]))

template = template_env.get_template("BuildFile.j2")
out_text = template.render(names=names, names_bkg=names_bkg)
f = open("BuildFile.xml", "w")
f.write(out_text)
f.close()



