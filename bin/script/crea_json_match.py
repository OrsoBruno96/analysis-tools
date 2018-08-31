#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment
from settings_parallelization import correction_level_signal, name_of_lep, \
    open_and_create_dir, mass_points_signal, base_dir, tmp_dir, scripts_dir
from os.path import join as ojoin
from os import chmod
from decimal import Decimal, getcontext

matches = ["0", "1", "2"]
getcontext().prec = 1

specific_directory = "fourth_jet_veto/medium_wp"


script_filename = ojoin(tmp_dir, "script/plot_stack.sh")
script_file = open_and_create_dir(script_filename)
command_list = list()
template_loader = FileSystemLoader(searchpath=ojoin(scripts_dir, 'templates/'))
template_env = Environment(loader=template_loader)

for mass in mass_points_signal:
    template_file = "gen_stack.json"
    template = template_env.get_template(template_file)
    cur_dir = ojoin(ojoin(base_dir, "raw_files/MC"), specific_directory)
    for c in correction_level_signal:
        c = "_".join(c)
        for lep in (True, False):
            coppie = []
            for m in matches:
                leg = ""
                name = ""
                colore = ""
                if int(m) == 0:
                    colore = "[255, 0, 0]"
                elif int(m) == 1:
                    colore = "[0, 255, 0]"
                elif int(m) == 2:
                    colore = "[0, 0, 255]"
                if lep:
                    leg = "SL/" + m + " match"
                    nome = "mass_histo_lepton_" + m + "_match;1"
                else:
                    leg = "FH/" + m + " match"
                    nome = "mass_histo_chromo_" + m + "_match;1"
                coppie.append((nome, leg, colore))
            outname = "_".join([mass['mass'], c, name_of_lep(lep)])
            input_filename = ojoin(cur_dir, "_".join([mass['mass'], c]) + ".root")
            binning = Decimal(mass['highx']/mass['bins'])
            context={
                'histos': [
                    {
                        'file': input_filename,
                        'name': h[0],
                        'legend': h[1],
                        'color': h[2],
                    } for h in coppie
                ],
                'x_axis': "M_{12} [GeV]",
                'y_axis': "Events / " + str(binning) + " GeV",
            }
            out_text = template.render(**context)
            out_dir = ojoin(ojoin(ojoin(base_dir, "plots/"), specific_directory), "stack")
            json_filename = ojoin(out_dir, outname + ".json")
            pdf_filename = ojoin(out_dir, outname + ".pdf")
            png_filename = ojoin(out_dir, outname + ".png")
            f = open_and_create_dir(json_filename)
            f.write(out_text)
            f.close()
            command = [
                "PlotStackStyle",
                "--input", json_filename,
                "--output", pdf_filename, png_filename
            ]
            command_list.append(" ".join(command))
            
for f in command_list:
    script_file.write(f + "\n")
script_file.close()
chmod(script_filename, 0755)
print(script_filename)
