#!/usr/bin/env python

# pileupCalcMC.py 

from argparse import ArgumentParser
from ROOT import TH1D
from ROOT import TFile

# parsing arguments
parser = ArgumentParser()
parser.add_argument("--data" , dest="data"   , help="file containing data pileup")
parser.add_argument("--mc"   , dest="mc"     , help="file containing mc pileup")
parser.add_argument("--up"   , dest="up"     , help="file containing data pileup up var")
parser.add_argument("--down" , dest="down"   , help="file containing data pileup down var")
args = parser.parse_args()
if not ( args.data and args.mc ):
   print "nothing to be done" 
   quit()

f_data = TFile(args.data,"old")
h_data = TH1D(f_data.Get("pileup"))
h_data.Scale(1./h_data.Integral())

f_mc   = TFile(args.mc,"old")
h_mc   = TH1D(f_mc.Get("pileup"))
h_mc.Scale(1./h_mc.Integral())

h_weight = TH1D(h_data.Clone("weight"));
h_weight.Divide(h_mc);

if args.up:
   h_wup = TH1D(h_data.Clone("weight_up"));
   f_up = TFile(args.up,"old")
   h_up = TH1D(f_up.Get("pileup"))
   h_up.Scale(1./h_up.Integral())
   h_wup.Divide(h_up)

if args.down:
   h_wdown = TH1D(h_data.Clone("weight_down"));
   f_down = TFile(args.down,"old")
   h_down = TH1D(f_down.Get("pileup"))
   h_down.Scale(1./h_down.Integral())
   h_wdown.Divide(h_down)


fout = TFile("PileUpWeight.root","recreate")
h_weight.Write()
if args.up:
   h_wup.Write()
if args.down:
   h_wdown.Write()
   
fout.Close()

