#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TChain
from jinja2 import FileSystemLoader, Environment
from ROOT import RooRealVar, RooDataHist, RooWorkspace, RooArgList, RooHistPdf, \
    RooArgSet
from ROOT.RooFit import Import
from pylab import loadtxt

from settings_parallelization import correction_level_bkg, correction_level_signal, \
    name_of_lep, open_and_create_dir, mkdir_p, get_signal_cl_from_bkg, tmp_dir, \
    condor_submit, condor_script_executable, base_dir, scripts_dir
from fit_script import subranges as fit_bkg_details
from fit_script import mass_points as fit_mc_details

from os.path import join as ojoin
from os import chmod


shape_bkg = True
shape_mc = True
specific_directory = "fourth_jet_veto/medium_wp"

lep = [False, True]
eras = ["C", "D", "E", "F"]
mass_points = ["120", "350", "600", "1200"]

lumi = 35.6

limits = {
    "120": {
        "lep": [120, 600, 200],
        "chr": [120, 600, 200],
    },
    "350": {
        "lep": [120, 600, 200],
        "chr": [120, 600, 200],
    },
    "600": {
        "lep": [120, 1000, 200],
        "chr": [120, 1000, 200],
    },
    "1200": {
        "lep": [120, 1200, 200],
        "chr": [120, 1200, 200],
    }
}


correct_fit_bkg = {
    "lep": {
        "120": ("first", "super_novosibirsk"),
        "350": ("first", "super_novosibirsk"),
        "600": ("first", "super_novosibirsk"),
        "1200": ("third", "super_novosibirsk"),
    },
    "chr": {
        "120": ("first", "super_novosibirsk"),
        "350": ("first", "super_novosibirsk"),
        "600": ("first", "super_novosibirsk"),
        "1200": ("third", "super_novosibirsk"),
    }
}


correct_fit_mc = {
    "lep": {
        "120": "bukin",
        "350": "bukin",
        "600": "bukin",
        "1200": "bukin",
    },
    "chr": {
        "120": "bukin",
        "350": "bukin",
        "600": "bukin",
        "1200": "bukin",
    }
}


parameters_name = {
    "bukin": ["Xp", "sp", "rho1", "rho2", "xi"],
    "super_novosibirsk": ["p0", "p1", "p3", "p4", "p5", "p6", ],
}



scale_factor_MC = lumi/1000
directory_bkg = ojoin(base_dir, ojoin("raw_files/bkg" , specific_directory))
directory_splitted_bkg = ojoin(base_dir, ojoin("split/bkg", specific_directory))
directory_mc = ojoin(base_dir, ojoin("raw_files/MC", specific_directory))
directory_sig = ojoin(base_dir, ojoin("raw_files/signal", specific_directory))
directory_fit = ojoin(ojoin(base_dir, "fit/bkg"), specific_directory)

if shape_bkg and shape_mc:
    sssspecific_dir = "shape_all"
else:
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
template_loader = FileSystemLoader(searchpath=ojoin(scripts_dir, 'combine/templates/'))
template_env = Environment(loader=template_loader)
if shape_bkg and not shape_mc:
    template = template_env.get_template("datacard_shape.j2")
    runtime = 60
elif shape_bkg and shape_mc:
    template = template_env.get_template("datacard_shape_all.j2")
    runtime = 300
else:
    template = template_env.get_template("datacard.j2")
    runtime = 10
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
            if shape_bkg or shape_mc:
                limit[0] = max(
                    (item for item in fit_bkg_details[name_of_lep(l)]
                     if item['name'] == correct_fit_bkg[name_of_lep(l)][mass][0]).next()['min'],
                    fit_mc_details[name_of_lep(l)][mass]['min'])
                limit[1] = min(
                    (item for item in fit_bkg_details[name_of_lep(l)]
                     if item['name'] == correct_fit_bkg[name_of_lep(l)][mass][0]).next()['max'],
                    fit_mc_details[name_of_lep(l)][mass]['max'])
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
            # mc_events = histo_mc.GetEntries()
            mc_events = histo_mc.Integral(0, limit[2] - 1)
            histo_bkg.Write()
            histo_mc.Write()
            histo_sig.Write()
            output_file.Close()
            output_filename = ojoin(out_dir, "datacards")
            output_filename = ojoin(
                output_filename, "_".join([mass, name_of_lep(l), cb]) + ".txt")
            parameters_list_datacard = list()
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
                file_parameters_bkg = ojoin(ojoin(directory_fit, "tables"), file_bkg_basename + ".txt")
                param, error = loadtxt(file_parameters_bkg, unpack=True)
                for p, e, n in zip(param, error,
                                   parameters_name[correct_fit_bkg[name_of_lep(l)][mass][1]]):
                    appo = dict()
                    appo["name"] = n; appo["sigma"] = e; appo["mean"] = p
                    parameters_list_datacard.append(appo)
            if shape_mc:
                file_mc_basename = "_".join(["fit", "mc", name_of_lep(l), cs, mass])
                input_mc = ojoin(directory_fit, file_mc_basename + ".root")
                output_mc = ojoin(directory_fit, file_mc_basename + "_normalized.root")
                histos_file = ojoin(out_dir, "_".join(
                    ["combine", name_of_lep(l), mass, cb]) + ".root")
                command = [
                    "AddNormalizationToFile",
                    "--input-file", input_mc,
                    "--output-file", output_mc,
                    "--value", str(mc_events),
                    "--name", correct_fit_mc[name_of_lep(l)][mass] + "_norm"
                ]
                normalization_command_list.append(command)
                file_parameters_mc = ojoin(ojoin(directory_fit, "tables"), file_mc_basename + ".txt")
                param, error = loadtxt(file_parameters_mc, unpack=True)
                for p, e, n in zip(param, error,
                                   parameters_name[correct_fit_mc[name_of_lep(l)][mass]]):
                    appo = dict()
                    appo["name"] = n; appo["sigma"] = e; appo["mean"] = p
                    parameters_list_datacard.append(appo)
            if shape_bkg and not shape_mc:
                out_text = template.render(
                    obs=bkg_events,
                    file_name=histos_file,
                    file_fit=output_bkg,
                    file_name_roomc=ojoin(out_dir, "roomc_" + out_filename2),
                    file_name_roobkg=ojoin(out_dir, "roobkg_" + out_filename2)
                )
            if shape_bkg and shape_mc:
                out_text = template.render(
                    obs=bkg_events,
                    file_fit_bkg=output_bkg,
                    file_fit_mc=output_mc,
                    file_name_roobkg=ojoin(out_dir, "roobkg_" + out_filename2),
                    parameters=parameters_list_datacard,
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
        runtime=runtime, memory=100)
        
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
