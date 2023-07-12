#!/usr/bin/env python

from __future__ import print_function

import sys,os
import array
from argparse import ArgumentParser
from argparse import HelpFormatter

from ROOT import TFile,TCanvas, TMultiGraph,TLine, TRatioPlot, gStyle, TH1, gROOT
from ROOT import kRed, kBlue, kBlack, kMagenta, kGreen, kCyan

import Analysis.Tools.CMS_lumi as cmslumi
import Analysis.Tools.tdrstyle as tdrstyle


TH1.SetDefaultSumw2()

#set the tdr style (why some setting don't work? using gStyle??? - not all settings work w/ gStyle???)
tdrstyle.setTDRStyle()

# More styles

#change the cmslumi variables (see cmslumi.py)
cmslumi.lumi_13TeV = "50 fb^{-1}"
cmslumi.writeExtraText = 1
cmslumi.extraText = "Simulation"
cmslumi.lumi_sqrtS = "13 TeV" # used with iPeriod = 0, e.g. for simulation-only plots (default is an empty string)

iPos = 0
if( iPos==0 ): cmslumi.relPosX = 0.12

# 
# Simple example of macro: plot with CMS name and lumi text
#  (this script does not pretend to work in all configurations)
# iPeriod = 1*(0/1 7 TeV) + 2*(0/1 8 TeV)  + 4*(0/1 13 TeV) 
# For instance: 
#               iPeriod = 3 means: 7 TeV + 8 TeV
#               iPeriod = 7 means: 7 TeV + 8 TeV + 13 TeV 
#               iPeriod = 0 means: free form (uses lumi_sqrtS)
# Initiated by: Gautier Hamel de Monchenault (Saclay)
# Translated in Python by: Joshua Hardenbrook (Princeton)
# Updated by:   Dinko Ferencek (Rutgers)
#

iPeriod = 0

def main():
    parser = ArgumentParser(prog='simple_cms_plot.py', formatter_class=lambda prog: HelpFormatter(prog,indent_increment=6,max_help_position=80,width=280), description='CMS plots',add_help=True)
    parser.add_argument("--input_files", dest="input_files", help="list of input files")
    parser.add_argument("--histograms", dest="histograms", help="list of histograms of each input file")
    parser.add_argument("--colors", dest="colors", help="color for each histogram")
    parser.add_argument("--draw_options", dest="draw_options", help="draw options for each histogram")
    parser.add_argument("--markers", dest="markers", help="markers for each histogram")
    parser.add_argument("--same", dest="same", action='store_true', help="all in the same canvas")
    parser.add_argument("--norm1", dest="norm1", action='store_true', help="normalise all to 1")
    args = parser.parse_args()

    input_files = args.input_files.split(',')
    histograms = args.histograms.split(',')
    if args.draw_options:
        draw_options = args.draw_options.split(',')
    same = args.same
    norm1 = args.norm1
            

    files = []
    plots = []
    canvases = []

    one_to_one = (len(input_files) == len(histograms) and len(input_files)>1)
    n_plots = len(input_files)
    single_file = ( not one_to_one and (len(input_files) < len(histograms)))

    if single_file:
        n_plots = len(histograms)
        # files.append(TFile(input_files[0],"old"))
        files.append(TFile(input_files[0],"old"))
        for i in range(n_plots):
            plots.append(files[0].Get(histograms[i]))
        canvases[0] = TCanvas("c_0","")
        same = True
    else:
        for i in range(n_plots):
            canvases.append(TCanvas("c_"+str(i),""))
            files.append(TFile(input_files[i],"old"))
            if one_to_one:
                plots.append(files[i].Get(histograms[i]))
                
            else:
                
                plots.append(files[i].Get(histograms[0]))
            
    if same:
        canvases[0].cd()
    for i in range(n_plots):
        if norm1:
            plots[i].Scale(1./plots[i].Integral())
        if not same:
            canvases[i].cd()
            plots[i].Draw()
        else:
            plots[i].Draw("same")
            
        
    if same:
        canvases[0].SaveAs("oioi.png")

main()
