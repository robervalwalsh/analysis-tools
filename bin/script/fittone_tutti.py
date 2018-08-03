#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas

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
# mass_points = ["120", "350", "1200"]
mass_points = ["350", ]

limits = {
    "120": [60, 150, 120],
    "350": [250, 380, 350],
    "1200": [900, 1400, 1200],
    }


def funVoigt(x, par):
    return TMath.Voigt(x[0] - par[0], par[1], par[2])

def doubleGauss(x, par):
    if (x[0] < par[0]):
        return TMath.Gaus(x[0], par[0], par[1], True)
    else:
        return TMath.Gaus(x[0], par[0], par[2], True)

def weibull(x, par):
    if (par[0] <= 0 or x[0] < 0):
        return 0
    else:
        return par[1]/par[0]*(x[0]/par[0])**(par[1] - 1)*TMath.Exp(-(x[0]/par[0])**par[1])
    
def gaussian(x, par):
    return TMath.Gaus(x[0], par[0], par[1], True)
    
def prendi_parametri(funzione):
    params = list()
    errors = list()
    for i in range(0, funzione.GetNpar()):
        params.append(funzione.GetParameter(i))
        errors.append(funzione.GetParError(i))
    return params, errors

def scrivi_parametri(params, errors, fileout):
    for p, e in zip(params, errors):
        fileout.write(str(p) + "\t" + str(e) + "\t")
    
# canvas = TCanvas("canvas", "", 700, 700)
    
for mass in mass_points:
    limit_string = "Mass > " + str(limits[mass][0]) + \
                   " && Mass < " + str(limits[mass][1])
    print("Mass point: " + mass)
    for l in lep:
        cur_dir = directory + mass
        output_filename = os.path.join("../output/tables/schifo/",
                                       "_".join(["fit", l, mass]) + ".txt")
        out_file = open(output_filename, "w")
        out_file.write("# media (2 match)\t\t\t sigma(2 match)\t\t\tmedia (tutti)\t\t\tsigma (tutti)")
        for c in correction_level:
            out_file.write("\n")
            input_filename = c + ".root"
            input_filename = os.path.join(cur_dir, input_filename)
            print(input_filename)
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

            # funzione = TF1("voigt", funVoigt, limits[mass][0], limits[mass][1], 3)
            # funzione = TF1("gaus1", "gaus/sqrt(2*3.1415*[2])/gaus(0)",
            #                limits[mass][0], limits[mass][1])
            # funzione = TF1("gaus1", gaussian, limits[mass][0], limits[mass][1], 2)
            # funzione = TF1("gaus1", doubleGauss, limits[mass][0], limits[mass][1], 3)
            # funzione = TF1("gaus1", weibull, limits[mass][0], limits[mass][1], 2)
            funzione.SetParameters(limits[mass][2], 30)
            # funzione.SetParLimits(1, 200, 400)
            # funzione.SetParLimits(2, 0, 80)
            fit_2_match = tree.UnbinnedFit(
                "gaus1", "Mass", "nMatches==2 && " +
                limit_string + filter_string, "RLME")
            # histo = TH1F("histo", "Fittone", 26, limits[mass][0], limits[mass][1])
            # print(histo.GetEntries())
            # tree.Draw("Mass>>histo", "nMatches==2 && " +
            #           limit_string + filter_string, "same")
            # # histo.Scale(1./histo.GetEntries())
            # histo.Scale(1./histo.Integral(0, 25))
            # histo.Scale(funzione.Integral(limits[mass][0], limits[mass][1]))
            # funzione.Draw("same")
            params, errors = prendi_parametri(funzione)
            scrivi_parametri(params, errors, out_file)
            fit_2_match = tree.UnbinnedFit("gaus1", "Mass",
                                           limit_string + filter_string, "RLME")
            params, errors = prendi_parametri(funzione)
            scrivi_parametri(params, errors, out_file)
