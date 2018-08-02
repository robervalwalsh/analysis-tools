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
matches = ["0", "1", "2"]

template_loader = FileSystemLoader(searchpath='./')
template_env = Environment(loader=template_loader)
template_file = "gen_match.json"
template = template_env.get_template(template_file)

for c in correction_level:
    for t in trigger_cuus:
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
                outname = c + "_" + t + "_lep.json"
            else:
                outname = c + "_" + t + "_chr.json"
            context = {'file_name': 'output/hists/jets/script_match/' +
                       c + "_" + t + ".root",
                       'coppie': coppie}
            out_text = template.render(**context)
            f = open("outdir/" + outname, "w")
            f.write(out_text)
            f.close()
