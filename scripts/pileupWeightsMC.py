#!/usr/bin/env python3

from argparse import ArgumentParser
from ROOT import TH1D
from ROOT import TH1F
from ROOT import TFile

# parsing arguments
parser = ArgumentParser()
parser.add_argument("--data"   , dest="data"   , help="file containing data pileup")
parser.add_argument("--mc"     , dest="mc"     , help="file containing mc pileup")
parser.add_argument("--output" , dest="out"    , help="name ouf output file", default = "PileupWeight.root")

args = parser.parse_args()
if not ( args.data and args.mc ):
   print("nothing to be done")
   quit()

data_pileup = []
data_pileup.append("pileup_nominal")
data_pileup.append("pileup_1up")
data_pileup.append("pileup_2up")
data_pileup.append("pileup_1down")
data_pileup.append("pileup_2down")

weight_title = []
weight_title.append("pileup weight nominal")
weight_title.append("pileup weight +1#sigma")
weight_title.append("pileup weight +2#sigma")
weight_title.append("pileup weight -1#sigma")
weight_title.append("pileup weight -2#sigma")

f_out = TFile(args.out,"recreate")

# MC pileup
f_mc   = TFile(args.mc,"old")
f_mc.cd()
try:
   mc_histogram   = TH1D(f_mc.Get("pileup"))
except TypeError as error:
   mc_histogram   = TH1F(f_mc.Get("pileup"))
mc_histogram.Scale(1./mc_histogram.Integral())

# DATA pileup
f_data = TFile(args.data,"old")
data_histograms = {}
for idx, pu in enumerate(data_pileup):
   key = pu.split("_")[1]
   f_data.cd()
   data_histograms[key] = TH1D(f_data.Get(pu))
   data_histograms[key].Scale(1./data_histograms[key].Integral())
   data_histograms[key].SetTitle(weight_title[idx])
   h_name = 'weight' if key=='nominal' else f'weight_{key}'
   data_histograms[key].SetName(h_name)
   data_histograms[key].Divide(mc_histogram)
   f_out.cd()
   data_histograms[key].Write()

f_out.Close()


