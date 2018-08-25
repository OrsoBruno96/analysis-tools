#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from os.path import join as ojoin
from pylab import loadtxt
from uncertainties import ufloat
from collections import OrderedDict

from settings_parallelization import correction_level_bkg, correction_level_signal, \
    open_and_create_dir, mkdir_p, tmp_dir, base_dir
from fit_script import subranges, mass_points, eras
from string_formatter import tex_format


specific_directory = "fourth_jet_veto/medium_wp"
fit_dir = ojoin(ojoin(base_dir, "fit/bkg"), specific_directory)
tables_dir = ojoin(fit_dir, "tables")


if __name__ == "__main__":
    for l, apppo in subranges.iteritems():
        for sub in apppo:
            corr_values = OrderedDict()
            out_filename = "_".join(["compare", "bkg", l] + eras + [sub['name'], ]) + ".txt"
            out_filename = ojoin(tables_dir, out_filename)
            for c in correction_level_bkg:
                c = "_".join(c)
                input_name = "_".join(["fit", "bkg", l] + eras + [c, sub['name'], ]) + ".txt"
                input_name = ojoin(tables_dir, input_name)
                params, errors = loadtxt(input_name, unpack=True)
                values = list()
                for p, e in zip(params, errors):
                    values.append(ufloat(p, e))
                corr_values[c] = values
            out_file = open_and_create_dir(out_filename)
            out_file.write("# Parameters compared for correction level. Function: " +
                           sub['model'] + "\n")
            for key, value in corr_values.iteritems():
                out_file.write("{0:21} ".format(key))
                for v in value:
                    out_file.write("{0:24} ".format(str(v)))
                out_file.write("\n")
            out_file.close()
            out_filename_latex = out_filename[:-3] + "tex"
            out_file_latex = open_and_create_dir(out_filename_latex)
            for key, value in corr_values.iteritems():
                out_file_latex.write(" & ".join(
                    [key, ] + [tex_format(v) for v in value]
                ))
                out_file_latex.write(" \\\\\n")
            out_file_latex.close()


    for l, apppo in mass_points.iteritems():
        for mass, sub in apppo.iteritems():
            corr_values = OrderedDict()
            out_filename = "_".join(["compare", "mc", l, mass]) + ".txt"
            out_filename = ojoin(tables_dir, out_filename)
            for c in correction_level_signal:
                c = "_".join(c)
                input_name = "_".join(["fit", "mc", l, c, mass]) + ".txt"
                input_name = ojoin(tables_dir, input_name)
                params, errors = loadtxt(input_name, unpack=True)
                values = list()
                for p, e in zip(params, errors):
                    values.append(ufloat(p, e))
                corr_values[c] = values
            out_file = open_and_create_dir(out_filename)
            out_file.write("# Parameters compared for correction level. Function: " +
                           sub['model'] + "\n")
            for key, value in corr_values.iteritems():
                out_file.write("{0:35} ".format(key))
                for v in value:
                    out_file.write("{0:24} ".format(str(v)))
                out_file.write("\n")
            out_file.close()
            out_filename_latex = out_filename[:-3] + "tex"
            out_file_latex = open_and_create_dir(out_filename_latex)
            for key, value in corr_values.iteritems():
                out_file_latex.write(" & ".join(
                    [key, ] + [tex_format(v) for v in value]
                ))
                out_file_latex.write(" \\\\\n")
            out_file_latex.close()
