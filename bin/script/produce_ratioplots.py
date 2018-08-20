#!/usr/bin/env python
# -*- coding:utf-8 -*-

from jinja2 import FileSystemLoader, Environment
from settings_parallelization import correction_level_bkg, correction_level_signal, \
    name_of_lep, open_and_create_dir, mass_points_signal, bkg_files
from ROOT import TFile, TChain, TH1F
from os.path import join as ojoin
from os import chmod
from subprocess import Popen, STDOUT, PIPE, call, check_call
from decimal import Decimal, getcontext


# Input variables
type_choice = "bkg"
plot_choice = 'mass'
corr_level1 = 'nothing_true'
corr_level2 = 'regression_true'
era1 = ['F',]
era2 = ['F',]
highx = 800
binx = 150
fix_min_ratio = None
fix_max_ratio = 2
lumi = 35.6
extra_filter1 = None
extra_filter2 = None

getcontext().prec = 1
lep = True


base_directory = "/nfs/dust/cms/user/zorattif/output"
cur_working = "no_reranking/medium_wp"
output_name = ojoin(ojoin(base_directory, ojoin("plots", cur_working)), "_".join(
    ["ratio", name_of_lep(lep), type_choice, plot_choice] + era1 +[corr_level1, ] + era2 + [corr_level2]) + ".pdf")



# Modify after this only if you know what are doing
tmp_filename = "/nfs/dust/cms/user/zorattif/trash/tmp.root"
templated_filename = "../_tmp/template/ratioplot.json"
template_loader = FileSystemLoader(searchpath='./templates/')
template_env = Environment(loader=template_loader)
template = template_env.get_template("ratioplot.j2")


possibilities = {
    'mass': {
        'tree': 'Mass',
        'title': "Invariant mass of bb jets",
    },
    'pt1': {
        'tree': 'Jet_newPt[0]',
        'title': "p_T of first jet",
    },
    'pt2': {
        'tree': 'Jet_newPt[1]',
        'title': 'p_T of second jet',
    },
    'pt3': {
        'tree': 'Jet_newPt[2]',
        'title': "p_T of third jet",
    }
}


def get_filename(type_choice, corr_level, era_or_mass):
    if type_choice == "MC":
        return "_".join([str(era_or_mass), corr_level]) + ".root"
    if type_choice == "signal":
        return "_".join(["sig", era_or_mass, corr_level]) + ".root"
    if type_choice == "bkg":
        return "_".join(["bkg", era_or_mass, corr_level]) + ".root"
    else:
        raise RuntimeError("get_filename called with wrong parameter")


def get_filter_string(lep):
    if lep:
        return "Leptonic_event"
    else:
        return "(!Leptonic_event)"

    
def get_legend(corr_level):
    if "true" in corr_level:
        appo = ""
    else:
        appo = "(NT)"
    if "nothing" in corr_level:
        appo2 = "Nothing"
    else:
        appo2 = list()
        if "smearing" in corr_level:
            appo2.append("SMR")
        if "regression" in corr_level:
            appo2.append("REG")
        if "btag" in corr_level:
            appo2.append("BTAG")
        if "fsr" in corr_level:
            appo2.append("FSR")
        appo2 = " ".join(appo2)
    return " ".join([appo2, appo])


def get_y_axis(binning):
    return "Events / " + str(binning) + " GeV"


def get_x_axis(choice):
    if choice == "mass":
        return "M_{12} [GeV]"
    elif "pt" in choice:
        return "p_T [GeV]"
    else:
        raise RuntimeError("get_x_axis called with wrong parameter")

    
def join_filter_string(first, second=None):
    if second is None:
        return first
    else:
        return " && ".join([first, second])



input_directory = ojoin(base_directory, "raw_files")
input_directory = ojoin(input_directory, type_choice)
input_directory = ojoin(input_directory, cur_working)
tree1 = TChain("output_tree")
tree2 = TChain("output_tree")

for e in era1:
    input_filename1 = ojoin(input_directory, get_filename(type_choice, corr_level1, e))
    tree1.Add(input_filename1)
for e in era2:
    input_filename2 = ojoin(input_directory, get_filename(type_choice, corr_level2, e))
    tree2.Add(input_filename2)

tmp_histo_file = TFile(tmp_filename, "recreate")
histo1 = TH1F("tmp1", possibilities[plot_choice]['title'], binx, 0, highx)
histo2 = TH1F("tmp2", possibilities[plot_choice]['title'], binx, 0, highx)
tree1.Draw(possibilities[plot_choice]['tree'] + ">>tmp1",
           join_filter_string(get_filter_string(lep), extra_filter1), "")
tree2.Draw(possibilities[plot_choice]['tree'] + ">>tmp2",
           join_filter_string(get_filter_string(lep), extra_filter2), "")
histo1.Write()
histo2.Write()
tmp_histo_file.Close()

histos = [
    {
        'file': tmp_filename,
        'name': "tmp1;1",
        'legend': get_legend(corr_level1),
        'color': [0, 0, 0],
    },
    {
        'file': tmp_filename,
        'name': "tmp2;1",
        'legend': get_legend(corr_level2),
        'color': [255, 0, 0],
    },
]

out_text = template.render(
    histos=histos,
    title=possibilities[plot_choice],
    x_axis=get_x_axis(plot_choice),
    y_axis=get_y_axis(Decimal(highx/binx)))
templated_file = open_and_create_dir(templated_filename)
templated_file.write(out_text)

command = ["RatioPlot", "--input", templated_filename, "--output", output_name]
if fix_min_ratio is not None:
    command += ["--min-ratio", str(fix_min_ratio)]
if fix_max_ratio is not None:
    command += ["--max-ratio", str(fix_max_ratio)]
if type_choice != "MC" and lumi is not None:
   command += ["--lumi", str(lumi)]
# proc = Popen(command, stdout=PIPE, stderr=PIPE)
print(" ".join(command))
out_bash_filename = "../_tmp/script/run_ratioplot.sh"
out_bash_file = open(out_bash_filename, "w")
out_bash_file.write(" ".join(command))
out_bash_file.close()
chmod(out_bash_filename, 0755)
