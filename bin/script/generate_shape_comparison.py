#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment
from settings_parallelization import name_of_lep, correction_level_signal, base_dir, \
    mass_points_signal, tmp_dir, open_and_create_dir, scripts_dir, mkdir_p
from os.path import join as ojoin
from os import chmod


specific_directory = "fourth_jet_veto/medium_wp"
lep = [True, False]

mc_dir = ojoin(ojoin(base_dir, "raw_files/MC"), specific_directory)

template_loader = FileSystemLoader(searchpath=ojoin(scripts_dir, "templates"))
template_env = Environment(loader=template_loader)
template = template_env.get_template("comparison_shapes.json")

comparisons = [
    ("smearing_btag_true", "smearing_btag_regression_true"),
    ("smearing_btag_true", "smearing_btag_fsr_true"),
    ("smearing_btag_true", "smearing_btag_regression_fsr_true")
]


plots_dir = ojoin(ojoin(ojoin(base_dir, "plots"), specific_directory), "shapes")
mkdir_p(plots_dir)

def selection_from_lep(lep):
    if lep:
        return "Weigth*Leptonic_event"
    else:
        return "Weigth*(!(Leptonic_event))"


colori = {
    "120": [0, 0, 255],
    "350": [255, 0, 0],
    "600": [0, 255, 0],
    "1200": [255, 130, 0],
}

binning = {
    "120": 30,
    "350": 50,
    "600": 60,
    "1200": 40,
}

lo_vogliamo_davvero = {
    "lep": {
        "120": True,
        "350": True,
        "600": True,
        "1200": True,
    },
    "chr": {
        "120": False,
        "350": True,
        "600": True,
        "1200": True,
    }
}


def pretty_name(corr):
    if corr == "smearing_btag_regression_true":
        return "Reg"
    elif corr == "smearing_btag_fsr_true":
        return "FSR"
    elif corr == "smearing_btag_regression_fsr_true":
        return "Reg + FSR"
    else:
        raise RuntimeError("pretty_name called with wrong argument")


command_list = list()
    
for l in lep:
    for c in comparisons:
        shapes = list()
        out_filename = ojoin(tmp_dir, "template/comparison_shapes_" +
                             name_of_lep(l) + "_" + "_".join(c) + ".json")
        for m in mass_points_signal:
            m = m['mass']
            if not lo_vogliamo_davvero[name_of_lep(l)][m]:
                continue
            shapes.append(
                {
                    'before': {
                        'root_file': ojoin(mc_dir, "_".join([m, c[0]]) + ".root"),
                        'selection': selection_from_lep(l),
                    },
                    'after': {
                        'root_file': ojoin(mc_dir, "_".join([m, c[1]]) + ".root"),
                        'selection': selection_from_lep(l),                        
                    },
                    'color': colori[m],
                    'name': m,
                    'bins': binning[m],
                }
            )
        out_text = template.render(
            x_axis="M_{12} [GeV]",
            y_axis="Events [a.u.]",
            title="Shape comparison",
            shapes=shapes,
            minx=0,
            maxx=1300,
            legend_before="Before corr",
            legend_after=pretty_name(c[1])
        )
        out_file = open_and_create_dir(out_filename)
        out_file.write(out_text)
        out_file.close()
        base_printname = "_".join(c) + "_" + name_of_lep(l)
        command_list.append(
            [
                "PlotShapes",
                "--input", out_filename,
                "--output", ojoin(plots_dir, base_printname + ".pdf"),
                ojoin(plots_dir, base_printname + ".png")
            ])
bash_filename = ojoin(tmp_dir, "scripts/shapes_cusu.sh")
bash_file = open_and_create_dir(bash_filename)
bash_file.write("#!/bin/bash\n")
for c in command_list:
    bash_file.write(" ".join(c) + "\n")
bash_file.close()
chmod(bash_filename, 0755)
print("Now run:")
print(bash_filename)
