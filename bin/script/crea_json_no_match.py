#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment

correction_level = ["nothing",
                    "only_smearing",
                    "smearing_btag",
                    "smearing_btag_regression",
                    "smearing_btag_regression_fsr"]

trigger_cuus = [
    "true",
    "false"
    ]


template_loader = FileSystemLoader(searchpath='./')
template_env = Environment(loader=template_loader)
template_file = "gen_first.json"
template = template_env.get_template(template_file)

for c in correction_level:
    for t in trigger_cuus:
        context = {
            'file_name': 'output/hists/jets/script/' + c + "_" + t + ".root",
        }
        out_text = template.render(**context)
        f = open(c + "_" + t + ".json", "w")
        f.write(out_text)
        f.close()
