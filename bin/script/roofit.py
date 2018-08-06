#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TFile, TCanvas
import ROOT
import os
ROOT.gStyle.SetOptStat(1)

directory = '../output/hists/jets/test_'

matches = [0, 1, 2]
# lep = ["lep", "chr"]
lep = ["lep", ]
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



w = ROOT.RooWorkspace()
w.factory('BifurGauss::g(Mass[0,800],mu[200,400],sigmaL[20, 50],sigmaR[20, 50])')
w.Print()

Mass =  w.var('Mass')
Mass.setRange(260, 380)
pdf = w.pdf('g')

frames = [Mass.frame() for i in range(0, len(mass_points)*len(correction_level)*len(lep))]

i = 0
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
            canv = TCanvas(c + l, c + l)
            canv.cd()
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
            data = ROOT.RooDataSet("Mass", "data", tree, ROOT.RooArgSet(Mass))
            data.plotOn(frames[i])
            fitResult = pdf.fitTo(data, ROOT.RooFit.Save(), ROOT.RooFit.PrintLevel(-1))
            fitResult.plotOn(frames[i], "cancro", "")
            print(fitResult.Print("same"))
            print(fitResult.floatParsFinal().Print("s"))
            # for p in fitResult.floatParsFinal():
            #     print(p)
            print(fitResult.covarianceMatrix())
            pdf.plotOn(frames[i])
            frames[i].Draw()
            i += 1

input("masknnafkjasjkf")
