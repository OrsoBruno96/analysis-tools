/**
 * @author Fabio Zoratti <fabio.zoratti96@gmail.com> <fabio.zoratti@desy.de> <fabio.zoratti@cern.ch>
 * @date 21/8/2018
 * @file FitBackground.cc
 */



#include <string>
#include <iostream>
#include <vector>
#include <fstream>

#include <boost/program_options.hpp>

#include "TChain.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TFile.h"

#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooArgList.h"
#include "RooWorkspace.h"
#include "RooPlot.h"

namespace bpo = boost::program_options;


int main(int argc, char* argv[]) {
  using std::string; using std::cout; using std::cerr; using std::endl;
  
  bpo::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help,h", "Produce help message")
    ("input-file", bpo::value<string>()->required(), "Input file")
    ("input-histo", bpo::value<string>()->required(), "Input name of histo")
    ("output-file", bpo::value<string>()->required(), "Output file")
    ("output-histo", bpo::value<string>()->required(), "Output name inside workspace")
    ;
  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, cmdline_options), vm);


  TFile input_file(vm["input-file"].as<string>().c_str(), "read");
  TH1F* histo = reinterpret_cast<TH1F*>(input_file.Get(vm["input-histo"].as<string>().c_str()));
  RooRealVar Mass("Mass", "Mass", 0, 1600);
  RooDataHist roohist(vm["output-histo"].as<string>().c_str(), vm["output-histo"].as<string>().c_str(), Mass, histo);
  RooWorkspace w("w");
  w.import(roohist);
  w.writeToFile(vm["output-file"].as<string>().c_str(), true);
  
  return 0;
}
