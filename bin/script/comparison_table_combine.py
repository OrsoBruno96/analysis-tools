#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-
from os.path import join as ojoin
from pylab import loadtxt
from uncertainties import ufloat
from collections import OrderedDict
from decimal import Decimal, getcontext

from settings_parallelization import correction_level_bkg, correction_level_signal, \
    open_and_create_dir, mkdir_p, tmp_dir, base_dir, name_of_lep
from fit_script import subranges, mass_points, eras
from string_formatter import tex_format

specific_directory = "fourth_jet_veto/tight_wp"
shape = "template"
combine_dir = ojoin(ojoin(base_dir, ojoin(ojoin("combine_tool", shape),
                                          specific_directory)), "out")

lep = [True, False]
mass_points = ["120", "350", "1200"]

getcontext().prec = 0

for l in lep:
    corr_values = OrderedDict()
    for m in mass_points:
        corr_values[m] = OrderedDict()
    for c in correction_level_bkg:
        c = "_".join(c)
        corr_values[m][c] = list()
        input_file = ojoin(ojoin(combine_dir, name_of_lep(l)), c) + ".txt"
        mass, sigma2m, sigma1m, media, sigma1p, sigma2p, obs = loadtxt(
            input_file, unpack=True, skiprows=1)
        appo = [mass, sigma2m, sigma1m, media, sigma1p, sigma2p, obs]
        appo = zip(*appo) # this should transpose the matrix
        for i in range(0, len(appo)):
            corr_values[str(Decimal(appo[i][0]))][c] = appo[i][1:]

    out_filename_txt = ojoin(ojoin(combine_dir, name_of_lep(l)), "z_comparison.txt")
    out_file_txt = open_and_create_dir(out_filename_txt)
    for key, value in corr_values.iteritems():
        out_file_txt.write("# mass point: " + str(key) + "\n")
        for c_appo, vall in value.iteritems():
            out_file_txt.write("{0:21} ".format(c_appo))
            for val in vall:
                out_file_txt.write("{0:9} ".format(val))
            out_file_txt.write("\n")
    out_file_txt.close()

    out_filename_tex = ojoin(ojoin(combine_dir, name_of_lep(l)), "z_comparison.tex")
    out_file_tex = open_and_create_dir(out_filename_tex)
    for key, value in corr_values.iteritems():
        out_file_tex.write("# mass point: " + str(key) + "\n")
        for c_appo, vall, in value.iteritems():
            out_file_tex.write("{0:21} ".format(c_appo))
            vall = ["$ " + str(v) + " $" for v in vall]
            vall = ["{0:11} ".format(v) for v in vall]
            out_file_tex.write(" & ".join(vall) + "\\\\\n")
