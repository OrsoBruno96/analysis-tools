#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from settings_parallelization import correction_level_bkg, open_and_create_dir
from os.path import join as ojoin
from os import chmod


base_directory = "/nfs/dust/cms/user/zorattif/output"
specific_directory = "no_reranking/medium_wp"
directory_bkg = ojoin(base_directory, ojoin("raw_files/bkg" , specific_directory))
directory_splitted_bkg = ojoin(base_directory, ojoin("split/bkg", specific_directory))
output_directory = ojoin(ojoin(ojoin(base_directory, "plots"), specific_directory), "fit")

script_filename = "../_tmp/script/run_fits.sh"
script_file = open(script_filename, "w")

eras = ["C", "D", "E", "F"]
lumi = 35.6

pars = [0.001, 1862, 240559, 43, 62, 1, -0.008]

subranges = [
    {
        'min': 120,
        'max': 600,
        'bins': 1000,
        'name': "first",
        'pars_filename': "../_tmp/fit/init_pars_supernovo_first.txt",
        'pars': pars
    },
    {
        'min': 400,
        'max': 1100,
        'bins': 300,
        'name': "second",
        'pars_filename': "../_tmp/fit/init_pars_supernovo_second.txt",
        'pars': pars
    },
    {
        'min': 800,
        'max': 1600,
        'bins': 50,
        'name': "third",
        'pars_filename': "../_tmp/fit/init_pars_supernovo_third.txt",
        'pars': pars
    },
]


for c in correction_level_bkg:
    c = "_".join(c)
    for sub in subranges:
        input_names = list()
        for e in eras:
            input_names.append(ojoin(directory_bkg, "_".join(["bkg", e, c]) + ".root"))
        out_basename = "_".join(["fit", "bkg"] + eras + [c, sub['name']])
        out_basename = ojoin(output_directory, out_basename)
        out_pars_file = open_and_create_dir(sub['pars_filename'])
        for f in sub['pars']:
            out_pars_file.write(str(f) + "\n")
        command = [
            "FitBackground",
            "--input", ] + input_names + [
                "--output", out_basename + ".txt",
                "--print", out_basename + ".png",
                "--lumi", str(lumi),
                "--min-x", str(sub["min"]),
                "--max-x", str(sub["max"]),
                "--bins", str(sub["bins"]),
                "--initial-pars", sub['pars_filename'],
                "--model", "super_novosibirsk",
        ]
        script_file.write(" ".join(command) + "\n")
        
script_file.close()
chmod(script_filename, 0755)
