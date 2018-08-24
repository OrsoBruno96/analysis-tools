/**
 * @author Fabio Zoratti <fabio.zoratti96@gmail.com> <fabio.zoratti@desy.de> <fabio.zoratti@cern.ch>
 * @date 24/8/2018
 * @file AddNormalizationToFile.cc
 */

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

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


int main(int argc, char* argv[]) {

  namespace bp = boost::program_options;
  using std::string; using std::cout; using std::endl; using std::cerr; using std::vector;
  bp::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help", "Produce help message")
    ("input-file", bp::value<string>()->required(), "Input ROOT file")
    ("output-file", bp::value<string>()->required(), "Output ROOT file")
    ("name", bp::value<string>()->required(), "Name of parameter to add")
    ("value", bp::value<Float_t>()->required(), "Value of parameter to add")
    ;
  bp::variables_map vm;
  bp::store(bp::parse_command_line(argc, argv, cmdline_options), vm);
  
  string new_variable = vm["name"].as<string>();
  Float_t value = vm["value"].as<Float_t>();
  TFile in_file(vm["input-file"].as<string>().c_str(), "read");
  RooWorkspace* w = reinterpret_cast<RooWorkspace*>(in_file.Get("w"));
  RooRealVar var(new_variable.c_str(), new_variable.c_str(), value, value);
  var.setVal(value);
  w->import(var);
  TFile out_file(vm["output-file"].as<string>().c_str(), "recreate");
  w->Write();
  in_file.Close();
  out_file.Close();
  return 0;
}
