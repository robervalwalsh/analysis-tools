#!/usr/bin/env python3

import argparse
import os
# Run the stuff in parallel
import concurrent.futures
import ROOT as r

from Analysis.Tools.toolbox import prompt_command, parse_listfile, directory_from_filename

# default values
NORMTAG = '/cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json'
YEAR = '2017'
MINBIASXSEC = 69200.
MINBIASXSECERR = 3200.
THREADS = 10
PERIOD = 'Run2'
MAX_BIN = 100
NUM_BINS = 100

parser = argparse.ArgumentParser(description='Obtain pileup weights for HLT paths')
parser.add_argument('--json', type=str, required=True, help='Path to the Golden JSON file')
parser.add_argument('--triggers', type=parse_listfile, required=True, help='List of triggers (comma-separated or in a text file)')
parser.add_argument('--step', type=int, help=f'step to be executed')
parser.add_argument('--normtag', type=str, default=NORMTAG, help=f'Path to the normtag file (default: {NORMTAG})')
parser.add_argument('--year', type=str, default=YEAR, help=f'Year of data taking for the pileup (default: {YEAR})')
parser.add_argument('--xsec', type=float, default=MINBIASXSEC, help=f'Minimum bias cross section (default: {MINBIASXSEC})')
parser.add_argument('--xsec_err', type=float, default=MINBIASXSECERR, help=f'Uncertainty on minimum bias cross section (default: {MINBIASXSECERR})')
parser.add_argument('--period', type=str, default=PERIOD, help=f'Run period (default: {PERIOD})')
parser.add_argument('--max_bin', type=int, default=MAX_BIN, help=f'Maximum pileup bin (default: {MAX_BIN})')
parser.add_argument('--num_bins', type=int, default=NUM_BINS, help=f'Number of pileup bins (default: {NUM_BINS})')
parser.add_argument('--threads', type=int, default=THREADS, help=f'Number of threads (default: {THREADS})')
parser.add_argument("--keep", action="store_true", help="Keep the root files for each xsec variation, otherwise keep only files with merged histograms")
parser.add_argument('--reference', type=str, help=f'Reference trigger for pileup weight')
parser.add_argument('--label', type=str, help=f'Label for the weight file')


epilog = """
More information in the links\n
https://twiki.cern.ch/twiki/bin/view/CMS/BrilcalcQuickStart\n
https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJSONFileforData#Pileup_for_specific_HLT_paths\n
https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJSONFileforData#Location_of_central_pileup_JSON - for the pileup_latest.txt\n
"""
parser.epilog = epilog

args = parser.parse_args()

if args.period != 'Run2':
    print('Not Run2 analysis. Look for the pileup file path at')
    print("https://twiki.cern.ch/twiki/bin/viewauth/CMS/PileupJSONFileforData#Location_of_central_pileup_JSON")
    print('and modify this script accordingly.')
    exit()

output_directory = directory_from_filename(args.json)
pileup = f'/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions{args.year[2:]}/13TeV/PileUp/UltraLegacy/pileup_latest.txt'
xsection = {}
for sigma in range(-2, 3):
    if sigma < 0:
        variation = f'{abs(sigma)}down'
    elif sigma > 0:
        variation = f'{abs(sigma)}up'
    else:
        variation = 'nominal'
    xsection[variation] = args.xsec + sigma*args.xsec_err

print(f'Pileup path: {pileup}')

def merge_histograms():
    # Loop through each trigger
    for trigger in args.triggers:
        # Create an output root file for the current trigger
        output_root_file = r.TFile(f'{output_directory}/pileupCalc_{trigger[:-3]}_merged.root', 'RECREATE')
     

        # Loop through each xsec variation
        for xsec_variation, xsec_value in xsection.items():
            input_file_path = f'{output_directory}/pileupCalc_{trigger[:-3]}_{xsec_variation}.root'
            input_root_file = r.TFile(input_file_path, 'READ')

            if input_root_file.IsOpen():
                # Get the histogram named 'pileup_{xsec[0]}' from the input file
                histogram = input_root_file.Get(f'pileup_{xsec_variation}')
                histogram.SetTitle(f'{histogram.GetTitle()} : x-section={int(xsec_value)}')

                if histogram:
                    # Normalize the histograms to unit (integral equal to 1)
                    if histogram.Integral() > 0:
                        histogram.Scale(1.0 / histogram.Integral())
                    # Write the histogram to the output file
                    output_root_file.cd()
                    histogram.Write(f'pileup_{xsec_variation}')

                # Close the input file
                input_root_file.Close()

            if not args.keep:
                # Delete the input file
                os.remove(input_file_path)

        # Close the output file for the current trigger
        output_root_file.Close()


def calculate_weights():
    # Open the output root file for the reference trigger
    reference_file = r.TFile(f'{output_directory}/pileupCalc_{args.reference[:-3]}_merged.root', 'READ')
    for trigger in args.triggers:
        # Create an output root file for the current trigger
        weight_label = f'_{args.label}' if args.label else ""
        weight_file_name = f'{output_directory}/PileupWeight_{trigger[:-3]}{weight_label}.root'
        weight_output_file = r.TFile(weight_file_name, 'RECREATE')
        weight_output_file.cd()
        # Open the output root file for the current trigger
        current_file = r.TFile(f'{output_directory}/pileupCalc_{trigger[:-3]}_merged.root', 'READ')
        # Loop through each xsec variation
        for xsec_variation, xsec_value in xsection.items():
            # Check if the reference file is open
            if reference_file.IsOpen():
                # Get the histogram for the reference trigger
                reference_histogram = reference_file.Get(f'pileup_{xsec_variation}')
                # Check if the current file is open
                if current_file.IsOpen():
                    # Get the histogram for the current trigger
                    current_histogram = current_file.Get(f'pileup_{xsec_variation}')
                    if reference_histogram and current_histogram:
                        # Create the weight histogram by dividing reference by current
                        weight_histogram = reference_histogram.Clone()
                        weight_histogram.Divide(current_histogram)
                        # Set the name of the weight histogram to include xsec_variation
                        if xsec_variation == "nominal":
                            weight_histogram.SetName(f'weight')
                        else:
                            weight_histogram.SetName(f'weight_{xsec_variation}')
                        weight_histogram.SetTitle(f'pilup weight {xsec_variation} : xsection = {int(xsec_value)}')
                        # Save the weight histogram to the weight output file
                        weight_output_file.cd()
                        weight_histogram.Write()

        # Close the current file
        current_file.Close()
        # Close the weight output file for the current trigger
        weight_output_file.Close()        
    
    # Close the reference file
    reference_file.Close()


# Define a function to execute a step of commands
def execute_step(step,xsec=('zero',0)):
    # Define cmds based on the step
    cmds = {
        1: [f"brilcalc lumi -c web --byls --normtag {args.normtag} -i {args.json} --hltpath {trigger} -o {output_directory}/brilcalc_{trigger[:-3]}.csv" for trigger in args.triggers],
        2: [f"pileupReCalc_HLTpaths.py -i {output_directory}/brilcalc_{trigger[:-3]}.csv --inputLumiJSON {pileup} --runperiod {args.period} -o {output_directory}/pileupReCalc_{trigger[:-3]}.txt" for trigger in args.triggers],
        3: [f"pileupCalc.py -i {args.json} --inputLumiJSON {output_directory}/pileupReCalc_{trigger[:-3]}.txt --calcMode true --minBiasXsec {xsec[1]} --maxPileupBin {args.max_bin}  --numPileupBins {args.num_bins} --pileupHistName pileup_{xsec[0]}  {output_directory}/pileupCalc_{trigger[:-3]}_{xsec[0]}.root" for trigger in args.triggers]
    }

    # Execute the commands for the given step in parallel
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.threads) as executor:
        results = list(executor.map(prompt_command_parallel, cmds[step]))

    return results


def prompt_command_parallel(cmd):
    return prompt_command(cmd)

# Create a ThreadPoolExecutor with the desired number of threads (adjust as needed, see dedicated parser argument)
if len(args.triggers) <= args.threads:
    print(f"*** Optimised number of threads: {args.threads} -> {len(args.triggers)}")
    args.threads = len(args.triggers)

# Determine which steps to execute
if args.step:
    # If a specific step is specified, execute only that step
    steps_to_execute = [args.step]
else:
    # If no specific step is specified, execute steps 1 to 4
    steps_to_execute = range(1, 6)

# Loop through the steps to execute
for step in steps_to_execute:
    # Print a message indicating the current step
    print(f'Executing step {step}...')

    # Execute the appropriate logic for each step
    if 0 < step < 3:
        # Execute step 1 and 2 logic
        execute_step(step)
    elif step == 3:
        # Execute step 3 logic for each xsec variation
        for variation, xsec_value in xsection.items():
            execute_step(step, (variation, xsec_value))
    elif step == 4:
        # Execute step 4 logic to merge histograms
        merge_histograms()
    elif step == 5:
        if args.reference:
            calculate_weights()
        else:
            print("No reference trigger specified")
    else:
        # Handle invalid steps (if any)
        pass


