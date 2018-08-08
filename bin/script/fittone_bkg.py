#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas
from math import erf

import numpy
import os
import pylab

directory = '../output/hists/bkg/test'

lep = ["lep", "chr"]
correction_level = ["nothing_false",
                    "regression_false",
                    "regression_fsr_false",
                    "regression_fsr_true",
]


def activ(x, p):
    return 0.5*(1 + erf(p[0]*(x[0] - p[1])))


def second(x, p):
    if p[2] == 0 or p[3] == 0:
        return 10**20
    chi = 2*TMath.Sqrt(TMath.Log(4))
    sig0 = 2/chi*TMath.ASinH(p[3]*chi/2)
    inside = 1 - (x[0] - p[1])*p[3]/p[2] - (x[0] - p[1])**2*p[3]*p[4]/p[2]
    return p[0]*TMath.Exp(-0.5*((TMath.Log(inside)/sig0)**2 + sig0**2))


def super_novosibirisk(x, par):
    return activ(x, [par[0], par[1]])*second(
        x, [par[2], par[3], par[4], par[5], par[6]])


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


canvases = ["" for i in range(0, len(correction_level)*len(lep))]
i = 0


limit_string = "Mass < 800"


for l in lep:
    cur_dir = directory
    output_filename = os.path.join("../output/tables/bkg/test_new/",
                                   "_".join(["fit", l]) + ".txt")
    out_file = open(output_filename, "w")
    out_file.write("Intestazione")
    for c in correction_level:
        canvases[i] = TCanvas("_".join([c, l]),
                              " ".join([c, l]))
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
        limits_up = 800
        limits_down = 0
        max_x = 1.1*limits_up
        min_x = 0.9*limits_down
        bins = 60
        binsize = (max_x - min_x)/bins
        histo = TH1F("histo", "Fittone", bins, min_x, max_x)
        canvases[i].cd()
        tree.Draw("Mass>>histo", " && ".join([filter_string, ]), "norm")
        histo.Scale(1./binsize)

        funzione.SetParLimits(0, 0, 0.1)
        funzione.SetParLimits(3, 0, 800)
        funzione.SetParameters(.03, 100, 0.006, 110, 5000, 0.1, 0.1)
        funzione.Draw("same")
        # input("cusumano")
        
        histo.Fit(funzione, "RLME", "", limits_down, limits_up)
        funzione.DrawClone("same")
        params, errors = prendi_parametri(funzione)
        print(params)
        funzione.SetParameters(*params)
        fit = tree.UnbinnedFit(
            "novosibirisk", "Mass",
            " && ".join([limit_string, filter_string]), "RLME")
        params, errors = prendi_parametri(funzione)
        funzione.DrawClone("same")
        scrivi_parametri(params, errors, out_file)
        input("test")


