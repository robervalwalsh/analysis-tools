#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TChain
from jinja2 import FileSystemLoader, Environment

import os

template_loader = FileSystemLoader(searchpath='./combine/templates/')
template_env = Environment(loader=template_loader)
template = template_env.get_template("datacard.j2")
template_script = template_env.get_template("combine_script.j2")


correction_level_bkg = ["nothing_false",
                    "regression_false",
                    "regression_fsr_false",
                    "regression_fsr_true",
]

correction_level_sig = [
    "smearing_btag_false",
    "smearing_btag_regression_false",
    "smearing_btag_regression_fsr_false",
    "smearing_btag_regression_fsr_true",
]

lep = [False, True]

eras = ["C", "D", "E", "F"]
mass_points = ["120", "350", "1200"]
limits = {
    "120": (0, 900, 40),
    "350": (0, 1000, 50),
    "1200": (0, 2000, 80),
}

directory_bkg = "../output/hists/bkg/fourth"
directory_splitted_bkg = "../output/split/bkg/"
directory_mc = "../output/hists/jets/mannaggia"
directory_sig = "../output/hists/sig/second/"
out_dir = "/afs/desy.de/user/z/zorattif/workdir/bin/output/combine_tool"
output_script_filedir = "../_tmp/script"

list_of_datacards = list()


for mass in mass_points:
    for cb, cs in zip(correction_level_bkg, correction_level_sig):
        for l in lep:
            appo_bkg = TChain("output_tree")
            appo_mc = TChain("output_tree")
            appo_sig = TChain("output_tree")
            if l:
                filter_string = "Leptonic_event"
                output_file_name_appo = "lep"
            else:
                filter_string = "!(Leptonic_event)"
                output_file_name_appo = "chr"
            limit = limits[mass]
            for e in eras:
                # appo_bkg.Add(os.path.join(directory_bkg, "_".join(["bkg", e, cb]) + ".root"))
                appo_sig.Add(os.path.join(directory_sig, "_".join(["sig", e, cb]) + ".root"))
            appo_bkg.Add(os.path.join(directory_splitted_bkg, "_".join([cb, "2"]) + ".root"))
            appo_mc.Add(os.path.join(directory_mc, "_".join([mass, cs]) + ".root"))
            output_file = TFile(os.path.join(
                out_dir, "_".join(["combine", output_file_name_appo, mass, cb]) + ".root"), "recreate")
            output_file.cd()
            histo_bkg = TH1F("bbnb_Mbb", "bbnb Mbb", limit[2], limit[0], limit[1])
            histo_mc = TH1F("MC_bbb_Mbb", "bbb Mbb MC", limit[2], limit[0], limit[1])
            histo_sig = TH1F("sig_bbb_Mbb", "bbb Mbb", limit[2], limit[0], limit[1])
            appo_bkg.Draw("Mass>>bbnb_Mbb", " && ".join([filter_string, ]), "")
            appo_mc.Draw("Mass>>MC_bbb_Mbb", " && ".join([filter_string, ]), "")
            appo_sig.Draw("Mass>>sig_bbb_Mbb", " && ".join([filter_string, ]), "")
            bkg_events = histo_bkg.GetEntries()
            histo_bkg.Write()
            histo_mc.Write()
            histo_sig.Write()

            output_filename = os.path.join(out_dir, "datacards")
            output_filename = os.path.join(output_filename,
                                           "_".join([mass, output_file_name_appo, cb]) + ".txt")
            out_text = template.render(
                obs=bkg_events,
                file_name=os.path.join(
                    out_dir, "_".join(["combine", output_file_name_appo, mass, cb]) + ".root"))
            output_file = open(output_filename, "w")
            output_file.write(out_text)
            list_of_datacards.append({'mass': mass, 'file': output_filename, 'correction': cb, 'lep': output_file_name_appo})

out_text = template_script.render(file_list=list_of_datacards, out_dir=os.path.join(out_dir, "out"))
out_filename = os.path.join(output_script_filedir, "run_combine_all.sh")
out_file = open(out_filename, "w")
out_file.write(out_text)
os.chmod(out_filename, 0755)
