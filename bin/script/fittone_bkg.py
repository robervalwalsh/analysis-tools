#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas, TGraph, TChain
from math import erf
from array import array
from hbbstyle import create_style, init_hist, CMS_prelim

import os
import pylab

directory = '../output/hists/bkg/second/'

# lep = ["lep", "chr"]
lep = ["lep", ]
correction_level = ["nothing_false",
                    "regression_false",
                    "regression_fsr_false",
                    "regression_fsr_true",
]

# initial_parameters = {
#     'nothing_false': [0.019036202047167248, 108.46227777619038, 60000, 94.19057345475692, 3457.2457898280404, 2.376534252399221, -0.7395165564524196],
#     'regression_false': [0.019036202047167248, 108.46227777619038, 70000, 94.19057345475692, 3457.2457898280404, 2.376534252399221, -0.7395165564524196],
#     'regression_fsr_false': [0.019036202047167248, 108.46227777619038, 70000, 94.19057345475692, 3457.2457898280404, 2.376534252399221, -0.7395165564524196],
#     'regression_fsr_true': [-0.00010365420408111028, 2544.169765556293, 90000, -14.047096630181915, 74.22295641759531, 1.2920013070751473, -0.005151969755087385],
# }


initial_parameters = [
    [0.0009711558376929633, 1862.1684253714488, 480559.7310660826, 43.53553217918233, 62.59323201876903, 1.4763639451494663, -0.007887396937449451], #[-0.00010365420408111028, 2544.169765556293, 90000, -14.047096630181915, 74.22295641759531, 1.2920013070751473, -0.005151969755087385],
    [-0.00010365420408111028, 2544.169765556293, 90000, -14.047096630181915, 74.22295641759531, 1.2920013070751473, -0.005151969755087385],
    [-0.00010365420408111028, 2544.169765556293, 90000, -14.047096630181915, 74.22295641759531, 1.2920013070751473, -0.005151969755087385],
]

subranges = [
    (120, 600),
    (400, 1100),
    (800, 1600),
]

bbins = [
    1000,
    300,
    50,
]

def activ(x, p):
    return 0.5*(1 + erf(p[0]*(x[0] - p[1])))


def second(x, p):
    if p[2] == 0 or p[3] == 0:
        return 10**20
    chi = 2*TMath.Sqrt(TMath.Log(4))
    sig0 = 2/chi*TMath.ASinH(p[3]*chi/2)
    inside = 1 - (x[0] - p[1])*p[3]/p[2] - (x[0] - p[1])**2*p[3]*p[4]/p[2]
    if inside < 0:
        return 10**20
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


def plot_residuals(histo, function, norm=False):
    x = array('d')
    y = array('d')
    for i in range(1, histo.GetNbinsX()):
        divisor = 1
        if norm:
            if histo.GetBinContent(i) == 0:
                divisor = 1
            else:
                divisor = TMath.Sqrt(histo.GetBinContent(i))
        else:
            divisor = 1
        # print("Debug: bin_content " + str(histo.GetBinContent(i)))
        # print("Debug: bin_center: " + str(histo.GetBinCenter(i)))
        # print("Debug: divisor " + str(divisor))
        # print("Debug: function " + str(function.Eval(histo.GetBinCenter(i))))
        # print("\n\n\n")
        y.append((histo.GetBinContent(i) -
                  function.Eval(histo.GetBinCenter(i)))/divisor)
        x.append(histo.GetBinCenter(i))
    chi2 = 0
    for yy in y:
        chi2 += yy**2
    return TGraph(histo.GetNbinsX(), x, y), chi2



create_style()

canvases = ["" for i in range(0, len(correction_level)*len(lep))]
# limit_string = "Mass < 600"
i = 0
for l in lep:
    cur_dir = directory
    output_filename = os.path.join("../output/tables/bkg/fourth/",
                                   "_".join(["fit", l]) + ".txt")
    out_file = open(output_filename, "w")
    out_file.write("# Super novosibirisk\n")
    out_file.write("# p0\tp1\tp2\tp3\tp4\tp5\tp6\tp7")
    input_filenames = ["C", "D", "E", "F"]
    input_filenames = [os.path.join("../output/links/", ii + ".root") for ii in input_filenames]
    tree = TChain("output_tree")
    for f in input_filenames:
        tree.Add(f)
    for init, ranges, bbbins in zip(initial_parameters, subranges, bbins):
        canvases[i] = TCanvas("_".join(["Triggered", l]),
                              " ".join(["Triggered", l]), 1500, 900)
        canvases[i].Divide(1, 2)
        canvases[i].cd(1)
        out_file.write("\n")
        filter_string = ""
        lc = ""
        if l == "lep":
            lc = "lepton"
            filter_string = "Leptonic_event"
        else:
            lc = "chromo"
            filter_string = "!(Leptonic_event)"
        limits_down = ranges[0]
        limits_up = ranges[1]
        limit_string = " && ".join(["(Mass < " + str(ranges[1]) + ")",
                                    "(Mass > " + str(ranges[0]) + ")"])
        max_x = 1.1*limits_up
        min_x = 0.9*limits_down
        funzione = TF1("novosibirisk", super_novosibirisk, min_x, max_x, 7)
        bins = bbbins
        binsize = (max_x - min_x)/bins
        histo = TH1F("histo", "Fittone", bins, limits_down, limits_up)
        init_hist(histo, "M_12 [GeV]", "Events / 10 GeV")
        canvases[i].cd(1)
        tree.Draw("Mass>>histo", " && ".join([filter_string, limit_string]), "")
        
        # funzione.SetParLimits(0, 0.01, 0.5)
        # funzione.SetParLimits(1, 80, 200)
        # funzione.SetParLimits(2, 30000, 800000)
        # funzione.SetParLimits(3, 2, 800)
        # funzione.SetParLimits(4, 100, 13000)
        # funzione.SetParLimits(5, -5, 5)
        # funzione.SetParLimits(6, 0, 0.8)
        funzione.SetParameters(*init)
        funzione.Draw("same")
        
        histo.Fit(funzione, "RLME", "", limits_down, limits_up)
        funzione.DrawClone("same")
        canvases[i].cd(2)
        graph, chi2 = plot_residuals(histo, funzione, True)
        graph.Draw("adp*")
        # CMS_prelim(False, "(13 TeV)", 1, 2)
        canvases[i].Update()
        print("Degrees of freedom")
        print(funzione.GetNDF())
        print("Chisquare:")
        print(chi2)
        print("Pvalue:")
        print(TMath.Prob(chi2, funzione.GetNDF()))
        params, errors = prendi_parametri(funzione)
        for p, e in zip(params, errors):
            out_file.write(str(p) + " " + str(e) + "\t")
        print(params)
        i += 1
        print(i)
        input("cusumano")

input("Fermati")
