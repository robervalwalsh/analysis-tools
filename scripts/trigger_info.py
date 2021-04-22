#!/usr/bin/env python3
from argparse import ArgumentParser
from argparse import HelpFormatter

import os
from glob import glob
import json
from colors import bcolors

red = bcolors.FAIL
magenta = bcolors.HEADER
bold = bcolors.BOLD
endc = bcolors.ENDC


def list_paths(info):
   for path, path_info in sorted(info.items()):
      print(f'  -> {path}')
   

def list_path_info(info,path):
   print(f'   {magenta}{bold}Path:{endc} {path}')
   l1seeds = info[path]['l1seeds']
#   print('   L1 seeds: ' + ', '.join([s for s in sorted(l1seeds)]))
   print(f'   {magenta}{bold}L1 Seeds:{endc}')
   for seed in sorted(l1seeds):
      print(f'     - {seed}')
   trgobjs = info[path]['trgobjs']
   print(f'   {magenta}{bold}Trigger objects:{endc}')
   for obj in trgobjs:
      print(f'     - {obj}')
   

def list_versions(vdir):
   samples = glob(f'{vdir}/mssmhbb-{year}-v*')
   samples = [os.path.basename(s) for s in samples]
   versions = [int(v.split('-')[-1].split('v')[-1]) for v in samples]
   versions =  sorted(versions)
   return versions
   

# --- main code ---

# parsing arguments
parser = ArgumentParser(prog='trigger_info.py', formatter_class=lambda prog: HelpFormatter(prog,indent_increment=6,max_help_position=80,width=280), description='Print trigger info',add_help=True)
parser.add_argument(dest="year"                                         , help="Year of data taking  (REQUIRED)")
parser.add_argument("-v"  , dest="version"                              , help="Ntuple production version")
parser.add_argument("-p"  , dest="path"                                 , help="HLT path")

args = parser.parse_args()

year = args.year
version = args.version

ntp_dir  = os.getenv('CMSSW_BASE')
ntp_dir += f'/src/Analysis/Tools/data/ntuples'

if not version or version == 'latest' or version == '-1':
   vs = list_versions(ntp_dir)
   if not version:
      print('')
      print(f'{red}{bold}The available versions for {year} are:{endc} {vs}')
      print() 
      os.sys.exit()
   else:
      version = vs[-1]

info_file  = f'{ntp_dir}/mssmhbb-{year}-v{version}/trigger_info.json'

if not os.path.isfile(info_file):
   print()
   print(f'{red}{bold} File with trigger info does not exist! {endc}')
   print(f'{info_file}')
   print()
   os.sys.exit()

print('')
print('--------------------------------------------------------')
print(f' Trigger information for {year} ntuples v{version}')
print('--------------------------------------------------------')

with open(info_file) as f:
   info_data = json.load(f)

if not args.path or args.path=='':
   print(f'{magenta}{bold}Listing all paths available:{endc}')
   list_paths(info_data)
else:
   list_path_info(info_data,args.path)
   
print('--------------------------------------------------------')
print('')
