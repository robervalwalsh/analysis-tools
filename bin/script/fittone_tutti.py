#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas
# from scipy.stats import crystalball as _crys

import numpy
import os

directory = '../output/hists/jets/stats_'

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
    "350": [180, 440, 330],
    "1200": [900, 1400, 1200],
    }


draw = True


def funVoigt(x, par):
    return TMath.Voigt(x[0] - par[0], par[1], par[2])


def doubleGauss(x, par):
    norm = numpy.pi*(par[1] + par[2])**2/2
    if norm == 0:
        return 10**20
    if (x[0] < par[0]):
        return TMath.Gaus(x[0], par[0], par[1], False)/(norm**0.5)
    else:
        return TMath.Gaus(x[0], par[0], par[2], False)/(norm**0.5)

    
def weibull(x, par):
    if (par[0] <= 0 or x[0] < 0):
        return 0
    else:
        return par[1]/par[0]*(x[0]/par[0])**(par[1] - 1)*TMath.Exp(-(x[0]/par[0])**par[1])

    
def gaussian(x, par):
    return TMath.Gaus(x[0], par[0], par[1], True)


def crystalball(x, par):
    return _crys((x - par[0])/par[1], par[2], par[3])

    
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
    


canvases = ["" for i in range(0, len(mass_points)*len(correction_level)*len(lep))]
i = 0

for mass in mass_points:
    limit_string = "Mass > " + str(limits[mass][0]) + \
                   " && Mass < " + str(limits[mass][1])
    print("Mass point: " + mass)
    for l in lep:
        cur_dir = directory + mass
        output_filename = os.path.join("../output/tables/stats_after/",
                                       "_".join(["fit", l, mass]) + ".txt")
        print("\n\n\n\n\n")
        print(output_filename)
        print("\n\n\n\n")
        out_file = open(output_filename, "w")
        out_file.write("# media (2 match)\t\t\t sigma(2 match)\t\t\tmedia (tutti)\t\t\tsigma (tutti)")
        for c in correction_level:
            canvases[i] = TCanvas("_".join([c, l, mass, "2 match"]),
                                  " ".join([c, l, mass, "2 match"]))
            # canvases[i + 1] = TCanvas("_".join([c, l, mass, "all"]),
            #                           " ".join([c, l, mass, "all"]))
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
                filter_string = "Leptonic_event"
            else:
                lc = "chromo"
                filter_string = "!(Leptonic_event)"
            # funzione = TF1("voigt", funVoigt, limits[mass][0], limits[mass][1], 3)
            # funzione = TF1("gaus1", gaussian, limits[mass][0], limits[mass][1], 2)
            funzione = TF1("gaus1", doubleGauss, limits[mass][0], limits[mass][1], 3)
            funzione_test = TF1("gaus2", doubleGauss, limits[mass][0], limits[mass][1], 3)
            # funzione = TF1("gaus1", weibull, limits[mass][0], limits[mass][1], 2)
            # funzione = TF1("gaus1", crystalball, limits[mass][0], limits[mass][1], 4)
            # funzione.SetParameters(limits[mass][2], 50, 50)
            # funzione.SetParLimits(1, 0, 100)
            # funzione.SetParLimits(2, 0, 100)
            max_x = 1.1*limits[mass][1]
            min_x = 0.9*limits[mass][0]
            bins = 60
            binsize = (max_x - min_x)/bins
            funzione_test.SetParameters(limits[mass][2], 30, 30)
            histo = TH1F("histo", "Fittone", bins, min_x, max_x)
            canvases[i].cd()
            tree.Draw("Mass>>histo", " && ".join(["nMatches==2", filter_string]), "norm")
            histo.Scale(1./binsize)
            histo.Fit(funzione_test, "RLME", "", limits[mass][0], limits[mass][1])
            params, errors = prendi_parametri(funzione_test)
            print(params)
            funzione.SetParameters(*params)
            fit_2_match = tree.UnbinnedFit(
                "gaus1", "Mass",
                " && ".join(["nMatches==2", limit_string, filter_string]), "RLME")
            params, errors = prendi_parametri(funzione)
            funzione.DrawClone("same")
            scrivi_parametri(params, errors, out_file)
            fit_2_match = tree.UnbinnedFit(
                "gaus1", "Mass", " && ".join([limit_string, filter_string]), "RLME")
            # canvases[i + 1].cd()
            # histo_all = TH1F("histo_all", "Fittone all", bins, min_x, max_x)
            # tree.Draw("Mass>>histo_all", "", "norm")
            # histo_all.Scale(1./binsize)
            # funzione.DrawClone("same")
            params, errors = prendi_parametri(funzione)
            scrivi_parametri(params, errors, out_file)
            # i += 2

cusumano = input("cusumano")
