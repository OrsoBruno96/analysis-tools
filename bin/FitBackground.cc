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



class ModelFunction {
public:
  ModelFunction(const std::string& x_var_name, const Double_t& min_x, const Double_t& max_x) :
    min_x_(min_x), max_x_(max_x), x_(x_var_name.c_str(), x_var_name.c_str(), min_x, max_x ) { pdf_ = nullptr; }
  ~ModelFunction() { delete pdf_; }
  virtual std::function<Double_t(Double_t* x, Double_t* p)> evaluate_normalized() const = 0;
  virtual std::function<Double_t(Double_t* x, Double_t*p)> evaluate_floating() const = 0;
  virtual std::string get_name() const = 0;
  virtual UInt_t get_par_number_normalized() const = 0;
  virtual UInt_t get_par_number() const = 0;
  virtual void pick_parameters(const TFitResult* fit_result);
  RooAbsPdf* get_pdf() { return pdf_; }
  TF1 create_normalized_tf1(const std::string& name, const Double_t& minx, const Double_t& maxx) const;
  TF1 create_tf1(const std::string& name, const Double_t& minx, const Double_t& maxx) const;

protected:
  std::vector<RooRealVar> parameters_;
  Double_t min_x_;
  Double_t max_x_;
  RooRealVar x_;
  RooAbsPdf* pdf_;
};


TF1 ModelFunction::create_normalized_tf1(const std::string& name, const Double_t& minx, const Double_t& maxx) const {
  return TF1(name.c_str(), evaluate_normalized(), minx, maxx, get_par_number_normalized());
}

TF1 ModelFunction::create_tf1(const std::string& name, const Double_t& minx, const Double_t& maxx) const {
  return TF1(name.c_str(), evaluate_floating(), minx, maxx, get_par_number());
}

void ModelFunction::pick_parameters(const TFitResult* fit_result) {
  for (UInt_t i = 0; i < get_par_number_normalized(); i++) {
    parameters_[i].setVal(fit_result->Parameter(i));
  }
}


class SuperNovosibirsk : public ModelFunction {
public:
  SuperNovosibirsk(const std::string& x_var_name, const Double_t& min_x, const Double_t& max_x);
  std::function<Double_t(Double_t* x, Double_t* p)> evaluate_normalized() const;
  std::function<Double_t(Double_t* x, Double_t* p)> evaluate_floating() const;
  std::string get_name() const { return "super_novosibirsk"; };
  UInt_t get_par_number() const { return 7; }
  UInt_t get_par_number_normalized() const { return 6; }
private:
};


std::function<Double_t(Double_t* x, Double_t* p)> SuperNovosibirsk::evaluate_floating() const {
  return [&](double* x, double* p) {
    return bkg_model_fit::super_novosibirsk(x[0], p[0], p[1], p[2],
                                            p[3], p[4], p[5], p[6]);
  };
}

std::function<Double_t(Double_t* x, Double_t* p)> SuperNovosibirsk::evaluate_normalized() const {
  return [&](double* x, double* p) {
    // fix this shit
    return bkg_model_fit::super_novosibirsk(x[0], p[0], p[1], 1,
                                            p[2], p[3], p[4], p[5]);
  };
}


SuperNovosibirsk::SuperNovosibirsk(const std::string& x_var_name, const Double_t& min_x, const Double_t& max_x) :
  ModelFunction(x_var_name, min_x, max_x) {
  parameters_.push_back(RooRealVar("p0", "p0", 0, 0.007));
  parameters_.push_back(RooRealVar("p1", "p1", 1400, 2200));
  parameters_.push_back(RooRealVar("p3", "p3", 10, 100));
  parameters_.push_back(RooRealVar("p4", "p4", 10, 100));
  parameters_.push_back(RooRealVar("p5", "p5", 0, 10));
  parameters_.push_back(RooRealVar("p6", "p6", -0.3, -0.0001));

  pdf_ = new super_novosibirsk("super_novosibirsk", "super_novosibirsk",
                               x_, parameters_[0], parameters_[1], parameters_[2],
                               parameters_[3], parameters_[4], parameters_[5]);
}


class Bukin : public ModelFunction {
public:
  Bukin(const std::string& x_var_name, const Double_t& min_x, const Double_t& max_x);
  std::function<Double_t(Double_t* x, Double_t* p)> evaluate_normalized() const;
  std::function<Double_t(Double_t* x, Double_t* p)> evaluate_floating() const;
  std::string get_name() const { return "bukin"; };
  UInt_t get_par_number() const { return 6; }
  UInt_t get_par_number_normalized() const { return 5; }
private:
};


Bukin::Bukin(const std::string& x_var_name, const Double_t& min_x, const Double_t& max_x) :
  ModelFunction(x_var_name, min_x, max_x) {
  parameters_.push_back(RooRealVar("Xp", "Xp", 10, 500));
  parameters_.push_back(RooRealVar("sp", "sp", 1, 100));
  parameters_.push_back(RooRealVar("rho1", "rho1", 0, 10));
  parameters_.push_back(RooRealVar("rho2", "rho2", 0, 10));
  parameters_.push_back(RooRealVar("xi", "xi", -0.99, 0.99));
  pdf_ = new bukin("bukin", "bukin",
                  x_, parameters_[0], parameters_[1], parameters_[2],
                  parameters_[3], parameters_[4]);
}

std::function<Double_t(Double_t* x, Double_t* p)> Bukin::evaluate_normalized() const {
  // fix this shit
  return [&](double* x, double* p) {
    return bkg_model_fit::bukin(x[0], p[0], p[1], p[2],
                                p[3], p[4], p[5]);
  };
}




std::function<Double_t(Double_t* x, Double_t* p)> Bukin::evaluate_floating() const {
  return [&](double* x, double* p) {
    return bkg_model_fit::bukin(x[0], p[0], p[1], p[2],
                                p[3], p[4], p[5]);
  };
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


// this can be improved a lot but for now it's like this.
void fix_limits(std::string model_name, TF1& model) {
  std::vector<UInt_t> indexes;
  Int_t fix_index = 0;
  if (model_name == "super_novosibirsk") {
    indexes = {0, 1, 3, 4, 5, 6};
    fix_index = 2;
  } else if (model_name == "bukin") {
    indexes = {1, 2, 3, 4, 5};
    fix_index = 0;
  } else {
    std::cerr << "Not implemented, be careful" << std::endl;
  }
  for (auto it: indexes) {
    Double_t param = model.GetParameter(it);
    if (param > 0) {
      model.SetParLimits(it, param*0.6, param*1.4);
    } else {
      model.SetParLimits(it, param*1.4, param*0.6);
    }
  }
  if (fix_index != -1) {
    Double_t param = model.GetParameter(fix_index);
    model.SetParLimits(fix_index, param, param);
  }
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
    ("unbinned", " EXPERIMENTAL. Perform an unbinned fit instead of a binned one. Actually it's still not working.")
    ;
  bp::variables_map vm;
  try {
    bp::store(bp::parse_command_line(argc, argv, cmdline_options), vm);
  } catch (const bp::error& e) {
    cerr << e.what() << endl;
    return -1;
  }

  bool print = false, logy = false, mc = true, print_initial = false, full_hadronic = false;
  bool use_integral = false, print_table = false, unbinned = false;
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
  if (vm.count("unbinned")) {
    unbinned = true;
    cerr << "You are using unbinned option but this it's still experimental and not " <<
      " working properly. Use it at your own risk and danger." << endl;
  }

  string model_name(vm["model"].as<string>());
  Float_t minx = vm["min-x"].as<Float_t>();
  Float_t maxx = vm["max-x"].as<Float_t>();
  UInt_t bins = vm["bins"].as<UInt_t>();
  string filename_pars = vm["initial-pars"].as<string>();
  vector<string> input_files = vm["input"].as<vector<string>>();
  string output_file = vm["output"].as<string>();
  vector<string> print_file;
  if (print) {
    print_file = vm["print"].as<vector<string>>();
  }
  
  
  ModelFunction* function_class = nullptr;
  if (model_name == "super_novosibirsk") {
    function_class = new SuperNovosibirsk("Mass", minx, maxx);
  } else if (model_name == "bukin") {
    function_class = new Bukin("Mass", minx, maxx);
  } else {
    cerr << "No known model with this name. Aborting." << endl;
    return -2;
  }
  std::function<Double_t(double* x, double*p)> model;
  if (unbinned) {
    model = function_class->evaluate_normalized();
  } else {
    model = function_class->evaluate_floating();
  }
  auto pdf = function_class->get_pdf();
  UInt_t npars = function_class->get_par_number();
  TChain chain("output_tree");  
  string filter_string;
  if (full_hadronic) {
    filter_string = "!Leptonic_event";
  } else {
    filter_string = "Leptonic_event";
  }
  vector<Float_t> init_pars;
  {
    std::ifstream infile(filename_pars.c_str());
    Float_t appo;
    while (infile >> appo) {
      init_pars.push_back(appo);
    }
    infile.close();
  }
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

  // Style stuff
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

  // Retrieve data from tree
  for (auto it: input_files) {
    chain.Add(it.c_str());
  }
  TF1 model_tf1("model_tf1", model, minx, maxx, npars);
  TH1F data_histo("data_histo", "data_histo", bins, minx, maxx);
  style.InitHist(&data_histo, "M_{12} [GeV]", (string("Events / ") +
                                               binnumber + string(" GeV")).c_str());
  string limit_string = "(Mass > " + std::to_string(minx) + ") && (Mass < " + \
    std::to_string(maxx) + ")";
  string selection = "Weigth*(" + filter_string + string(" && ") + limit_string + ")";
  chain.Draw("Mass>>data_histo", selection.c_str(), "E");

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
  TFitResultPtr fit_result;
  if (unbinned) {
    fit_result = data_histo.Fit(&model_tf1, fit_options.c_str(), "", minx, maxx);
    // Why is not possible passing a TF1 instead of a stupid string?
    // This solution is really bad but it's what these APIS permit.
    fix_limits(model_name, model_tf1);
    // fit_result = chain.UnbinnedFit("model_tf1",
    //                                "Mass", selection.c_str(), fit_options.c_str());
    chain.UnbinnedFit("model_tf1", "Mass", selection.c_str(), fit_options.c_str());
  } else {
    fit_result = data_histo.Fit(&model_tf1, fit_options.c_str(), "", minx, maxx);
  }
  // bool success = fit_result->IsValid();
  {
    TFitResult* appo = fit_result.Get();
    function_class->pick_parameters(appo);
  }

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
    exp_value = model_tf1.Integral(x - binning/2, x + binning/2)/binning;
    Double_t divisor = TMath::Sqrt(exp_value);
    if (divisor == 0) {
      divisor = 1;
    }
    Double_t y = (data_histo.GetBinContent(i) - exp_value)/divisor;
    
    dx[i - 1] = x;
    dy[i - 1] = y;
    // Double_t R = 0.03; // sigmaF/F
    // Double_t df = data_histo.GetBinCenter(i)/exp_value;
    // df = df > 2 ? 2 : df;
    // ddy[i - 1] = TMath::Sqrt(df + exp_value/4*TMath::Power(1 + df, 2)*R*R -
    //                          (1 + df)*R*TMath::Sqrt(data_histo.GetBinCenter(i)));
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

  return 0;
}
