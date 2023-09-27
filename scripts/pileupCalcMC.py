#!/usr/bin/env python3

# pileupCalcMC.py 

from argparse import ArgumentParser
from importlib import import_module
from ROOT import TH1D
from ROOT import TFile

OUTPUT = 'MyMCPileupHistogram.root'
NBINS = 100
MINBINS = 0
MAXBINS = 100

# parsing arguments
parser = ArgumentParser()
parser.add_argument("pileup"                                                    , help="Python mixing module, e.g. SimGeneral.MixingModule.mix_2017_25ns_UltraLegacy_PoissonOOTPU_cfi")
parser.add_argument("--n"    , dest="nbins" , type=int, default=NBINS           , help=f"Number of bins (default: {NBINS})")
parser.add_argument("--min"  , dest="min"   , type=int, default=MINBINS         , help=f"Min pileup     (default: {MINBINS})")
parser.add_argument("--max"  , dest="max"   , type=int, default=MAXBINS         , help=f"Max pileup     (default: {MAXBINS})")
parser.add_argument("--out"  , dest="out"   , default=f'{OUTPUT}'               , help=f"Output file    (default: {OUTPUT})")
args = parser.parse_args()
if not args.pileup:
   print("nothing to be done") 
   quit()
   
puconfig = import_module(args.pileup)

bins = puconfig.mix.input.nbPileupEvents.probFunctionVariable
values = puconfig.mix.input.nbPileupEvents.probValue


hPU = TH1D("pileup",  "true pileup", args.nbins, args.min, args.max)

for i in range(1, args.nbins+1):
   try: 
      hPU.SetBinContent(i,values[bins.index(hPU.GetBinLowEdge(i))])
   except ValueError as error:
      hPU.SetBinContent(i,0)
      
fout = TFile(args.out,"recreate")
hPU.Write()
fout.Close()

