# Reads all histograms and puts in a table the number of entries

from ROOT import TH1F, TFile

import os


directory = 'hists/jets/script_match/'

matches = [0, 1, 2]
lep = ["lep", "chr"]
correction_level = ["nothing",
                    "only_smearing",
                    "smearing_btag",
                    "smearing_btag_regression",
                    "smearing_btag_regression_fsr"]

trigger_level = [
    "true",
    "false"
    ]

for t in trigger_level:
    for l in lep:
        output_filename = os.path.join("output_tables", "_".join([l, t]) + ".txt")
        out_file = open(output_filename, "w")
        out_file.write("# 0 match\t1 match\t2 match")
        for c in correction_level:
            out_file.write("\n")
            input_filename = "_".join([c, t]) + ".root"
            input_filename = os.path.join(directory, input_filename)
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
