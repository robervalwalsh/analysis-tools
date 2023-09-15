#!/usr/bin/env python3
'''
## Get the lumi sections of a trigger with a certain L1 seed always unprescaled

### brilcalc command

`brilcalc` provides the L1 prescale information of an HLT path in a run-by-run basis (see example below). Therefore, one must prepare the data by running on each run first.

#### brilcalc example
```bash
brilcalc trg -c web --prescale --hltpath "HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33_v*" -r 304507
```

It returns the prescale info of the HLT path and the L1 seeds
```
+--------+-------+----------+-------------+----------------------------------------------------------+-------+-----------------------------------------+
| run    | cmsls | prescidx | totprescval | hltpath/prescval                                         | logic | l1bit/prescval                          |
+--------+-------+----------+-------------+----------------------------------------------------------+-------+-----------------------------------------+
| 304507 | 1     | 1        | 1.00        | HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33_v5/1 | OR    | L1_DoubleJet100er2p3_dEta_Max1p6/0.00   |
|        |       |          |             |                                                          |       | L1_DoubleJet112er2p3_dEta_Max1p6/1.00   |
| 304507 | 50    | 2        | 1.00        | HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33_v5/1 | OR    | L1_DoubleJet112er2p3_dEta_Max1p6/1.00   |
|        |       |          |             |                                                          |       | L1_DoubleJet100er2p3_dEta_Max1p6/1.00   |
| 304507 | 76    | 3        | 1.00        | HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33_v5/1 | OR    | L1_DoubleJet100er2p3_dEta_Max1p6/1.00   |
|        |       |          |             |                                                          |       | L1_DoubleJet112er2p3_dEta_Max1p6/1.00   |
| 304507 | 172   | 4        | 1.00        | HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33_v5/1 | OR    | L1_DoubleJet112er2p3_dEta_Max1p6/1.00   |
|        |       |          |             |                                                          |       | L1_DoubleJet100er2p3_dEta_Max1p6/1.00   |
+--------+-------+----------+-------------+----------------------------------------------------------+-------+-----------------------------------------+
```
where `cmsls` is the first LS with that prescale

The option `--output-style "csv"` is more suitable for scripting, i.e.
```bash
brilcalc trg -c web --prescale --hltpath "HLT_DoublePFJets100MaxDeta1p6_DoubleCaloBTagCSV_p33_v*" -r 304507 --output-style "csv"
```

'''

import argparse
import json
from collections import defaultdict,OrderedDict
import os
from pathlib import Path
from Analysis.Tools.toolbox import prompt_command, file_lines, print_markdown_table
from FWCore.PythonUtilities.LumiList import LumiList

def create_lumi_json(psidxjson, lumis_dict):
    json_dict = defaultdict(list)
    for run, lumiranges in psidxjson.items():
        for lr in lumiranges:
            for lumi in lumis_dict[run]:
                if lumi in lr:
                    json_dict[run].append(lr)

    return json_dict

# default values
THREADS = 20

parser = argparse.ArgumentParser(description='Obtain certified LS when the L1 seed is active/inactive (brilcal)')
parser.add_argument('--json', type=str, help='Path to the Golden JSON file')
parser.add_argument('--hlt', type=str, help='HLT Path')
parser.add_argument('--l1', type=str, help='L1 Seed')
parser.add_argument('--threads', type=int, default=THREADS, help=f'Number of threads (default: {THREADS})')

args = parser.parse_args()

# Output file name = input json with L1Active suffix
output_name = Path(args.json).stem
output_ext = Path(args.json).suffix
output_active   = f'{output_name}_L1Active{output_ext}'
output_inactive = f'{output_name}_L1Inactive{output_ext}'

# Get runs list from the JSON file
with open(args.json, "r") as json_file:
    certified_data = json.load(json_file)
certified_data = OrderedDict(sorted(certified_data.items()))
runs = list(certified_data.keys())

cmds = [f"brilcalc trg -c web --prescale --hltpath {args.hlt} -r {run} --output-style csv" for run in runs]

# Run the stuff in parallel
import concurrent.futures

def prompt_command_parallel(cmd):
    return prompt_command(cmd)

# Create a ThreadPoolExecutor with the desired number of threads (adjust as needed)
num_threads = args.threads  # Number of threads for parallel execution
if len(runs) <= args.threads:
    print(f"*** Optimised number of threads: {args.threads} -> {len(runs)}")
    num_threads = len(runs)
with concurrent.futures.ThreadPoolExecutor(max_workers=num_threads) as executor:
    results = list(executor.map(prompt_command_parallel, cmds))

# Separate the results into outputs, errors, and codes
outputs, errors, codes = zip(*results)

# process outputs
outputs_list = []
outputs_list.extend(line.strip() for output in outputs for line in output.split('\n') if not line.startswith('#') and line.strip() != '' and 'None' not in line)
outputs_list = [output.split(',') for output in outputs_list]

# Get the lumi section indices for 0 and non-0 prescales of the l1seed
psidxlumis = defaultdict(list)  # each element of the list is the first LS of the prescale column
lumis_ps0  = defaultdict(list)   # inactive 
lumis_ps1  = defaultdict(list)   # active

for output in outputs_list:
    run, lumi_section, _, _, _, logics, l1s = output
    lumi_section = int(lumi_section)
    psidxlumis[run].append(lumi_section)

    for l1 in l1s.strip().split(" "):
        l1_name, l1_ps = l1.strip().split("/")
        l1_ps = float(l1.strip().split("/")[1])
        if l1_name == args.l1:
            if float(l1_ps) > 0.:
                lumis_ps1[run].append(lumi_section)
            else:
                lumis_ps0[run].append(lumi_section)
psidxlumis = OrderedDict(sorted(psidxlumis.items()))

# This creates a json style LS lists from psidxlumis and the GoldenJSON
# e.g. psidxlumis['306423'] = [1, 111, 237] and last LS in GoldenJson is 333
# psidxjson = [[1, 110], [111, 236], [237, 333]]
# each lumi section range correspond to a prescale column
psidxjson = defaultdict(list)
for run, lumis in psidxlumis.items():
    lumis.sort()
    for i,lumi in enumerate(lumis):
        if i < len(lumis)-1:
            psidxjson[run].append([lumis[i],lumis[i+1]-1])
        else:
            psidxjson[run].append([lumis[i], certified_data[run][-1][1]])
psidxjson = OrderedDict(sorted(psidxjson.items()))


# creating a json like dict for lumi ranges where L1 is inactive
json0 = create_lumi_json(psidxjson, lumis_ps0)
# creating a json like dict for lumi ranges where L1 is active
json1 = create_lumi_json(psidxjson, lumis_ps1)

# convert to LumiList object
lumi_json0 = LumiList (compactList = json0)
lumi_json1 = LumiList (compactList = json1)
lumi_cert = LumiList (compactList = certified_data)

# Certified lumis where L1 is active
certified_active   = lumi_cert - lumi_json0
certified_active.writeJSON(output_active)
# Certified lumis where L1 is inactive
certified_inactive = lumi_cert - lumi_json1
certified_inactive.writeJSON(output_inactive)
