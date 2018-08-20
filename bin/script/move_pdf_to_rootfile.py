#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import gROOT, TFile, RooWorkspace
from settings_parallelization import mass_points_signal, name_of_lep, correction_level_bkg

from os.path import join as ojoin

import ROOT

base_dir = "/nfs/dust/cms/user/zorattif/output"
fit_dir = ojoin(base_dir, "fit/bkg/no_reranking/medium_wp")
out_dir = ojoin(base_dir, "combine_tool/shape/no_reranking/medium_wp")

gROOT.ProcessLineSync(".x super_novosibirsk.cxx")
lep = [True, False]

for c in correction_level_bkg:
    c = "_".join(c)
    for l in lep:
        for m in mass_points_signal:
            m = m['mass']
            function_filename = ojoin(fit_dir, c + ".root")
            output_filename = "_".join(["combine", name_of_lep(l), m, c]) + ".root"
            output_filename = ojoin(out_dir, output_filename)
            function_file = TFile(function_filename, "read")
            workspace = function_file.Get("w")
            out_file = TFile(output_filename, "update")
            workspace.Write()
            function_file.Close()
            out_file.Close()
