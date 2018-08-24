#!/usr/bin/env python
# -*- coding:utf-8 -*-


from settings_parallelization import correction_level_bkg, correction_level_signal, \
    mass_points_signal, bkg_files, split_list, mkdir_p, condor_script_executable, \
    tmp_dir, condor_submit, open_and_create_dir
from subprocess import Popen, STDOUT, PIPE
from time import sleep
from os.path import join as ojoin
from os import chmod, devnull

import sys

DEVNULL = open(devnull, "wb")
process_list = list()
base_out_dir = "/nfs/dust/cms/user/zorattif/output/"
SPLITTING = 10


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
            output_file = ojoin(output_dir, output_file)
            for lista, i in zip(split_list(params['filenames'], SPLITTING), range(0, 10000)):
                tmp_output_file = "_".join([output_file, params["era"], str(i)]) + ".root"
                veralista = [ojoin(params['basedir'], l) for l in lista]
                print(tmp_output_file)
                condor_submit(
                    process_list,
                    executable,
                    [tmp_output_file, str(bins), str(highx)] + veralista,
                    "_".join([executable, params["era"], str(i)]), 600, 1000)
    bash_filename = ojoin(tmp_dir, "run_condor" + type_of + ".sh")
    bash_file = open_and_create_dir(bash_filename)
    bash_file.write("#!/bin/bash\n")
    for p in process_list:
        bash_file.write(" ".join([condor_script_executable,
                                  p['filename'], p['runtime'], p['memory']]) + "\n")
    chmod(bash_filename, 0755)
    return bash_filename
    
    
if sys.argv[1] == "bkg" and len(sys.argv) == 2:
    output_dir = ojoin(base_out_dir, "raw_files/bkg/distribution_corrections_before_jets_3_FSR_pt_20_deltar_08/medium_wp")
    mkdir_p(output_dir)
    proc = Popen([fill_list("bkg", output_dir), ], stdout=PIPE)
    print(proc.stdout.read())
    proc.wait()
elif sys.argv[1] == "bkg" and sys.argv[2] is not None and sys.argv[2] == "merge":
    output_dir = ojoin(base_out_dir, "raw_files/bkg/distribution_corrections_before_jets_3_FSR_pt_20_deltar_08/medium_wp")
    for params in bkg_files:
        for cl in correction_level_bkg:
            output_file = "_".join([params['mass'], params['era'], cl[0], cl[1]])
            output_file = ojoin(output_dir, output_file)
            print(output_file)
            to_merge = list()
            for lista, i in zip(split_list(params['filenames'], SPLITTING), range(0, 10000)):
                to_merge.append("_".join([output_file, params["era"], str(i)]) + ".root")
            proc = Popen(["hadd", "-f", output_file + ".root"] + to_merge, stdout=PIPE)
            print(proc.stdout.read())
            proc.wait()
            # Here we do rm if all is ok
            
elif sys.argv[1] == "mc":
    output_dir = ojoin(base_out_dir, "raw_files/MC/distribution_corrections_before_jets_3_FSR_pt_20_deltar_08/medium_wp")
    mkdir_p(output_dir)
    for params in mass_points_signal:
        for cl in correction_level_signal:
            executable = "_".join(["mc", cl[0], cl[1]])
            output_file = "_".join([params['mass'], cl[0], cl[1]]) + ".root"
            output_file = ojoin(output_dir, output_file)
            veralista = [ojoin(params['basedir'], l) for l in params['filenames']]
            condor_submit(
                process_list,
                executable, [output_file, str(params['bins']), str(params['highx'])] + veralista,
                "_".join([params['mass'], cl[0], cl[1]]), 600, 1000)
    bash_filename = "_tmp/run_condor_mc.sh"
    bash_file = open(bash_filename, "w")
    bash_file.write("#!/bin/bash\n")
    for p in process_list:
        bash_file.write(" ".join([condor_script_executable,
                                  p['filename'], p['runtime'], p['memory']]) + "\n")
    bash_file.close()
    chmod(bash_filename, 0755)
    proc = Popen([bash_filename, ], stdout=PIPE)
    print(proc.stdout.read())
    proc.wait()

elif sys.argv[1] == 'sig' and len(sys.argv) == 2:
    output_dir = ojoin(base_out_dir, "raw_files/signal/distribution_corrections_before_jets_3_FSR_pt_20_deltar_08/medium_wp")
    mkdir_p(output_dir)
    proc = Popen([fill_list("sig", output_dir), ], stdout=PIPE)
    print(proc.stdout.read())
    proc.wait()
    
elif sys.argv[1] == "sig" and len(sys.argv) == 3 and sys.argv[2] == "merge":
    output_dir = ojoin(base_out_dir, "raw_files/signal/distribution_corrections_before_jets_3_FSR_pt_20_deltar_08/medium_wp")
    for params in bkg_files:
        for cl in correction_level_bkg:
            output_file = "_".join(["sig", params['era'], cl[0], cl[1]])
            output_file = ojoin(output_dir, output_file)
            to_merge = list()
            for lista, i in zip(split_list(params['filenames'], SPLITTING), range(0, 10000)):
                to_merge.append("_".join([output_file, params["era"], str(i)]) + ".root")
            proc = Popen(["hadd", "-f", output_file + ".root"] + to_merge, stdout=PIPE)
            print(proc.stdout.read())
            proc.wait()
