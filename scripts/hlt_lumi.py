#!/usr/bin/env python3

# todo: print CSV output

import argparse
import os
from Analysis.Tools.external.toolbox import prompt_command, print_markdown_table, parse_listfile

# default values
NORMTAG = '/cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json'
UNIT = '/pb'
THREADS = 10
DECIMALS = 4

parser = argparse.ArgumentParser(description='Obtain HLT paths luminosities (brilcal)')
parser.add_argument('--json', type=str, help='Path to the Golden JSON file')
parser.add_argument('--triggers', type=parse_listfile, help='List of triggers (comma-separated or in a text file)')
parser.add_argument('--normtag', type=str, default=NORMTAG, help=f'Path to the normtag file (default: {NORMTAG})')
parser.add_argument('--unit', type=str, default=UNIT, help=f'Unit default: {UNIT})')
parser.add_argument('--threads', type=int, default=THREADS, help=f'Number of threads (default: {THREADS})')
parser.add_argument('--decimals', type=int, default=DECIMALS, help=f'Number of decimals in results (default: {DECIMALS})')
parser.add_argument('--output', type=str, help='Output file')

args = parser.parse_args()

if args.output:
    file_format_mapping = {
        ".md": "md",
        ".csv": "csv",
        # Add more mappings as needed
    }
    # Get the file extension (including the dot)
    file_extension = os.path.splitext(args.output)[1]
    # Determine the file format based on the extension
    file_format = file_format_mapping.get(file_extension, None)
else:
    file_format = ""    
    
# brilcalc commands 
cmds = [f"brilcalc lumi -c web --normtag {args.normtag} -u {args.unit} -i {args.json} --hltpath {trigger}" for trigger in args.triggers]

# Run the stuff in parallel
import concurrent.futures

def prompt_command_parallel(cmd):
    return prompt_command(cmd)

# Create a ThreadPoolExecutor with the desired number of threads (adjust as needed, see dedicated parser argument)
if len(args.triggers) <= args.threads:
    print(f"*** Optimised number of threads: {args.threads} -> {len(args.triggers)}")
    args.threads = len(args.triggers)
with concurrent.futures.ThreadPoolExecutor(max_workers=args.threads) as executor:
    results = list(executor.map(prompt_command_parallel, cmds))

# Separate the results into outputs, errors, and codes
outputs, errors, codes = zip(*results)

# Recorded luminosity of the triggers
recorded_lumis = []
triggers_ok = []
for trigger, output, error, code in zip(args.triggers, outputs, errors, codes):
    if code != 0:
        print(f"Command failed with return code {return_code}.")
        continue
    # HLT path w/o the _v*
    trg_name_fmted = trigger if not trigger.endswith('_v*') else trigger[:-3]
    triggers_ok.append(trg_name_fmted)
    recorded_lumis.append(float(next((line for line in output.split('\n') if line.startswith("#Sum recorded")), None).split()[-1]))

# Print table
columns_titles = ["HLT Path",f"Recorded Luminosity [{args.unit}]"]
columns = [triggers_ok,recorded_lumis]

if file_format == 'md' or not args.output:
    print_markdown_table(columns=columns, titles=columns_titles,decimals=args.decimals,output=args.output)
