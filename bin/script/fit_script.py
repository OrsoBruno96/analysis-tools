#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from settings_parallelization import correction_level_bkg, open_and_create_dir, mkdir_p, \
    tmp_dir, correction_level_signal, base_dir, condor_submit, create_condor_file
from os.path import join as ojoin
from os import chmod


specific_directory = "fourth_jet_veto/medium_wp"
directory_bkg = ojoin(base_dir, ojoin("raw_files/bkg" , specific_directory))
directory_splitted_bkg = ojoin(base_dir, ojoin("split/bkg", specific_directory))
directory_mc = ojoin(base_dir, ojoin("raw_files/MC" , specific_directory))
output_print_directory = ojoin(ojoin(ojoin(base_dir, "plots"), specific_directory), "fit")
output_root_directory = ojoin(base_dir, ojoin("fit/bkg/", specific_directory))
output_table_directory = ojoin(output_root_directory, "tables")
mkdir_p(output_print_directory)
mkdir_p(output_root_directory)
mkdir_p(output_table_directory)

script_filename_bkg = ojoin(tmp_dir, "script/run_fits_bkg.sh")
script_file_bkg = open(script_filename_bkg, "w")
script_file_bkg.write("# -*- coding:utf-8 -*-\nsource /cvmfs/cms.cern.ch/cmsset_default.sh\ncmsenv\n")

script_filename_mc = ojoin(tmp_dir, "script/run_fits_mc.sh")
script_file_mc = open(script_filename_mc, "w")
script_file_mc.write("# -*- coding:utf-8 -*-\nsource /cvmfs/cms.cern.ch/cmsset_default.sh\ncmsenv\n")

eras = ["C", "D", "E", "F"]
lumi = 35.6

pars = [0.001, 1862, 240559, 43, 62, 1, -0.008]
pars_chr = [0.001, 1700,  60559, 250, 70, 1, -0.008]
process_list = list()

subranges = {
    "lep": [
        {
            'min': 110,
            'max': 700,
            'bins': 750,
            'name': "first",
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_lep_supernovo_first.txt"),
            'pars': pars,
            'model': "super_novosibirsk",
            'logy': False
        },
        {
            'min': 400,
            'max': 1100,
            'bins': 300,
            'name': "second",
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_lep_supernovo_second.txt"),
            'pars': pars,
            'model': "super_novosibirsk",
            'logy': True,
        },
        {
        'min': 800,
            'max': 1600,
            'bins': 50,
            'name': "third",
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_lep_supernovo_third.txt"),
            'pars': pars,
            'model': "super_novosibirsk",
            'logy': True,
        },
    ],
    "chr": [
        {
            'min': 200,
            'max': 700,
            'bins': 300,
            'name': "first",
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_chr_supernovo_first.txt"),
            'pars': pars_chr,
            'model': "super_novosibirsk",
            'logy': False,
        },
        {
            'min': 400,
            'max': 1100,
            'bins': 200,
            'name': "second",
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_chr_supernovo_second.txt"),
            'pars': pars_chr,
            'model': "super_novosibirsk",
            'logy': True,
        },
        {
            'min': 800,
            'max': 1600,
            'bins': 50,
            'name': "third",
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_chr_supernovo_third.txt"),
            'pars': pars_chr,
            'model': "super_novosibirsk",
            'logy': True,
        },
    ],
}


pars_bukin_350 = [350, 310., 60., 0.01, 0.01, -0.2]
pars_bukin_1200 = [80, 1100, 300, 0.01, 0.01, 0.01]
pars_bukin_120 = [50, 120, 30, 0.01, 0.01, 0.01]

mass_points = {
    "lep": {
        "120": {
            'min': 0,
            'max': 400,
            'bins': 40,
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_bukin_120.txt"),
            'pars': pars_bukin_120,
            'model': "bukin",
            'logy': False,
        },
        "350": {
            'min': 80,
            'max': 550,
            'bins': 60,
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_bukin_350.txt"),
            'pars': pars_bukin_350,
            'model': "bukin",
            'logy': False,
        },
        "1200": {
            'min': 100,
            'max': 1400,
            'bins': 60,
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_bukin_1200.txt"),
            'pars': pars_bukin_1200,
            'model': "bukin",
            'logy': False,
        },
    },
    "chr": {
        "120": {
            'min': 150,
            'max': 500,
            'bins': 20,
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_bukin_120.txt"),
            'pars': pars_bukin_120,
            'model': "bukin",
            'logy': False,
        },
        "350": {
            'min': 180,
            'max': 500,
            'bins': 60,
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_bukin_350.txt"),
            'pars': pars_bukin_350,
            'model': "bukin",
            'logy': False,
        },
        "1200": {
            'min': 300,
            'max': 1500,
            'bins': 60,
            'pars_filename': ojoin(tmp_dir, "fit/init_pars_bukin_1200.txt"),
            'pars': pars_bukin_1200,
            'model': "bukin",
            'logy': False,
        }
    }
}

if __name__ == "__main__":
    for c in correction_level_bkg:
        c = "_".join(c)
        for l, subb in subranges.iteritems():
            for sub in subb:
                input_names = list()
                for e in eras:
                    input_names.append(ojoin(directory_bkg, "_".join(["bkg", e, c]) + ".root"))
                out_basename = "_".join(["fit", "bkg", l] + eras + [c, sub['name'], ])
                out_pars_file = open_and_create_dir(sub['pars_filename'])
                for f in sub['pars']:
                    out_pars_file.write(str(f) + "\n")
                command = [
                    "FitBackground",
                    "--input", ] + input_names + [
                        "--output", ojoin(output_root_directory, out_basename + ".root"),
                        "--print", ojoin(output_print_directory, out_basename + ".pdf"),
                        ojoin(output_print_directory, out_basename + ".png"),
                        "--lumi", str(lumi),
                        "--min-x", str(sub["min"]),
                        "--max-x", str(sub["max"]),
                        "--bins", str(sub["bins"]),
                        "--output-table", ojoin(output_table_directory, out_basename + ".txt"),
                        "--initial-pars", sub['pars_filename'],
                        "--model", sub['model'],
                        "--use-integral",
                    ]
                if l == "chr":
                    command.append("--full-hadronic")
                if sub['logy']:
                    command.append("--log-y")
                script_file_bkg.write(" ".join(command) + "\n")
                condor_submit(
                    process_list, "", command, ojoin(tmp_dir, ojoin("condor/fit", out_basename)),
                    120, 100)

        
    script_file_bkg.close()
    chmod(script_filename_bkg, 0755)


    for c in correction_level_signal:
        c = "_".join(c)
        for l, appppo in mass_points.iteritems():
            for m, sub in appppo.iteritems():
                input_filename = ojoin(directory_mc, "_".join([m, c]) + ".root")
                out_basename = "_".join(["fit", "mc", l, c, m])
                out_pars_file = open_and_create_dir(sub['pars_filename'])
                for f in sub['pars']:
                    out_pars_file.write(str(f) + "\n")
                command = [
                    "FitBackground",
                    "--input", input_filename,
                    "--output", ojoin(output_root_directory, out_basename + ".root"),
                    "--print", ojoin(output_print_directory, out_basename + ".pdf"),
                    ojoin(output_print_directory, out_basename + ".png"),
                    "--min-x", str(sub["min"]),
                    "--max-x", str(sub["max"]),
                    "--bins", str(sub["bins"]),
                    "--output-table", ojoin(output_table_directory, out_basename + ".txt"),
                    "--initial-pars", sub['pars_filename'],
                    "--model", sub['model'],
                    "--use-integral",
                ]
                if l == "chr":
                    command.append("--full-hadronic")
                if sub['logy']:
                    command.append("--log-y")
                script_file_mc.write(" ".join(command) + "\n")
                condor_submit(
                    process_list, "", command, ojoin(tmp_dir, ojoin("condor/fit", out_basename)),
                    120, 100)

    script_file_mc.close()
    chmod(script_filename_mc, 0755)


    print("I produced 2 bash scripts that can be run with condor. Please run:")
    print(script_filename_bkg)
    print(script_filename_mc)
    condor_filename = ojoin(tmp_dir, "script/condor_fit.sh")
    create_condor_file(condor_filename, process_list)
    chmod(condor_filename, 0755)
    print("I created a condor submission file to run all parallel instead of "
          + "previous one. If you want condor, run: ")
    print(condor_filename)
