#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TChain
from jinja2 import FileSystemLoader, Environment
from ROOT import RooRealVar, RooDataHist, RooWorkspace, RooArgList, RooHistPdf, \
    RooArgSet
from ROOT.RooFit import Import

from settings_parallelization import correction_level_bkg, correction_level_signal, \
    name_of_lep, open_and_create_dir, mkdir_p, get_signal_cl_from_bkg, tmp_dir, \
    condor_submit, condor_script_executable, base_dir
from os.path import join as ojoin
from os import chmod


shape_bkg = False
specific_directory = "only_three_jets/medium_wp"

lep = [False, True]
eras = ["C", "D", "E", "F"]
mass_points = ["120", "350", "1200"]

lumi = 35.6

limits = {
    "120": {
        "lep": (120, 600, 60),
        "chr": (120, 600, 60),
    },
    "350": {
        "lep": (120, 600, 120),
        "chr": (120, 600, 120),
    },
    "1200": {
        "lep": (120, 1200, 90),
        "chr": (120, 1200, 90),
    }
}


correct_fit_bkg = {
    "lep": {
        "120": ("first", "super_novosibirsk"),
        "350": ("first", "super_novosibirsk"),
        "1200": ("third", "super_novosibirsk"),
    },
    "chr": {
        "120": ("first", "super_novosibirsk"),
        "350": ("first", "super_novosibirsk"),
        "1200": ("third", "super_novosibirsk"),
    }
}


scale_factor_MC = lumi/1000
directory_bkg = ojoin(base_dir, ojoin("raw_files/bkg" , specific_directory))
directory_splitted_bkg = ojoin(base_dir, ojoin("split/bkg", specific_directory))
directory_mc = ojoin(base_dir, ojoin("raw_files/MC", specific_directory))
directory_sig = ojoin(base_dir, ojoin("raw_files/signal", specific_directory))
directory_fit = ojoin(ojoin(base_dir, "fit/bkg"), specific_directory)

if shape_bkg:
    sssspecific_dir = "shape"
else:
    sssspecific_dir = "template"
out_dir = ojoin(base_dir, ojoin(
    ojoin("combine_tool", sssspecific_dir), specific_directory))
mkdir_p(out_dir)
output_script_filedir = ojoin(tmp_dir, "script")


list_of_datacards = list()
roofit_file_list = list()
normalization_command_list = list()
template_loader = FileSystemLoader(searchpath='./combine/templates/')
template_env = Environment(loader=template_loader)
if shape_bkg:
    template = template_env.get_template("datacard_shape.j2")
else:
    template = template_env.get_template("datacard.j2")
template_script = template_env.get_template("combine_script.j2")


# Retrieve histograms or pdf from trees. Limits are set for each mass point.
# Also stuff is done for every correction level
for mass in mass_points:
    for cb in correction_level_bkg:
        cs = get_signal_cl_from_bkg(cb)
        cb = "_".join(cb)
        cs = "_".join(cs)
        for l in lep:
            appo_bkg = TChain("output_tree")
            appo_mc = TChain("output_tree")
            appo_sig = TChain("output_tree")
            limit = limits[mass][name_of_lep(l)]
            limit_string = "(Mass > " + str(limit[0]) + \
                           ") && (Mass < " + str(limit[1]) + ")"
            if l:
                filter_string = "Leptonic_event"
            else:
                filter_string = "!(Leptonic_event)"
            for e in eras:
                appo_sig.Add(ojoin(directory_sig, "_".join(["sig", e, cb]) + ".root"))
            appo_bkg.Add(ojoin(directory_splitted_bkg, "_".join([cb, "2"]) + ".root"))
            appo_mc.Add(ojoin(directory_mc, "_".join([mass, cs]) + ".root"))
            output_filename = "_".join(["combine", name_of_lep(l), mass, cb]) + ".root"
            out_filename2 = output_filename
            roofit_file_list.append(output_filename)
            output_filename = ojoin(out_dir, output_filename)
            output_file = TFile(output_filename, "recreate")
            output_file.cd()
            histo_bkg = TH1F("bbnb_Mbb", "bbnb Mbb", limit[2], limit[0], limit[1])
            histo_mc = TH1F("MC_bbb_Mbb", "bbb Mbb MC", limit[2], limit[0], limit[1])
            histo_sig = TH1F("sig_bbb_Mbb", "bbb Mbb", limit[2], 0, 200)
            appo_bkg.Draw("Mass>>bbnb_Mbb", " && ".join([filter_string, limit_string]), "goff")
            appo_mc.Draw(
                "Mass>>MC_bbb_Mbb", "Weigth*" + " && ".join(
                    [filter_string, limit_string]), "goff")
            histo_mc.Scale(scale_factor_MC)
            appo_sig.Draw("Mass>>sig_bbb_Mbb", " && ".join([filter_string, limit_string]), "goff")
            bkg_events = histo_bkg.GetEntries()
            histo_bkg.Write()
            histo_mc.Write()
            histo_sig.Write()
            output_file.Close()
            output_filename = ojoin(out_dir, "datacards")
            output_filename = ojoin(
                output_filename, "_".join([mass, name_of_lep(l), cb]) + ".txt")
            if shape_bkg:
                file_bkg_basename = "_".join(["fit", "bkg", name_of_lep(l)] + eras + \
                                    [cb, correct_fit_bkg[name_of_lep(l)][mass][0], ])
                input_bkg = ojoin(directory_fit, file_bkg_basename + ".root")
                output_bkg = ojoin(directory_fit, file_bkg_basename + "_normalized.root")
                histos_file = ojoin(out_dir, "_".join(
                    ["combine", name_of_lep(l), mass, cb]) + ".root")
                command = [
                    "AddNormalizationToFile",
                    "--input-file", input_bkg,
                    "--output-file", output_bkg,
                    "--value", str(bkg_events),
                    "--name", correct_fit_bkg[name_of_lep(l)][mass][1] + "_norm"
                ]
                normalization_command_list.append(command)
                out_text = template.render(
                    obs=bkg_events,
                    file_name=histos_file,
                    file_fit=output_bkg,
                    file_name_roomc=ojoin(out_dir, "roomc_" + out_filename2),
                    file_name_roobkg=ojoin(out_dir, "roobkg_" + out_filename2)
                )
            else:
                out_text = template.render(
                    obs=bkg_events,
                    file_name=ojoin(
                        out_dir, "_".join(["combine", name_of_lep(l), mass, cb]) + ".root"))
            output_file = open_and_create_dir(output_filename)
            output_file.write(out_text)
            list_of_datacards.append({'mass': mass, 'file': output_filename,
                                      'correction': cb, 'lep': name_of_lep(l),
                                      })

            
list_of_directories = [
    ojoin(ojoin(ojoin(out_dir, "out"),
                name_of_lep(l)), "_".join(c)) for c in correction_level_bkg for l in lep]



move_filename = ojoin(tmp_dir, "script/move_hists.sh")
normalize_filename = ojoin(tmp_dir, "script/normalize_pdf.sh")
if shape_bkg:
    move_file = open(move_filename, "w")
    for f in roofit_file_list:
        command = [
            "MoveRoohisto",
            "--input-file", ojoin(out_dir, f),
            "--input-histo", "bbnb_Mbb",
            "--output-file", ojoin(out_dir, "roobkg_" + f),
            "--output-histo", "Roo_bbnb_Mbb"
        ]
        move_file.write(" ".join(command) + "\n")
        command = [
            "MoveRoohisto",
            "--input-file", ojoin(out_dir, f),
            "--input-histo", "MC_bbb_Mbb",
            "--output-file", ojoin(out_dir, "roomc_" + f),
            "--output-histo", "Roo_MC_bbb_Mbb"
        ]
        move_file.write(" ".join(command) + "\n")
    move_file.close()
    chmod(move_filename, 0755)
    normalize_file = open(normalize_filename, "w")
    for c in normalization_command_list:
        normalize_file.write(" ".join(c) + "\n")
    normalize_file.close()
    chmod(normalize_filename, 0755)

process_list = list()

for l in list_of_datacards:
    condor_submit(
        process_list,
        "cd", [ojoin(ojoin(out_dir, "out", l['lep']), l['correction']), " && ",
               "combine", "-M", "AsymptoticLimits", "--rMin=-20", "--rMax=20", "-n", "Hbb",
               "-m", l['mass'], l['file']],
        "_".join(["combine", l['correction'], l['lep'], l['mass']]),
        runtime=10, memory=100)
        
bash_filename = ojoin(ojoin(tmp_dir, "script"), "condor_combine.sh")
bash_file = open_and_create_dir(bash_filename)
bash_file.write("#!/bin/bash\n")
for p in process_list:
    bash_file.write(
        " ".join([condor_script_executable, p['filename'], p['runtime'], p['memory']]) + "\n")
bash_file.close()
chmod(bash_filename, 0755)
print("Condor: run " + bash_filename)



out_text = template_script.render(
    file_list=list_of_datacards,
    out_dir=ojoin(out_dir, "out"),
    lep=[name_of_lep(l) for l in lep],
    corrections=["_".join(c) for c in correction_level_bkg],
    directories=list_of_directories)
out_filename = ojoin(output_script_filedir, "run_combine_all.sh")
out_file = open_and_create_dir(out_filename)
out_file.write(out_text)
chmod(out_filename, 0755)

script_filename = out_filename

for c in correction_level_bkg:
    c = "_".join(c)
    for l in lep:
        cur_dir = ojoin(out_dir, "out")
        ll = name_of_lep(l)
        cur_dir = ojoin(cur_dir, ll)
        cur_dir = ojoin(cur_dir, c)
        out_filename = "HbbLimits"
        out_filename = ojoin(cur_dir, out_filename)
        out_file = open_and_create_dir(out_filename)
        for m in mass_points:
            out_file.write(
                ojoin(
                    cur_dir,
                    "higgsCombineHbb.AsymptoticLimits.mH" + m + ".root\n"))

print("Now please run the following: ")
if shape_bkg:
    print(move_filename)
    print(normalize_filename)
print(script_filename)
