#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas, TGraph, TChain
from ROOT import RooRealVar, RooGenericPdf, RooArgSet, RooArgList, RooDataSet, RooAbsPdf, \
    RooWorkspace
from ROOT.RooFit import bindFunction, Binning

from hbbstyle import create_style, init_hist, CMS_prelim

import ROOT
import os

subranges = [
    (120, 600),
    (400, 1100),
    (800, 1600),
]

bbins = [
    1000,
    300,
    50,
]


base_dir = "/nfs/dust/cms/user/zorattif/output"
input_dir_bkg = os.path.join(base_dir, "raw_files/bkg/no_reranking/medium_wp")
output_dir_fitted = os.path.join(base_dir, "fit/bkg/no_reranking/medium_wp")

def prendi_parametri(funzione):
    params = list()
    errors = list()
    for i in range(0, funzione.GetNpar()):
        params.append(funzione.GetParameter(i))
        errors.append(funzione.GetParError(i))
    return params, errors


def scrivi_parametri(params, errors, fileout):
    for p, e in zip(params, errors):
        fileout.write(str(p) + "\t" + str(e) + "\t")


correction_level_bkg = [
    "nothing_false",
    "regression_false",
    "fsr_false",
    "regression_fsr_false",
    "nothing_true",
    "regression_true",
    "fsr_true",
    "regression_fsr_true",
]

lep = [True, False]
eras = ["C", "D", "E"]

print("Compiling")
w = RooWorkspace("w")
ROOT.gROOT.ProcessLineSync(".x super_novosibirsk.cxx+")

# frames = [Mass.frame() for i in range(0, len(correction_level)*len(lep))]

for c in correction_level_bkg:
    print("Correction level: " + c)
    ch = TChain("output_tree")
    for e in eras:
        ch.Add(os.path.join(input_dir_bkg, "_".join(["bkg", e, c]) + ".root"))
    tree = ch.CopyTree("Leptonic_event")
    Mass = RooRealVar("Mass", "Mass", 120, 600)
    p0 = RooRealVar("p0", "p0", 0, 0.005)
    p1 = RooRealVar("p1", "p1", 1600, 2000)
    p3 = RooRealVar("p3", "p3", 30, 50)
    p4 = RooRealVar("p4", "p4", 40, 80)
    p5 = RooRealVar("p5", "p5", 0, 5)
    p6 = RooRealVar("p6", "p6", -0.05, -0.001)
    

    print("Getting func")
    super_novosibirsk = ROOT.super_novosibirsk(
        "super_novosibirsk", "super_novosibirsk", Mass, p0, p1, p3, p4, p5, p6)
    frame = Mass.frame()
    print("Retrieving data from tree")
    data = RooDataSet("Mass", "Mass", tree, RooArgSet(Mass))
    data.plotOn(frame, Binning(100))
    print("Plotting and fitting stuff")
    # super_novosibirsk.plotOn(frames[i])
    # fitResult = super_novosibirsk.fitTo(
    #     data, ROOT.RooFit.Save(), ROOT.RooFit.PrintLevel(-1))
    # fitResult.plotOn(frame)
    super_novosibirsk.plotOn(frame)

    # Now i need to do some tricks because some idiot decided to call a method
    # as a python builtin.
    getattr(w, 'import')(super_novosibirsk)
    w.Print()
    output_filename = c
    output_filename = os.path.join(output_dir_fitted, c + ".root")
    # out_file = TFile(os.path.join(output_dir_fitted, c + ".root"), "recreate")
    w.writeToFile(output_filename, True)
    # out_file.Close()
    frame.Draw()

input("asd")
