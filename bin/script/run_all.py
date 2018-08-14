#!/usr/bin/env python
# -*- coding:utf-8 -*-


from settings_parallelization import correction_level_bkg, correction_level_signal, \
    mass_points_signal, bkg_files, split_list
from subprocess import Popen, STDOUT, PIPE
from time import sleep
from jinja2 import FileSystemLoader, Environment
template_loader = FileSystemLoader(searchpath='./script/templates')
template_env = Environment(loader=template_loader)
template_exec = template_env.get_template("exe.j2")
# template_submit = template_env.get_template("htc_sub.j2")

import os
import sys

DEVNULL = open(os.devnull, "wb")
condor_script_executable = "../scripts/htc_sub.sh"
process_list = list()


def condor_submit(executable, args, name, runtime=1800, memory=1000):
    out_text = template_exec.render(
        executable=executable, arg_list=args)
    filename = name + ".sh"
    filename = os.path.join("_tmp/condor/", filename)
    fileout = open(filename, "w")
    fileout.write(out_text)
    fileout.close()
    os.chmod(filename, 0755)
    process_list.append(
        {'filename': filename, 'runtime': str(runtime),
         'memory': str(memory)})



SPLITTING = 7
    

def fill_list(type_of, output_dir):
    bins = 100
    highx = 800
    name = ""
    if type_of == "bkg":
        name = "bkg"
    elif type_of == "sig":
        name = "sig"
    else:
        raise RuntimeError("Function called with bad argument")
    for params in bkg_files:
        for cl in correction_level_bkg:
            executable = name + "_" + "_".join(cl)
            output_file = "_".join([name, params['era'], cl[0], cl[1]])
            output_file = os.path.join(output_dir, output_file)
            for lista, i in zip(split_list(params['filenames'], SPLITTING), range(0, 10000)):
                tmp_output_file = "_".join([output_file, params["era"], str(i)]) + ".root"
                veralista = [os.path.join(params['basedir'], l) for l in lista]
                print(tmp_output_file)
                condor_submit(
                    executable,
                    [tmp_output_file, str(bins), str(highx)] + veralista,
                    "_".join([executable, params["era"], str(i)]), 1800, 1000)
    bash_filename = "_tmp/run_condor.sh"
    bash_file = open(bash_filename, "w")
    bash_file.write("#!/bin/bash\n")
    for p in process_list:
        bash_file.write(" ".join([condor_script_executable,
                                  p['filename'], p['runtime'], p['memory']]) + "\n")
    os.chmod(bash_filename, 0755)
    return bash_filename
    
    
if sys.argv[1] == "bkg" and len(sys.argv) == 2:
    output_dir = "output/hists/bkg/fourth"
    proc = Popen([fill_list("bkg", output_dir), ], stdout=PIPE)
    proc.wait()
elif sys.argv[1] == "bkg" and sys.argv[2] is not None and sys.argv[2] == "merge":
    output_dir = "output/hists/bkg/fourth"
    for params in bkg_files:
        for cl in correction_level_bkg:
            output_file = "_".join([params['mass'], params['era'], cl[0], cl[1]])
            output_file = os.path.join(output_dir, output_file)
            to_merge = list()
            for lista, i in zip(split_list(params['filenames'], SPLITTING), range(0, 10000)):
                to_merge.append("_".join([output_file, params["era"], str(i)]) + ".root")
            process = Popen(["hadd", "-f", output_file + ".root"] + to_merge, stdout=PIPE)
            process.wait()
            # Here we do rm if all is ok
            
elif sys.argv[1] == "mc":
    output_dir = "output/hists/jets/last"
    for params in mass_points_signal:
        for cl in correction_level_signal:
            executable = "_".join(["mc", cl[0], cl[1]])
            output_file = "_".join([params['mass'], cl[0], cl[1]]) + ".root"
            output_file = os.path.join(output_dir, output_file)
            veralista = [os.path.join(params['basedir'], l) for l in params['filenames']]
            condor_submit(
                executable, [output_file, str(params['bins']), str(params['highx'])] + veralista,
                "_".join([params['mass'], cl[0], cl[1]]), 3600, 1000)
    bash_filename = "_tmp/run_condor.sh"
    bash_file = open(bash_filename, "w")
    bash_file.write("#!/bin/bash\n")
    for p in process_list:
        bash_file.write(" ".join([condor_script_executable,
                                  p['filename'], p['runtime'], p['memory']]) + "\n")
    os.chmod(bash_filename, 0755)
    proc = Popen([bash_filename, ], stdout=PIPE)
    proc.wait()

elif sys.argv[1] == 'sig' and len(sys.argv) == 2:
    output_dir = "output/hists/sig/first"
    proc = Popen([fill_list("sig", output_dir), ], stdout=PIPE)
    proc.wait()
    
elif sys.argv[1] == "sig" and len(sys.argv) == 3 and sys.argv[2] == "merge":
    output_dir = "output/hists/sig/first"
    for params in bkg_files:
        for cl in correction_level_bkg:
            output_file = "_".join(["sig", params['era'], cl[0], cl[1]])
            output_file = os.path.join(output_dir, output_file)
            to_merge = list()
            for lista, i in zip(split_list(params['filenames'], SPLITTING), range(0, 10000)):
                to_merge.append("_".join([output_file, params["era"], str(i)]) + ".root")
            process = Popen(["hadd", "-f", output_file + ".root"] + to_merge, stdout=PIPE)
            process.wait()
