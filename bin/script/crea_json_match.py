#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment
import os

correction_level = ["nothing_false",
                    "only_smearing_false",
                    "smearing_btag_false",
                    "smearing_btag_regression_false",
                    "smearing_btag_regression_fsr_false",
                    "smearing_btag_regression_fsr_true"]

matches = ["0", "1", "2"]
mass_points = ["120", "350", "1200"]

template_loader = FileSystemLoader(searchpath='./templates/')
template_env = Environment(loader=template_loader)
base_dir = "output/hists/jets/test_"

for mass in mass_points:
    template_file = "gen_mass_" + mass + ".json"
    template = template_env.get_template(template_file)
    cur_dir = base_dir + mass
    for c in correction_level:
        for lep in (True, False):
            coppie = []
            for m in matches:
                leg = ""
                name = ""
                colore = ""
                if int(m) == 0:
                    colore = "[255, 0, 0]"
                elif int(m) == 1:
                    colore = "[0, 255, 0]"
                elif int(m) == 2:
                    colore = "[0, 0, 255]"
                if lep:
                    leg = "Lep/" + m + " match"
                    nome = "mass_histo_lepton_" + m + "_match"
                else:
                    leg = "Non lep/" + m + " match"
                    nome = "mass_histo_chromo_" + m + "_match"
                coppie.append((nome, leg, colore))
            outname = ""
            if lep:
                outname = "_".join([c, "lep"]) + ".json"
            else:
                outname = "_".join([c, "chr"]) + ".json"
            context = {'file_name': os.path.join(cur_dir, c + ".root"),
                       'coppie': coppie}
            out_text = template.render(**context)
            f = open(os.path.join("../" + cur_dir, outname), "w")
            f.write(out_text)
            f.close()
