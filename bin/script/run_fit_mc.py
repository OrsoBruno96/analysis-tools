#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from settings_parallelization import correction_level_signal, open_and_create_dir, \
    name_of_lep
from os.path import join as ojoin
from os import chmod

base_directory = "/nfs/dust/cms/user/zorattif/output"
specific_directory = "no_reranking/medium_wp"
directory_mc = ojoin(base_directory, ojoin("raw_files/MC" , specific_directory))
output_directory = ojoin(ojoin(ojoin(base_directory, "plots"), specific_directory), "fit")

script_filename = "../_tmp/script/run_fits_mc.sh"
script_file = open(script_filename, "w")


pars_bukin_350 = [350, 310., 60., 0.01, 0.01, -0.2]
pars_bukin_1200 = [80, 1100, 300, 0.01, 0.01, 0.01]

mass_points = {
    "350": {
        'min': 80,
        'max': 550,
        'bins': 60,
        'pars_filename': "../_tmp/fit/init_pars_bukin_350.txt",
        'pars': pars_bukin_350,
        'function': "bukin"
    },
    "1200": {
        'min': 100,
        'max': 1400,
        'bins': 60,
        'pars_filename': "../_tmp/fit/init_pars_bukin_1200.txt",
        'pars': [80, 1100, 300, 0.01, 0.01, 0.01, 400, 50],
        'function': "bukin_modified"
    }
}

lep = [True, False]

for c in correction_level_signal:
    c = "_".join(c)
    for m, params in mass_points.iteritems():
        for l in lep:
            input_filename = ojoin(directory_mc, "_".join([m, c]) + ".root")
            out_basename = "_".join(["fit", "mc", m, c, name_of_lep(l)])
            out_basename = ojoin(output_directory, out_basename)
            out_pars_file = open_and_create_dir(params['pars_filename'])
            for f in params['pars']:
                out_pars_file.write(str(f) + "\n")
            out_pars_file.close()
            if l:
                extra = list()
            else:
                extra = ["--full-hadronic", ]
            command = [
                "FitBackground",
                "--input", input_filename,
                "--output", out_basename + ".txt",
                "--print", out_basename + ".pdf",
                "--min-x", str(params["min"]),
                "--max-x", str(params["max"]),
                "--bins", str(params["bins"]),
                "--initial-pars", params['pars_filename'],
            ] + extra + [
                "--model", params['function'],
            ]
            script_file.write(" ".join(command) + "\n")
script_file.close()
chmod(script_filename, 0755)

