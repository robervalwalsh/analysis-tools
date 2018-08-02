#!/usr/bin/env python
# -*- coding:utf-8 -*-

# Reads all histograms and puts in a table the number of entries

from ROOT import TH1F, TFile

import os


directory = '../output/hists/jets/'

matches = [0, 1, 2]
lep = ["lep", "chr"]
correction_level = ["nothing_false",
                    "only_smearing_false",
                    "smearing_btag_false",
                    "smearing_btag_regression_false",
                    "smearing_btag_regression_fsr_false",
                    "smearing_btag_regression_fsr_true"]

mass_points = ["120", "350", "1200"]


for mass in mass_points:
    cur_dir = os.path.join(directory, "before_mass_" + mass)
    for l in lep:
        output_filename = os.path.join("../output/tables/before/match", str(mass))
        output_filename = os.path.join(output_filename, "_".join([l, mass]) + ".txt")
        print(output_filename)
        out_file = open(output_filename, "w")
        out_file.write("# 0 match\t1 match\t2 match")
        for c in correction_level:
            out_file.write("\n")
            input_filename = c + ".root"
            input_filename = os.path.join(cur_dir, input_filename)
            root_file = TFile(input_filename, "r")
            for m in matches:
                lc = ""
                if l == "lep":
                    lc = "lepton"
                else:
                    lc = "chromo"
                hist_name = "_".join(["mass_histo", lc, str(m), "match;1"])
                hist = root_file.Get(hist_name)
                entries = hist.GetEntries()
                out_file.write(str(entries) + "\t")
