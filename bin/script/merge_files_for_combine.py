#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TChain
from jinja2 import FileSystemLoader, Environment

from settings_parallelization import correction_level_bkg, correction_level_signal, \
    name_of_lep, open_and_create_dir
from os.path import join as ojoin

import os


shape_bkg = True



lep = [False, True]
eras = ["C", "D", "E", "F"]
mass_points = ["120", "350", "1200"]
limits = {
    "120": (80, 1000, 60),
    "350": (100, 1000, 120),
    "1200": (100, 2000, 90),
}


base_directory = "/nfs/dust/cms/user/zorattif/output"
specific_directory = "no_reranking/medium_wp"

directory_bkg = os.path.join(base_directory, os.path.join("raw_files/bkg" , specific_directory))
directory_splitted_bkg = os.path.join(base_directory, os.path.join("split/bkg", specific_directory))
directory_mc = os.path.join(base_directory, os.path.join("raw_files/MC", specific_directory))
directory_sig = os.path.join(base_directory, os.path.join("raw_files/signal", specific_directory))

if shape_bkg:
    sssspecific_dir = "shape"
else:
    sssspecific_dir = "template"
out_dir = os.path.join(base_directory, os.path.join(
    os.path.join("combine_tool", sssspecific_dir), specific_directory))
output_script_filedir = "../_tmp/script"



list_of_datacards = list()
template_loader = FileSystemLoader(searchpath='./combine/templates/')
template_env = Environment(loader=template_loader)
if shape_bkg:
    template = template_env.get_template("datacard_shape.j2")
else:
    template = template_env.get_template("datacard.j2")
template_script = template_env.get_template("combine_script.j2")

for mass in mass_points:
    for cb, cs in zip(correction_level_bkg, correction_level_signal):
        cb = "_".join(cb)
        cs = "_".join(cs)
        for l in lep:
            appo_bkg = TChain("output_tree")
            appo_mc = TChain("output_tree")
            appo_sig = TChain("output_tree")
            limit = limits[mass]
            limit_string = "(Mass > " + str(limit[0]) + \
                           ") && (Mass < " + str(limit[1]) + ")"
            if l:
                filter_string = "Leptonic_event"
                output_file_name_appo = "lep"
            else:
                filter_string = "!(Leptonic_event)"
                output_file_name_appo = "chr"
            for e in eras:
                appo_sig.Add(os.path.join(directory_sig, "_".join(["sig", e, cb]) + ".root"))
            appo_bkg.Add(os.path.join(directory_splitted_bkg, "_".join([cb, "2"]) + ".root"))
            appo_mc.Add(os.path.join(directory_mc, "_".join([mass, cs]) + ".root"))
            output_file = TFile(os.path.join(
                out_dir, "_".join(["combine", output_file_name_appo, mass, cb]) + ".root"), "recreate")
            output_file.cd()
            histo_bkg = TH1F("bbnb_Mbb", "bbnb Mbb", limit[2], limit[0], limit[1])
            histo_mc = TH1F("MC_bbb_Mbb", "bbb Mbb MC", limit[2], limit[0], limit[1])
            histo_sig = TH1F("sig_bbb_Mbb", "bbb Mbb", limit[2], limit[0], limit[1])
            appo_bkg.Draw("Mass>>bbnb_Mbb", " && ".join([filter_string, limit_string]), "")
            appo_mc.Draw("Mass>>MC_bbb_Mbb", "Weigth*" + " && ".join([filter_string,
                                                                      limit_string]), "")
            appo_sig.Draw("Mass>>sig_bbb_Mbb", " && ".join([filter_string, limit_string]), "")
            bkg_events = histo_bkg.GetEntries()
            histo_bkg.Write()
            histo_mc.Write()
            histo_sig.Write()

            output_filename = os.path.join(out_dir, "datacards")
            output_filename = os.path.join(output_filename,
                                           "_".join([mass, output_file_name_appo, cb]) + ".txt")
            if shape_bkg:
                file_bkg = ojoin(ojoin(ojoin(base_directory, "fit/bkg"),
                                       specific_directory), cb) + ".root"
                out_text = template.render(
                    obs=bkg_events,
                    file_name=os.path.join(
                        out_dir, "_".join(
                            ["combine", output_file_name_appo, mass, cb]) + ".root"),
                    file_fit=file_bkg)
            else:
                out_text = template.render(
                    obs=bkg_events,
                    file_name=os.path.join(
                        out_dir, "_".join(["combine", output_file_name_appo, mass, cb]) + ".root"))
            output_file = open(output_filename, "w")
            output_file.write(out_text)
            list_of_datacards.append({'mass': mass, 'file': output_filename,
                                      'correction': cb, 'lep': output_file_name_appo})



list_of_directories = [os.path.join(
    os.path.join(
        os.path.join(out_dir, "out"),
        name_of_lep(l)), "_".join(c)) for c in correction_level_bkg for l in lep]

out_text = template_script.render(
    file_list=list_of_datacards,
    out_dir=os.path.join(out_dir, "out"),
    lep=[name_of_lep(l) for l in lep],
    corrections=["_".join(c) for c in correction_level_bkg],
    directories=list_of_directories)
out_filename = os.path.join(output_script_filedir, "run_combine_all.sh")
out_file = open(out_filename, "w")
out_file.write(out_text)
os.chmod(out_filename, 0755)


for c in correction_level_bkg:
    c = "_".join(c)
    for l in lep:
        cur_dir = os.path.join(out_dir, "out")
        ll = name_of_lep(l)
        cur_dir = os.path.join(cur_dir, ll)
        cur_dir = os.path.join(cur_dir, c)
        out_filename = "HbbLimits"
        out_filename = os.path.join(cur_dir, out_filename)
        out_file = open_and_create_dir(out_filename)
        for m in mass_points:
            out_file.write(
                os.path.join(
                    cur_dir,
                    "higgsCombineHbb.AsymptoticLimits.mH" + m + ".root\n"))
