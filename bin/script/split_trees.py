#!/usr/bin/env python
# -*- coding:utf-8 -*-

from settings_parallelization import correction_level_bkg, correction_level_signal, \
    name_of_lep, open_and_create_dir, mkdir_p, base_dir, tmp_dir
from os import chmod
from os.path import join as ojoin

specific_directory = "fourth_jet_veto/tight_wp"

directory_bkg = ojoin(base_dir, ojoin("raw_files/bkg" , specific_directory))
directory_splitted_bkg = ojoin(base_dir, ojoin("split/bkg", specific_directory))
mkdir_p(directory_splitted_bkg)

script_filename = ojoin(tmp_dir, "script/split_all.sh")
script_file = open_and_create_dir(script_filename)
eras = ["C", "D", "E", "F"]
prescale = 11



commands = list()
for c in correction_level_bkg:
    c = "_".join(c)
    input_files = list()
    for e in eras:
        input_files.append(ojoin(directory_bkg, "_".join(["bkg", e, c]) + ".root"))
    command = ["SplitTree",
               "--input"] + input_files + [
                   "--output-dir", directory_splitted_bkg,
                   "--output-name", c,
                   "--prescale", str(prescale)
                   ]
    commands.append(" ".join(command))

for c in commands:
    script_file.write(c + "\n")
    
script_file.close()
chmod(script_filename, 0755)
print("Now please run")
print(script_filename)
