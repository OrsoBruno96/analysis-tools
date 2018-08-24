#!/usr/bin/env python2.7
# -*- coding:utf-8 -*-

from ROOT import TH1F, TFile, TMath, TF1, TH1F, TCanvas, TGraph, TChain
from ROOT import RooRealVar, RooGenericPdf, RooArgSet, RooArgList, RooDataSet, RooAbsPdf, \
    RooWorkspace, RooDataHist
from ROOT.RooFit import bindFunction, Binning, Import
from os.path import join as ojoin

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
specific_directory = "fourth_jet_veto/medium_wp"
input_dir_bkg = ojoin(base_dir, ojoin("raw_files/bkg", specific_directory))
output_dir_fitted = ojoin(base_dir, ojoin("fit/bkg", specific_directory))

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


initial = [
    [0.001, 1862, 43, 62, 1.5, -0.008],
    [0.001, 1862, 43, 62, 1.5, -0.008],
    [0.001, 1862, 43, 62, 1.5, -0.008],
    [0.001, 1862, 43, 62, 1.5, -0.008],
    [0.001, 1862, 43, 62, 1.5, -0.008],
    [0.001, 1862, 43, 62, 1.5, -0.008],
    [0.001, 1862, 43, 62, 1.5, -0.008],
    [0.001, 1862, 43, 62, 1.5, -0.008],
]


class Chi2Ndf:

    def __init__(self, chi2, fridum):
        self.chi2 = chi2
        self.ndof = fridum

    def __str__(self):
        return "Chi2: " + str(self.chi2) + "\tndof: " + str(self.ndof)

    def pvalue(self):
        return TMath.Prob(self.chi2, self.ndof)



def chiQuadro(frame, curvename, histname, nFitParam=0):
    # blind_lowEdge, blind_highEdge, chi2_lowEdge, chi2_highEdge):
    curve = frame.getCurve(curvename)
    hist  = frame.getHist(histname)
    # curve_xstart = CurveRange(curve).first
    # curve_xstop = CurveRange(curve).second
    curve_xstart = 120
    curve_xstop = 600
    
    x = 0
    y = 0
    chi2 = 0
    nbins = 0
    for i in range(0, hist.GetN()):
	hist.GetPoint(i, x, y)
	if (x < curve_xstart or x > curve_xstop):
            continue
	if (x > blind_lowEdge and x < blind_highEdge):
            continue
	if (x < chi2_lowEdge or x > chi2_highEdge):
            continue
	eyl = hist.GetEYlow()[i] 
	eyh = hist.GetEYhigh()[i] 
	exl = hist.GetEXlow()[i] 
	exh = hist.GetEXhigh()[i] 
	curve_yavg = curve.average(x-exl,x+exh)
	pull = 0.
	if (y != 0):
	    pull = ((y - curve_yavg)/eyl) if (y > curve_yavg) else ((y - curve_yavg)/eyh)
	    chi2 += pull*pull
	    nbins += 1
	return Chi2Ndf(chi2, nbins - nFitParam)


lep = [True, ]
# eras = ["C", "D", "E", "F"]
eras = ["C", ]

print("Compiling")
ROOT.gROOT.ProcessLineSync(".x super_novosibirsk.cxx+")
ranges = range(0, len(correction_level_bkg)*len(lep))
Mass = RooRealVar("Mass", "Mass", 120, 600)
frames = [Mass.frame() for i in ranges]
frames2 = [Mass.frame() for i in ranges]
workspaces = [RooWorkspace("w") for i in ranges]
canvases = [TCanvas("canv" + str(i), "canv" + str(i), 800, 800) for i in ranges]
canvases2 = [TCanvas("2canv" + str(i), "2canv" + str(i), 800, 800) for i in ranges]



for c, init, frame, frame2, w, canv, canv2 in zip(
        correction_level_bkg, initial, frames, frames2, workspaces, canvases, canvases2):
    print("Correction level: " + c)
    canv.cd()
    ch = TChain("output_tree")
    for e in eras:
        ch.Add(ojoin(input_dir_bkg, "_".join(["bkg", e, c]) + ".root"))
    tree = ch.CopyTree("Leptonic_event")
    p0 = RooRealVar("p0", "p0", 0, 0.007)
    p1 = RooRealVar("p1", "p1", 1400, 2200)
    p3 = RooRealVar("p3", "p3", 10, 100)
    p4 = RooRealVar("p4", "p4", 10, 100)
    p5 = RooRealVar("p5", "p5", 0, 10)
    p6 = RooRealVar("p6", "p6", -0.3, -0.0001)
    for p, iii in zip([p0, p1, p3, p4, p5, p6], init):
        p.setVal(iii)

    print("Getting func")
    super_novosibirsk = ROOT.super_novosibirsk(
        "super_novosibirsk", "super_novosibirsk", Mass, p0, p1, p3, p4, p5, p6)
    frame = Mass.frame()
    print("Retrieving data from tree")
    # data = RooDataSet("Mass", "Mass", tree, RooArgSet(Mass))
    # data.plotOn(frame, Binning(80))
    histo = TH1F(c, c, 300, 120, 600)
    tree.Draw("Mass>>" + c, "")
    data = RooDataHist(c, c, RooArgList(Mass), Import(histo))
    data.plotOn(frame)
    print("Plotting and fitting stuff")
    # super_novosibirsk.plotOn(frame)
    # frame.Draw()
    # input("A morte beppe")
    try:
        fitResult = super_novosibirsk.fitTo(
            data, ROOT.RooFit.Save(), ROOT.RooFit.PrintLevel(-1))
        print("-Log(L): " + str(fitResult.minNll()))
        # print("NDOF: " + str(fitResult.Ndf()))
    except KeyboardInterrupt:
        print("Stopped the fitting")
    # fitResult.plotOn(frame)
    super_novosibirsk.plotOn(frame)

    # Now i need to do some tricks because some idiot decided to call a method
    # as a python builtin.
    hresid = frame.pullHist()
    getattr(w, 'import')(Mass)    
    getattr(w, 'import')(super_novosibirsk)
    # w.Print()
    output_filename = c
    output_filename = ojoin(output_dir_fitted, c + ".root")
    w.writeToFile(output_filename, True)
    # print(chiQuadro(frame, "super_novosibirsk", c, 6))
    frame.Draw()
    canv2.cd()
    frame2.addPlotable(hresid, "P")
    frame2.Draw()
    print("Real chi2: " + str(frame.chiSquare()))
    print("Cancro")
    input("Fermati")

input("asd")
