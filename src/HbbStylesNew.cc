/*
 * HbbStylesNew.cpp
 *
 *  Created on: Sep 25, 2016
 *      Author: shevchen
 */

#include "Analysis/Tools/interface/HbbStylesNew.h"

double HbbStylesNew::textSize_ = 0.05;

void HbbStylesNew::SetStyle()
{
  TStyle *HbbStyle = new TStyle("Htt-Style","The Perfect Style for Plots ;-)");
  gStyle = HbbStyle;

  // Canvas
  HbbStyle->SetCanvasColor     (0);
  HbbStyle->SetCanvasBorderSize(10);
  HbbStyle->SetCanvasBorderMode(0);
  HbbStyle->SetCanvasDefH      (700);
  HbbStyle->SetCanvasDefW      (700);
  HbbStyle->SetCanvasDefX      (100);
  HbbStyle->SetCanvasDefY      (100);

  // color palette for 2D temperature plots
  HbbStyle->SetPalette(1,0);

  // Pads
  HbbStyle->SetPadColor       (0);
  HbbStyle->SetPadBorderSize  (10);
  HbbStyle->SetPadBorderMode  (0);
  HbbStyle->SetPadBottomMargin(0.1);
  HbbStyle->SetPadTopMargin   (0.05);
  HbbStyle->SetPadLeftMargin  (0.1);
  HbbStyle->SetPadRightMargin (0.05);
  HbbStyle->SetPadGridX       (0);
  HbbStyle->SetPadGridY       (0);
  HbbStyle->SetPadTickX       (1);
  HbbStyle->SetPadTickY       (1);

  // Frames
  HbbStyle->SetLineWidth(3);
  HbbStyle->SetFrameFillStyle ( 0);
  HbbStyle->SetFrameFillColor ( 0);
  HbbStyle->SetFrameLineColor ( 1);
  HbbStyle->SetFrameLineStyle ( 0);
  HbbStyle->SetFrameLineWidth ( 2);
  HbbStyle->SetFrameBorderSize(10);
  HbbStyle->SetFrameBorderMode( 0);

  // Histograms
  HbbStyle->SetHistFillColor(2);
  HbbStyle->SetHistFillStyle(0);
  HbbStyle->SetHistLineColor(1);
  HbbStyle->SetHistLineStyle(0);
  HbbStyle->SetHistLineWidth(3);
  HbbStyle->SetNdivisions(505);

  // Functions
  HbbStyle->SetFuncColor(1);
  HbbStyle->SetFuncStyle(0);
  HbbStyle->SetFuncWidth(2);

  // Various
  HbbStyle->SetMarkerStyle(20);
  HbbStyle->SetMarkerColor(kBlack);
  HbbStyle->SetMarkerSize (1.4);

  HbbStyle->SetTitleBorderSize(0);
  HbbStyle->SetTitleFillColor (0);
  HbbStyle->SetTitleX         (0.2);

  HbbStyle->SetTitleSize  (0.055,"X");
  HbbStyle->SetTitleOffset(1.200,"X");
  HbbStyle->SetLabelOffset(0.005,"X");
  HbbStyle->SetLabelSize  (0.050,"X");
  HbbStyle->SetLabelFont  (42   ,"X");

  HbbStyle->SetStripDecimals(kFALSE);
  HbbStyle->SetLineStyleString(11,"20 10");

  HbbStyle->SetTitleSize  (0.055,"Y");
  HbbStyle->SetTitleOffset(1.000,"Y");
  HbbStyle->SetLabelOffset(0.010,"Y");
  HbbStyle->SetLabelSize  (0.050,"Y");
  HbbStyle->SetLabelFont  (42   ,"Y");

  HbbStyle->SetTextSize   (0.055);
  HbbStyle->SetTextFont   (42);

  HbbStyle->SetStatFont   (42);
  HbbStyle->SetTitleFont  (42);
  HbbStyle->SetTitleFont  (42,"X");
  HbbStyle->SetTitleFont  (42,"Y");

  HbbStyle->SetOptStat    (0);
  return;
}

TCanvas* HbbStylesNew::MakeCanvas(const char* name, const char *title, int dX, int dY)
{
  // Start with a canvas
  TCanvas *canvas = new TCanvas(name,title,0,0,dX,dY);
  // General overall stuff
  canvas->SetFillColor      (0);
  canvas->SetBorderMode     (0);
  canvas->SetBorderSize     (10);
  // Set margins to reasonable defaults
  canvas->SetLeftMargin     (0.14);
  canvas->SetRightMargin    (0.05);
  canvas->SetTopMargin      (0.08);
  canvas->SetBottomMargin   (0.15);
  // Setup a frame which makes sense
  canvas->SetFrameFillStyle (0);
  canvas->SetFrameLineStyle (0);
  canvas->SetFrameBorderMode(0);
  canvas->SetFrameBorderSize(10);
  canvas->SetFrameFillStyle (0);
  canvas->SetFrameLineStyle (0);
  canvas->SetFrameBorderMode(0);
  canvas->SetFrameBorderSize(10);

  return canvas;
}

void HbbStylesNew::InitSubPad(TPad* pad, int i)
{
  pad->cd(i);
  TPad *tmpPad = (TPad*) pad->GetPad(i);
  tmpPad->SetLeftMargin  (0.18);
  tmpPad->SetTopMargin   (0.05);
  tmpPad->SetRightMargin (0.07);
  tmpPad->SetBottomMargin(0.15);
  return;
}

void HbbStylesNew::InitSignal(TH1 *hist)
{
  hist->SetFillStyle(0.);
  hist->SetLineStyle(11);
  hist->SetLineWidth(2.);
  hist->SetLineColor(kBlue+3);
}

void HbbStylesNew::InitHist(TH1 *hist, const char *xtit, const char *ytit, int color, int style)
{
  hist->SetXTitle(xtit);
  hist->SetYTitle(ytit);
  hist->SetLineColor(color);
  hist->SetLineWidth(    2.);
  hist->SetFillColor(color);
  hist->SetFillStyle(style);
  //  hist->GetYaxis()->SetRangeUser(0.1,100000);
  hist->SetTitleSize  (0.055,"Y");
  hist->SetTitleOffset(1.200,"Y");
  hist->SetLabelOffset(0.014,"Y");
  hist->SetLabelSize  (0.040,"Y");
  hist->SetLabelFont  (42   ,"Y");
  hist->SetTitleSize  (0.055,"X");
  hist->SetTitleOffset(1.210,"X");
  hist->SetLabelOffset(0.014,"X");
  hist->SetLabelSize  (0.050,"X");
  hist->SetLabelFont  (42   ,"X");
  hist->SetMarkerStyle(20);
  hist->SetMarkerColor(color);
  hist->SetMarkerSize (0.6);
  hist->GetYaxis()->SetTitleFont(42);
  hist->GetXaxis()->SetTitleFont(42);
  hist->SetTitle("");
  return;
}

void HbbStylesNew::InitData(TH1* hist)
{
  hist->SetMarkerStyle(20.);
  hist->SetMarkerSize (1.3);
  hist->SetLineWidth  ( 2.);
}

void HbbStylesNew::SetLegendStyle(TLegend* leg)
{
  leg->SetFillStyle (0);
  leg->SetFillColor (0);
  leg->SetBorderSize(0);
  leg->SetTextFont(42);
  leg->SetTextSize(textSize_);
}

void HbbStylesNew::CMSPrelim(bool MC, const char* dataset, double lowX, double lowY) {
  cmsprel  = new TPaveText(lowX, lowY+0.06, lowX+0.30, lowY+0.16, "NDC");
  cmsprel->SetBorderSize(   0 );
  cmsprel->SetFillStyle(    0 );
  cmsprel->SetTextAlign(   12 );
  cmsprel->SetTextSize ( 0.05 );
  cmsprel->SetTextColor(    1 );
  cmsprel->SetTextFont (   62 );
  if (!MC) cmsprel->AddText("CMS           ");
  else       cmsprel->AddText("CMS Simulation");
  cmsprel->Draw();

  float lowXlumi  = lowX+0.59;
  float lowYlumi  = lowY+0.158;
  float highXlumi = lowX+0.72;
  float highYlumi = lowY+0.161;

  if (!MC) {
    lowXlumi  = lowX+0.48;
    lowYlumi  = lowY+0.155;
    highXlumi = lowX+0.75;
    highYlumi = lowY+0.232;
  }

  lumi     = new TPaveText(lowXlumi, lowYlumi, highXlumi, highYlumi, "NDC"); 

  lumi->SetBorderSize(   0 );
  lumi->SetFillStyle(    0 );
  lumi->SetTextAlign(   12 );
  lumi->SetTextSize ( 0.05 );
  lumi->SetTextColor(    1 );
  lumi->SetTextFont (   62 );
  if ( !MC ) lumi->AddText(dataset);
  else lumi->AddText("13 TeV");
  lumi->Draw();

  wip     = new TPaveText(lowX-0.005, lowY+0.05, lowX+0.4, lowY+0.06, "NDC");
  wip->SetBorderSize(   0 );
  wip->SetFillStyle(    0 );
  wip->SetTextAlign(   12 );
  wip->SetTextSize ( 0.04 );
  wip->SetTextColor(    1 );
  wip->SetTextFont (   52 );
  wip->AddText("Work in Progress");
  wip->Draw();
}


void HbbStylesNew::plotchannel(TString channel) {
  TLatex * tex = new TLatex(0.2,0.94,channel);
  tex->SetNDC();
  tex->SetTextSize(0.06);
  tex->SetLineWidth(2);
  tex->Draw();
}

HbbStylesNew::HbbStylesNew() {
  tex = nullptr;
  wip = nullptr;
  lumi = nullptr;
  cmsprel = nullptr;
}

HbbStylesNew::~HbbStylesNew() {
  delete tex;
  delete wip;
  delete lumi;
  delete cmsprel;
}


void HbbStylesNew::InitGraph(TGraphErrors* hist, const char* xtit, const char* ytit, int color, int style) {
  hist->GetXaxis()->SetTitle(xtit);
  hist->GetYaxis()->SetTitle(ytit); 
  hist->SetLineColor(color);
  hist->SetLineWidth(    2.);
  hist->SetFillColor(color);
  hist->SetFillStyle(style);
  //  hist->GetYaxis()->SetRangeUser(0.1,100000);
  hist->GetYaxis()->SetTitleSize  (0.12);
  hist->GetYaxis()->SetTitleOffset(0.5);
  hist->GetYaxis()->SetLabelOffset(0.01);
  hist->GetYaxis()->SetLabelSize(0.09);
  hist->GetYaxis()->SetLabelFont(42);
  hist->GetXaxis()->SetTitleSize  (0.12);
  hist->GetXaxis()->SetTitleOffset(1.300);
  hist->GetXaxis()->SetLabelOffset(0.014);
  hist->GetXaxis()->SetLabelSize  (0.09);
  hist->GetXaxis()->SetLabelFont  (42   );

  hist->SetMarkerStyle(20);
  hist->SetMarkerColor(color);
  hist->SetMarkerSize (0.6);
  hist->GetYaxis()->SetTitleFont(42);
  hist->GetXaxis()->SetTitleFont(42);
  hist->SetTitle("");

  
}

