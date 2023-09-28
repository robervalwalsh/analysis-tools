#!/usr/bin/env python3

# pileupCalcMC.py 

# Summary:
# This script calculates pileup weights for Monte Carlo (MC) samples based on a specified Python mixing module or data file.
# It creates histograms for both MC and data pileup, computes weights, and saves them to an output ROOT file.


from argparse import ArgumentParser
from importlib import import_module
from ROOT import TH1D
from ROOT import TFile

# Default constants for parameters
OUTPUT = 'PileupWeightMC.root'
NBINS = 100
MINBINS = 0
MAXBINS = 100

# parsing arguments
parser = ArgumentParser()
parser.add_argument("pileup"                                                    , help="Python mixing module, e.g. SimGeneral.MixingModule.mix_2017_25ns_UltraLegacy_PoissonOOTPU_cfi")
# TODO: if not a python module, rather a root file with the histogram, require data for the weights
parser.add_argument("--n"    , dest="nbins" , type=int, default=NBINS           , help=f"Number of bins (default: {NBINS})")
parser.add_argument("--min"  , dest="min"   , type=int, default=MINBINS         , help=f"Min pileup     (default: {MINBINS})")
parser.add_argument("--max"  , dest="max"   , type=int, default=MAXBINS         , help=f"Max pileup     (default: {MAXBINS})")
parser.add_argument("--out"  , dest="out"   , default=f'{OUTPUT}'               , help=f"Output file    (default: {OUTPUT})")
parser.add_argument("--data" , dest="data"  , help=f"File containing data pileup to produce weights (optional)")

args = parser.parse_args()
if not args.pileup:
   print("nothing to be done") 
   quit()
   
puconfig = import_module(args.pileup)

bins = puconfig.mix.input.nbPileupEvents.probFunctionVariable
values = puconfig.mix.input.nbPileupEvents.probValue


mc_histogram = TH1D("pileup_true",  "true pileup", args.nbins, args.min, args.max)

for bin in range(1, args.nbins+1):
   try: 
      mc_histogram.SetBinContent(bin,values[bins.index(mc_histogram.GetBinLowEdge(bin))])
      mc_histogram.SetBinError(bin,0)
   except ValueError as error:
      mc_histogram.SetBinContent(bin,0)
      mc_histogram.SetBinError(bin,0)

mc_histogram.Scale(1./mc_histogram.Integral())      

f_out = TFile(args.out,"recreate")
mc_histogram.Write()

if args.data:
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

   # DATA pileup
   f_data = TFile(args.data,"old")
   data_histograms = {}
   for idx, pu in enumerate(data_pileup):
      key = pu.split("_")[1]
      f_data.cd()
      data_histograms[key] = TH1D(f_data.Get(pu))
      for bin in range(1, data_histograms[key].GetNbinsX() + 1):
         data_histograms[key].SetBinError(bin, 0.0)
      f_out.cd()
      data_histograms[key].Write()
      data_histograms[key].Scale(1./data_histograms[key].Integral())
      data_histograms[key].SetTitle(weight_title[idx])
      h_name = 'weight' if key=='nominal' else f'weight_{key}'
      data_histograms[key].SetName(h_name)
      data_histograms[key].Divide(mc_histogram)

# this could have been done in the previous loop, but
# the histograms are better organised in the output file
# if done this way
   f_out.cd()
   for idx, pu in enumerate(data_pileup):
      key = pu.split("_")[1]
      data_histograms[key].Write()

f_out.Close()

