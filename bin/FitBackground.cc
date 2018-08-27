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
#include <exception>

#include <boost/program_options.hpp>

#include "TChain.h"
#include "TH1F.h"
#include "TF1.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TVectorF.h"
#include "TFitResult.h"
#include "TFile.h"

#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooArgList.h"
#include "RooWorkspace.h"
#include "RooPlot.h"


#include "tdrstyle.C"
#include "Analysis/Tools/interface/HbbStylesNew.h"
#include "Analysis/Tools/src/classes.h"


namespace bkg_model_fit {
Double_t super_novosibirsk(Double_t x, Double_t p0, Double_t p1, Double_t p2, Double_t p3, Double_t p4, Double_t p5, Double_t p6) {
  Double_t first, second, inside, sigma0;
  const Double_t xi = 2*TMath::Sqrt(TMath::Log(4));
  sigma0 = 2/xi*TMath::ASinH(p5*xi/2);
  Double_t sigma02 = TMath::Power(sigma0, 2);
  first = 0.5*(TMath::Erf(p0*(x - p1)) + 1);
  inside = 1 - (x - p3)*p5/p4 - TMath::Power(x - p3, 2)*p5*p6/p4;
  // std::cout << "1 - (" << x << " - " << p3 << ")*" << p5 << "/" << p4;
  // std::cout <<  " - (" << x << " - " << p3 << ")**2*" << p5 << "*" << p6;
  // std::cout << "/" << p4 << " = " << inside << std::endl;
  second = TMath::Exp(-0.5/sigma02*TMath::Power(TMath::Log(inside), 2) - \
                      0.5*sigma02);
  return p2*first*second;
}



Double_t bukin(Double_t x, Double_t Ap, Double_t Xp, Double_t sp, Double_t rho1, Double_t rho2, Double_t xi) {
  const Double_t sqr2log2 = TMath::Sqrt(2*TMath::Log(2));
  const Double_t log2 = TMath::Log(2);
  Double_t x1 = Xp + sp*sqr2log2*(xi/TMath::Sqrt(xi*xi + 1) - 1);
  Double_t x2 = Xp + sp*sqr2log2*(xi/TMath::Sqrt(xi*xi + 1) + 1);

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


  /**
   * Bukin function with exponential raise on the left.
   */
  Double_t modified_bukin(Double_t x, Double_t Ap, Double_t Xp, Double_t sp, Double_t rho1, Double_t rho2, Double_t xi, Double_t Xt, Double_t tau) {
    if (x < Xt) {
      return TMath::Exp(x/tau)*bukin(Xt, Ap, Xp, sp, rho1, rho2, xi)/TMath::Exp(Xt/tau);
    } else {
      return bukin(x, Ap, Xp, sp, rho1, rho2, xi);
    }
  }
  
}

class undefined_model : public std::exception {
  
public:
  undefined_model(const std::string name) { name_ = name; }
  virtual const char* what() const throw() {
    return ("Requested undefined model with name " + name_).c_str(); }
private:
  std::string name_;
};




std::function<Double_t(Double_t* x, Double_t* p)> choose_model(const std::string model_name,
                                                               RooRealVar& x,
                                                               std::vector<RooRealVar*>& parameters,
                                                               RooAbsPdf*& pdf,
                                                               UInt_t& npars) {
  using std::vector;
  std::function<Double_t(Double_t* x, Double_t* p)> model;
  if (model_name == "super_novosibirsk") {
    model = [&](double* x, double* p) {
      return bkg_model_fit::super_novosibirsk(x[0], p[0],
                                              p[1], p[2],
                                              p[3], p[4], p[5], p[6]);
    };
    
    RooRealVar* p0 = new RooRealVar("p0", "p0", 0, 0.007);
    RooRealVar* p1 = new RooRealVar("p1", "p1", 1400, 2200);
    RooRealVar* p3 = new RooRealVar("p3", "p3", 10, 100);
    RooRealVar* p4 = new RooRealVar("p4", "p4", 10, 100);
    RooRealVar* p5 = new RooRealVar("p5", "p5", 0, 10);
    RooRealVar* p6 = new RooRealVar("p6", "p6", -0.3, -0.0001);
    pdf = new super_novosibirsk("super_novosibirsk", "super_novosibirsk",
                                x, *p0, *p1, *p3, *p4, *p5, *p6);
    vector<RooRealVar*> appo( {p0, p1, p3, p4, p5, p6} );
    for (auto it: appo) {
      parameters.push_back(it);
    }
    
    npars = 7;
    return model;
  } else if (model_name == "bukin") {
    model = [&](double* x, double* p) {
      return bkg_model_fit::bukin(x[0], p[0],
                                  p[1], p[2],
                                  p[3], p[4], p[5]);
    };

    RooRealVar* p1 = new RooRealVar("Xp", "Xp", 10, 500);
    RooRealVar* p3 = new RooRealVar("sp", "sp", 1, 100);
    RooRealVar* p4 = new RooRealVar("rho1", "rho1", 0, 10);
    RooRealVar* p5 = new RooRealVar("rho2", "rho2", 0, 10);
    RooRealVar* p6 = new RooRealVar("xi", "xi", -0.99, 0.99);
    pdf = new bukin("bukin", "bukin",
                                x, *p1, *p3, *p4, *p5, *p6);
    vector<RooRealVar*> variables( {p1, p3, p4, p5, p6} );
    for (unsigned int i = 0; i < 5; i++) {
      parameters.push_back(variables[i]);
    }
    
    npars = 6;
    return model;
  } else if (model_name == "bukin_modified") {
    model = [&](double* x, double* p) {
      return bkg_model_fit::modified_bukin(x[0], p[0],
         p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    };
    npars = 8;
    std::cerr << "Part not implemented! be careful" << std::endl;
    return model;
  } else {
    throw undefined_model(model_name);
  }
}




void set_successful_parameters(const std::string model_name, std::vector<RooRealVar*>& params, const TFitResult* fit_result) {

  if (model_name == "super_novosibirsk") {
    params[0]->setVal(fit_result->GetParams()[0]);
    params[1]->setVal(fit_result->GetParams()[1]);
    params[2]->setVal(fit_result->GetParams()[3]);
    params[3]->setVal(fit_result->GetParams()[4]);
    params[4]->setVal(fit_result->GetParams()[5]);
    params[5]->setVal(fit_result->GetParams()[6]);
  } else if (model_name == "bukin") {
    for (int i = 0; i < 5; i++) {
      params[i]->setVal(fit_result->GetParams()[i+1]);
    }
  } else {
    std::cerr << "Not implemented" << std::endl;
  }

}

void set_style_pave_fit_quality(TPaveText& pave, const Double_t& chi2,
                                const UInt_t& ndof, const Double_t& pvalue) {
  using std::string;
  pave.SetBorderSize(   0 );
  pave.SetFillStyle(    0 );
  pave.SetTextAlign(   12 );
  pave.SetTextSize ( 0.03 );
  pave.SetTextColor(    1 );
  pave.SetTextFont (   42 );

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
    pave.AddText(("#chi^{2}/ndof = " + appo1 + "/" + std::to_string(ndof)).c_str());
    pave.AddText(("p-value: " + appo2).c_str());

  }
  return;
}


void print_result_table(std::ofstream& out_file, const TFitResult* result, const UInt_t& npars, std::string model_name) {
  out_file << "# Model name: " << model_name << "\n";
  out_file << "# Parameter\t\terror\n";
  for (UInt_t i = 0; i < npars; i++) {
    out_file << result->Parameter(i) << "\t" << result->Error(i) << "\n";
  } 
  return;
}



int main(int argc, char* argv[]) {

  namespace bp = boost::program_options;
  using std::string; using std::cout; using std::endl; using std::cerr; using std::vector;
  bp::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help", "Produce help message")
    ("output", bp::value<string>()->required(), "Output ROOT file")
    ("output-table", bp::value<string>(), "File where to store table of fit parameters.")
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
    ("use-integral", "Compute minimization using average integral of function instead of value in center of bin")
    ;
  bp::variables_map vm;
  bp::store(bp::parse_command_line(argc, argv, cmdline_options), vm);

  bool print = false, logy = false, mc = true, print_initial = false, full_hadronic = false;
  bool use_integral = false, print_table = false;
  string out_table_name;
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
  if (vm.count("use-integral")) {
    use_integral = true;
  }
  if (vm.count("output-table")) {
    print_table = true;
    out_table_name = vm["output-table"].as<string>();
  }

  string model_name(vm["model"].as<string>());
  Float_t minx = vm["min-x"].as<Float_t>();
  Float_t maxx = vm["max-x"].as<Float_t>();
  UInt_t bins = vm["bins"].as<UInt_t>();
  string filename_pars = vm["initial-pars"].as<string>();

  UInt_t npars = 0;
  
  
  RooAbsPdf* pdf = nullptr;
  vector<RooRealVar*> variables_to_delete;
  RooRealVar Mass("Mass", "Mass", minx, maxx);

  std::function<Double_t(Double_t* x, Double_t* p)> model = choose_model(model_name, Mass,
                                                                         variables_to_delete,
                                                                         pdf,
                                                                         npars);
    
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
  TF1 model_tf1("model_tf1", model, minx, maxx, npars);
  TH1F data_histo("data_histo", "data_histo", bins, minx, maxx);
  style.InitHist(&data_histo, "M_{12} [GeV]", (string("Events / ") + binnumber + string(" GeV")).c_str());
  data_histo.SetTitle("Background");
  string limit_string = "(Mass > " + std::to_string(minx) + ") && (Mass < " + \
    std::to_string(maxx) + ")";
  chain.Draw("Mass>>data_histo",
             ("Weigth*(" + filter_string + string(" && ") + limit_string + ")").c_str(),
             "E");

  for (UInt_t i = 0; i < init_pars.size(); i++) {
    model_tf1.SetParameter(i, init_pars[i]);
  }
  TF1 clone(model_tf1);
  if (print_initial) {
    clone.SetFillColor(kBlue);
    clone.DrawCopy("same");
  }
  string fit_options("RLMES");
  if (use_integral) {
    fit_options += string("I");
  }
  TFitResultPtr fit_result = data_histo.Fit(&model_tf1, fit_options.c_str(), "", minx, maxx);
  bool success = fit_result->IsValid();

  set_successful_parameters(model_name, variables_to_delete, fit_result.Get());
  if (print_table) {
    std::ofstream out_table_file(out_table_name.c_str());
    print_result_table(out_table_file, fit_result.Get(), npars, model_name);
    out_table_file.close();
  }
  
  if (!print_initial) {
    model_tf1.SetFillColor(kRed);
    model_tf1.DrawClone("same");
  }

  string appo;
  Float_t left = 0, right = 0;
  if (mc) {
    appo = string("MC");
    left = 0.15;
    right = 0.82;
  } else {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << lumi;
    appo = ss.str() + " fb^{-1} (13 TeV)";
    left = 0.15;
    right = 0.79;
  }
  style.CMSPrelim(mc, appo.c_str(), left, right);
  if (logy) {
    pad_histo.SetLogy();
  }
  leg.Draw("same");



  // Plot residuals and calculate chisquare and pvalue
  pad_res.cd();
  TVectorD dx(data_histo.GetNbinsX() - 1), dy(data_histo.GetNbinsX() - 1), \
    ddy(data_histo.GetNbinsX() - 1), ddx(data_histo.GetNbinsX() - 1);
  
  Double_t chi2 = 0;
  for (Int_t i = 1; i < data_histo.GetNbinsX(); i++) {
    Double_t x = data_histo.GetBinCenter(i);
    Double_t exp_value;
    if (success) {
      exp_value = model_tf1.Integral(x - binning/2, x + binning/2)/binning;
    } else {
      // exp_value = 1;
      exp_value = model_tf1.Integral(x - binning/2, x + binning/2)/binning;
    }
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
  TPaveText fit_result_pave(0.65, 0.8, 0.73, 0.85, "NDC");
  set_style_pave_fit_quality(fit_result_pave, chi2, ndof, pvalue);
  fit_result_pave.Draw();

  TFile output_file_(output_file.c_str(), "recreate");
  RooWorkspace w("w");
  w.import(*pdf);
  w.Write();
  cout << "Written workspace on file " << output_file << endl;
  
  c1->Update();
  if (print) {
    for (auto it : print_file) {
      c1->Print(it.c_str());
    }
  }


  for (auto it: variables_to_delete) {
    delete it;
    it = nullptr;
  }
  delete pdf;
  return 0;
}
