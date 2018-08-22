/**
 * @author Fabio Zoratti <fabio.zoratti96@gmail.com> <fabio.zoratti@desy.de> <fabio.zoratti@cern.ch>
 * @date 21/8/2018
 * @file FitBackground.cc
 */


#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <functional>

#include <boost/program_options.hpp>

#include "TChain.h"
#include "TH1F.h"
#include "TF1.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TVectorF.h"

#include "tdrstyle.C"
#include "Analysis/Tools/interface/HbbStylesNew.h"


namespace bkg_model_fit {
Double_t super_novosibirsk(Double_t x, Double_t p0, Double_t p1, Double_t p2, Double_t p3, Double_t p4, Double_t p5, Double_t p6) {
  Double_t first, second, inside, sigma0;
  const Double_t xi = 2*TMath::Sqrt(TMath::Log(4));
  sigma0 = 2/xi*TMath::ASinH(p5*xi/2);
  Double_t sigma02 = TMath::Power(sigma0, 2);
  first = 0.5*(TMath::Erf(p0*(x - p1)) + 1);
  inside = 1 - (x - p3)*p5/p4 - TMath::Power(x - p3, 2)*p5*p6/p4;
  second = TMath::Exp(-0.5/sigma02*TMath::Power(TMath::Log(inside), 2) - \
                      0.5*sigma02);
  return p2*first*second;
}



Double_t bukin(Double_t x, Double_t Ap, Double_t Xp, Double_t sp, Double_t rho1, Double_t rho2, Double_t xi) {
  const Double_t sqr2log2 = TMath::Sqrt(2*TMath::Log(2));
  const Double_t log2 = TMath::Log(2);
  Double_t x1 = Xp + sp*sqr2log2*(xi/TMath::Sqrt(xi + 1) - 1);
  Double_t x2 = Xp + sp*sqr2log2*(xi/TMath::Sqrt(xi + 1) + 1);

  Double_t interno = 0;
  
  if (x < x1) {
    interno = -log2 + rho1*TMath::Power((x - x1)/(Xp - x1), 2);
    Double_t sopra, sotto;
    sopra = xi*TMath::Sqrt(1 + xi*xi)*(x - x1)*sqr2log2;
    sotto = sp*TMath::Log(xi + TMath::Sqrt(1 + xi*xi))*TMath::Power(TMath::Sqrt(1 + xi*xi) - xi, 2);
    interno += sopra/sotto;
  } else if (x > x2) {
    interno = -log2 + rho2*TMath::Power((x - x2)/(Xp - x2), 2);
    Double_t sopra, sotto;
    sopra = xi*TMath::Sqrt(1 + xi*xi)*(x - x2)*sqr2log2;
    sotto = sp*TMath::Log(xi + TMath::Sqrt(1 + xi*xi))*TMath::Power(TMath::Sqrt(1 + xi*xi) + xi, 2);
    interno -= sopra/sotto;
  } else {
    Double_t sopra = TMath::Log(1 + 2*xi*TMath::Sqrt(1 + xi*xi)*(x - Xp)/(sqr2log2 * sp));
    Double_t sotto = TMath::Log(1 + 2*xi*(xi - TMath::Sqrt(xi*xi + 1)));
    Double_t ratio = sopra/sotto;
    interno = -log2*TMath::Power(ratio, 2);
  }

  return Ap*TMath::Exp(interno);
}

  
}





int main(int argc, char* argv[]) {

  namespace bp = boost::program_options;
  using std::string; using std::cout; using std::endl; using std::cerr; using std::vector;
  bp::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help", "Produce help message")
    ("output", bp::value<string>()->required(), "Output ROOT file")
    ("input", bp::value<vector<string>>()->multitoken()->composing()->required(), "Input ROOT files")
    ("print", bp::value<vector<string>>()->multitoken()->composing(), "Optional: files where to print the canvas with fitted function")
    ("log-y", "Set log scale for y axis")
    ("lumi", bp::value<Float_t>(), "Luminosity in fb-1 to be shown. If not provided, the dataset is assumed to be a MC")
    ("min-x", bp::value<Float_t>()->default_value(0), "Min x to choose for the fit")
    ("max-x", bp::value<Float_t>()->default_value(1500), "Max x to choose for the fit")
    ("bins", bp::value<UInt_t>()->required(), "Number of bins for fit")
    ("initial-pars", bp::value<string>()->required(), "Initial parameters for the fit. Give a filename")
    ("model", bp::value<string>()->required(), "Model to choose for the fit")
    ("full-hadronic", "Fits full hadronic channel instead of the leptonic")
    ("print-initial", "Prints the function with the initial parameters")
    ;
  bp::variables_map vm;
  bp::store(bp::parse_command_line(argc, argv, cmdline_options), vm);

  bool print = false, logy = false, mc = true, print_initial = false, full_hadronic = false;
  Float_t lumi = 0;
  if (vm.count("lumi")) {
    mc = false;
    lumi = vm["lumi"].as<Float_t>();
  }

  if (vm.count("log-y")) {
    logy = true;
  }
  
  if (vm.count("help")) {
    cout << cmdline_options << endl;
    return 0;
  }
  if (vm.count("print")) {
    print = true;
  }
  if (vm.count("print-initial")) {
    print_initial = true;
  }
  if (vm.count("full-hadronic")) {
    full_hadronic = true;
  }
  std::function<Double_t(Double_t* x, Double_t* p)> model;
  string model_name(vm["model"].as<string>());
  UInt_t npars = 0;
  if (model_name == "super_novosibirsk") {
    model = [&](double* x, double* p) {
      return bkg_model_fit::super_novosibirsk(x[0], p[0],
                                              p[1], p[2],
                                              p[3], p[4], p[5], p[6]);
    };
    npars = 7;
  } else if (model_name == "bukin") {
    model = [&](double* x, double* p) {
      return bkg_model_fit::bukin(x[0], p[0],
                                  p[1], p[2],
                                  p[3], p[4], p[5]);
    };
    npars = 6;
  } else {
    cerr << "No available model with the name given." << endl;
    return -7;
  }
  
  TChain chain("output_tree");
  
  string filter_string;
  if (full_hadronic) {
    filter_string = "!Leptonic_event";
  } else {
    filter_string = "Leptonic_event";
  }
  vector<string> input_files = vm["input"].as<vector<string>>();
  string output_file = vm["output"].as<string>();
  vector<string> print_file;
  if (print) {
    print_file = vm["print"].as<vector<string>>();
  }
  Float_t minx = vm["min-x"].as<Float_t>();
  Float_t maxx = vm["max-x"].as<Float_t>();
  UInt_t bins = vm["bins"].as<UInt_t>();
  string filename_pars = vm["initial-pars"].as<string>();
  // vector<Float_t> init_pars( {0.001, 1862, 240559, 43, 62, 1, -0.008} );
  vector<Float_t> init_pars;
  std::ifstream infile(filename_pars.c_str());
  {
    Float_t appo;
    while (infile >> appo) {
      init_pars.push_back(appo);
    }
  }
  infile.close();
  if (init_pars.size() != npars) {
    cerr << "The initial parameters given have not the right size." << endl;
    cerr << "I want this size: " << npars << " and i got " << init_pars.size() << endl;
    return -6;
  }
  
  Double_t binning = (maxx - minx) / bins;
  string binnumber;
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << binning;
    binnumber = ss.str();
  }
  
  HbbStylesNew style;
  style.SetStyle();
  setTDRStyle();
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  TCanvas* c1 = style.MakeCanvas("c1", "c1", 800, 800);
  TPad pad_histo("pad1", "Upper pad", 0.0, 0.3, 1.0, 1.0, 0);
  TPad pad_res("pad2", "Lower pad", 0.0, 0.0, 1.0, 0.3, 0);
  pad_histo.Draw();
  pad_res.Draw();
  pad_histo.cd();
  TLegend leg(0.58, 0.63, 0.98, 0.93);
  style.SetLegendStyle(&leg);    
  
  for (auto it: input_files) {
    chain.Add(it.c_str());
  }
  TF1 super_novosibirsk("super_novosibirsk", model, minx, maxx, npars);
  TH1F data_histo("data_histo", "data_histo", bins, minx, maxx);
  style.InitHist(&data_histo, "M_{12} [GeV]", (string("Events / ") + binnumber + string(" GeV")).c_str());
  data_histo.SetTitle("Background");
  string limit_string = "(Mass > " + std::to_string(minx) + ") && (Mass < " + \
    std::to_string(maxx) + ")";
  chain.Draw("Mass>>data_histo",
             (filter_string + string(" && ") + limit_string).c_str(),
             "");

  for (UInt_t i = 0; i < init_pars.size(); i++) {
    super_novosibirsk.SetParameter(i, init_pars[i]);
  }
  TF1 clone(super_novosibirsk);
  if (print_initial) {
    clone.SetFillColor(kBlue);
    clone.DrawCopy("same");
  }
  data_histo.Fit(&super_novosibirsk, "RLME", "", minx, maxx);
  if (!print_initial) {
    super_novosibirsk.SetFillColor(kRed);
    super_novosibirsk.DrawClone("same");
  }

  string appo;
  if (mc) {
    appo = string("MC");
  } else {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << lumi;
    appo = ss.str() + " fb^{-1} (13 TeV)";
  }
  style.CMSPrelim(mc, appo.c_str(), 0.15, 0.79);
  if (logy) {
    c1->SetLogy();
  }
  leg.Draw("same");
  
  pad_res.cd();
  TVectorD dx(data_histo.GetNbinsX() - 1), dy(data_histo.GetNbinsX() - 1), \
    ddy(data_histo.GetNbinsX() - 1), ddx(data_histo.GetNbinsX() - 1);
  
  Double_t chi2 = 0;
  for (Int_t i = 1; i < data_histo.GetNbinsX(); i++) {
    Double_t x = data_histo.GetBinCenter(i);
    Double_t exp_value = super_novosibirsk.Integral(x - binning/2, x + binning/2)/binning;
    Double_t divisor = TMath::Sqrt(exp_value);
    Double_t y = (data_histo.GetBinContent(i) - exp_value)/divisor;
    
    dx[i - 1] = x;
    dy[i - 1] = y;
    ddy[i - 1] = 1;
    ddx[i - 1] = binning / 2;
    chi2 += y*y;
  }

  cout << "Chi2: " << chi2 << endl;
  UInt_t ndof = bins - npars;
  Float_t pvalue = TMath::Prob(chi2, ndof);
  cout << "ndof: " << ndof << endl;
  cout << "pvalue: " << pvalue << endl;


  TGraphErrors graph(dx, dy, ddx, ddy);
  graph.Draw("AP");
  graph.GetXaxis()->SetLimits(minx, maxx);
  style.InitGraph(&graph, "M_{12} [GeV]", "#frac{Data - Fit}{#sqrt{Fit}}");

  pad_histo.cd();
  TPaveText fit_result(0.65, 0.8, 0.73, 0.85, "NDC");
  fit_result.SetBorderSize(   0 );
  fit_result.SetFillStyle(    0 );
  fit_result.SetTextAlign(   12 );
  fit_result.SetTextSize ( 0.03 );
  fit_result.SetTextColor(    1 );
  fit_result.SetTextFont (   42 );

  {
    string appo1, appo2;
    if (chi2 != chi2) {
      appo1 = "nan";
    }
    if (pvalue != pvalue) {
      appo2 = "nan";
    }
    if (chi2 == chi2 && pvalue == pvalue) {
      std::stringstream ss1, ss2;
      ss1 << std::fixed << std::setprecision(1) << chi2;
      appo1 = ss1.str();
      ss2 << std::fixed << std::setprecision(3) << pvalue;
      appo2 = ss2.str();
    }
    fit_result.AddText(("#chi^{2}/ndof = " + appo1 + "/" + std::to_string(ndof)).c_str());
    fit_result.AddText(("p-value: " + appo2).c_str());
    fit_result.Draw();
  }
  
  c1->Update();
  if (print) {
    for (auto it : print_file) {
      c1->Print(it.c_str());
    }
  }

  return 0;
}
