#!/usr/bin/env python
# -*- coding:utf-8 -*-


from settings_parallelization import correction_level_bkg, correction_level_signal, \
    mass_points_signal, bkg_files, split_list
from subprocess import Popen, STDOUT
from time import sleep

import os
import sys

DEVNULL = open(os.devnull, "wb")

process_list = list()

if sys.argv[1] == "bkg":
    output_dir = "output/hists/bkg/third"
    bins = 100
    highx = 800
    
    for params in bkg_files:
        for cl in correction_level_bkg:
            executable = "bkg_" + "_".join(cl)
            output_file = "_".join([params['mass'], params['era'], cl[0], cl[1]])
            output_file = os.path.join(output_dir, output_file)
            for lista, i in zip(split_list(params['filenames'], 8), range(0, 10000)):
                tmp_output_file = output_file + "_" + str(i) + ".root"
                veralista = [os.path.join(params['basedir'], l) for l in lista ]
                process_list.append(Popen([executable, tmp_output_file,
                                           str(bins), str(highx)] + veralista,
                                          stdout=DEVNULL, stderr=STDOUT))
            print("Now I wait a bit")
            sleep(20)
    print("All process launched.")
    appo = [p.wait() for p in process_list]
    print("All process exited. Now i merge the result")
    for params in bkg_files:
        for cl in correction_level_bkg:
            output_file = "_".join([params['mass'], params['era'], cl[0], cl[1]])
            output_file = os.path.join(output_dir, output_file)
            to_merge = list()
            for i in range(0, len(split_list(params['filenames'], 10))):
                to_merge.append(output_file + "_" + str(i) + ".root")
            output_file += ".root"
            Popen(["hadd", output_file] + to_merge)
            Popen(["rm"] + to_merge)
    print("Merged")
            
elif sys.argv[1] == "signal":
    output_dir = "output/hists/jets/last"
    for params in mass_points_signal:
        for cl in correction_level_signal:
            executable = "_".join(["sig", cl[0], cl[1]])
            output_file = "_".join([params['mass'], cl[0], cl[1]]) + ".root"
            output_file = os.path.join(output_dir, output_file)
            veralista = [os.path.join(params['basedir'], l) for l in params['filenames']]
            process_list.append(
                Popen([executable, output_file, str(params['bins']),
                       str(params['highx'])] + veralista,
                      stdout=DEVNULL, stderr=STDOUT))
    print("All process launched.")
    appo = [p.wait() for p in process_list]
    print("All process exited.")

