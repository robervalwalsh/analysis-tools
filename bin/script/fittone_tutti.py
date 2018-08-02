#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1

import os



directory = 'hists/jets/new_mass_'

matches = [0, 1, 2]
lep = ["lep", "chr"]
correction_level = ["nothing_false",
                    "only_smearing_false",
                    "smearing_btag_false",
                    "smearing_btag_regression_false",
                    "smearing_btag_regression_fsr_false",
                    "smearing_btag_regression_fsr_true",
]
mass_points = ["120", "350", "1200"]
limits = {
    "120": [60, 150, 120],
    "350": [280, 360, 350],
    "1200": [900, 1400, 1200],
    }

# limits = {
#     "120": [60, 150, 120],
#     "350": [250, 380, 350],
#     "1200": [900, 1400, 1200],
#     }




def funVoigt(x, par):
    return TMath.Voigt(x[0] - par[0], par[1], par[2])


for mass in mass_points:
    limit_string = "Mass > " + str(limits[mass][0]) + \
                   " && Mass < " + str(limits[mass][1])
    print("Mass point: " + mass)
    for l in lep:
        cur_dir = directory + mass
        output_filename = os.path.join("../output/tables/before/",
                                       "_".join(["fit", l, mass]) + ".txt")
        out_file = open(output_filename, "w")
        out_file.write("# media (2 match)\t\t\t sigma(2 match)\t\t\tmedia (tutti)\t\t\tsigma (tutti)")
        for c in correction_level:
            out_file.write("\n")
            input_filename = c + ".root"
            input_filename = os.path.join(cur_dir, input_filename)
            root_file = TFile(input_filename, "r")
            tree = root_file.Get("output_tree")
            filter_string = ""
            lc = ""
            if l == "lep":
                lc = "lepton"
                filter_string = " && Leptonic_event"
            else:
                lc = "chromo"
                filter_string = " && !(Leptonic_event)"

            # funzioneVoigt = TF1("voigt", funVoigt, limits[mass][0], limits[mass][1], 3)
            funzione = TF1("gaus1", "gaus/sqrt(2*3.1415*[2])/gaus(0)",
                           limits[mass][0], limits[mass][1])
            funzione.SetParameters(10, limits[mass][2], 50)
            funzione.SetParLimits(0, 0, 50)
            fit_2_match = tree.UnbinnedFit(
                "gaus1", "Mass", "nMatches==2 && " +
                limit_string + filter_string, "RLME")
            media = funzione.GetParameter(1)
            dmedia = funzione.GetParError(1)
            sigma = funzione.GetParameter(2)
            dsigma = funzione.GetParError(2)
            # funzioneVoigt.SetParameters(limits[mass][2], 30, 20)
            # tentativo_disperato = tree.UnbinnedFit("voigt", "Mass", limit_string, "RLME")
            # print(funzioneVoigt.GetParameter(1))

            out_file.write(str(media) + "\t" + str(dmedia) + 
                           "\t" + str(sigma) + "\t" + str(dsigma) + "\t")
            fit_2_match = tree.UnbinnedFit("gaus1", "Mass",
                                           limit_string + filter_string, "RLME")
            media = funzione.GetParameter(1)
            dmedia = funzione.GetParError(1)
            sigma = funzione.GetParameter(2)
            dsigma = funzione.GetParError(2)
            out_file.write(str(media) + "\t" + str(dmedia) + 
                           "\t" + str(sigma) + "\t" + str(dsigma) + "\t")
