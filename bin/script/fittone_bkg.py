#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas
from math import erf

import numpy
import os

directory = '../output/hists/bkg/test'

lep = ["lep", "chr"]
correction_level = ["nothing_false",
                    "regression_false",
                    "regression_fsr_false",
                    "regression_fsr_true",
]


def activ(x, par):
    return 0.5*(1 + erf(p[0]*(x[0] - p[1])))


def second(x, par):
    chi = 2*TMath.Sqrt(TMath.Log(4))
    sig0 = 2/chi*TMath.ArSinh(p[3]*chi/2)
    inside = 1 - (x[0] - p[1])*p[3]/p[2] - (x[0] - p[1])**2*p[3]*p[4]/p[3]
    return p[0]*TMath.Exp(-0.5*((TMath.Log(inside)/sig0)**2 + sig0**2))


def super_novosibirisk(x, par):
    return activ(x, par[:2])*second(x, par[-5:])


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

for l in lep:
    cur_dir = directory
    output_filename = os.path.join("../output/tables/bkg/test/",
                                   "_".join(["fit", l]) + ".txt")
    out_file = open(output_filename, "w")
    out_file.write("Intestazione")
    for c in correction_level:
        canvases[i] = TCanvas("_".join([c, l, mass, "2 match"]),
                              " ".join([c, l, mass, "2 match"]))
        out_file.write("\n")
        input_filename = c + ".root"
        input_filename = os.path.join(cur_dir, input_filename)
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
        funzione = TF1("novosibirisk", super_novosibirisk, 0, 800, 7)
        max_x = 1.1*limits[mass][1]
        min_x = 0.9*limits[mass][0]
        bins = 60
        binsize = (max_x - min_x)/bins
        histo = TH1F("histo", "Fittone", bins, min_x, max_x)
        canvases[i].cd()
        tree.Draw("Mass>>histo", " && ".join([filter_string, ]), "norm")
        histo.Scale(1./binsize)
        histo.Fit(funzione_test, "RLME", "", limits[mass][0], limits[mass][1])
        params, errors = prendi_parametri(funzione)
        print(params)
        funzione.SetParameters(*params)
        fit = tree.UnbinnedFit(
            "gaus1", "Mass",
            " && ".join([limit_string, filter_string]), "RLME")
        params, errors = prendi_parametri(funzione)
        funzione.DrawClone("same")
        scrivi_parametri(params, errors, out_file)
