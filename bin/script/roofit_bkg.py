#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas, TGraph, TChain
from ROOT import RooRealVar, RooGenericPdf, RooArgSet, RooArgList, RooDataSet, RooAbsPdf
from ROOT.RooFit import bindFunction, Binning

from math import erf
from array import array
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



input_dir_bkg = "/nfs/dust/cms/user/zorattif/output/raw_files/bkg/no_reranking/medium_wp"
correction_level = ["nothing_true",
                    "regression_true",
                    "regression_fsr_true",
                    "regression_fsr_true",
]
lep = [True, False]
ch = TChain("output_tree")
ch.Add(os.path.join(input_dir_bkg, "bkg_*_nothing_true.root"))
tree = ch.CopyTree("Leptonic_event")


Mass = RooRealVar("Mass", "Mass", 120, 600)
p0 = RooRealVar("p0", "p0", 0, 0.005)
p1 = RooRealVar("p1", "p1", 1600, 2000)
p3 = RooRealVar("p3", "p3", 30, 50)
p4 = RooRealVar("p4", "p4", 40, 80)
p5 = RooRealVar("p5", "p5", 0, 5)
p6 = RooRealVar("p6", "p6", -0.05, -0.001)

a = ROOT.RooRealVar("a", "a", 1)
b = ROOT.RooRealVar("b", "b", 2, -10, 10)
y = ROOT.RooRealVar("y", "y", -10, 10)

# ROOT.RooClassFactory.makePdf("super_novosibirsk", "x,p0,p1,p2,p3,p4,p5,p6")
print("Compiling")
ROOT.gROOT.ProcessLineSync(".x super_novosibirsk.cxx+")

# super_novosibirisk_roo = RooGenericPdf(
#     "super_novosibirisk_roo",
#     "(0.5*(1+TMath::Erf((Mass - p0)/p1)))*1",
#     RooArgList(Mass, p0, p1))
print("Getting func")
super_novosibirsk = ROOT.super_novosibirsk(
    "super_novosibirsk", "super_novosibirsk", Mass, p0, p1, p3, p4, p5, p6)

# frames = [Mass.frame() for i in range(0, len(correction_level)*len(lep))]
frame = Mass.frame()
print("Retrieving data from tree")
data = RooDataSet("Mass", "Mass", tree, RooArgSet(Mass))
data.plotOn(frame, Binning(1000))
# input("Intermediate step: ")
print("Plotting and fitting stuff")
# super_novosibirsk.plotOn(frames[i])

fitResult = super_novosibirsk.fitTo(
    data, ROOT.RooFit.Save(), ROOT.RooFit.PrintLevel(-1))
# fitResult.plotOn(frame)
super_novosibirsk.plotOn(frame)
frame.Draw()

input("asd")
