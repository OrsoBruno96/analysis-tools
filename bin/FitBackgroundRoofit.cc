/**
 * @author Fabio Zoratti <fabio.zoratti96@gmail.com> <fabio.zoratti@desy.de> <fabio.zoratti@cern.ch>
 * @date 21/8/2018
 * @file FitBackground.cc
 */


#include <string>
#include <iostream>
#include <vector>


#include <boost/program_options.hpp>

#include "TChain.h"
#include "TH1F.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooArgList.h"
#include "RooWorkspace.h"
#include "RooPlot.h"

// #include "tdrstyle.C"
#include "HbbStylesNew.cc"
#include "Analysis/Tools/interface/super_novosibirsk.h"



int main(int argc, char* argv[]) {
  namespace bp = boost::program_options;
  using std::string; using std::cout; using std::endl; using std::cerr; using std::vector;
  bp::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help,h", "Produce help message")
    ("output,o", bp::value<string>(), "Output ROOT file")
    ("input,i", bp::value<vector<string>>()->multitoken()->composing(), "Input ROOT files")
    ;

  bp::variables_map vm;
  bp::store(bp::parse_command_line(argc, argv, cmdline_options), vm);
  if (vm.count("help")) {
    cout << cmdline_options << endl;
  }
  if (vm.count("output") == 0) {
    cerr << "Please specify output file." << endl;
    return -1;
  }
  if (vm.count("input") == 0) {
    cerr << "Please specify input file." << endl;
    return -2;
  }
      
      
  
  TChain chain("output_tree");
  string filter_string("Leptonic_event");
  TTree* tree = chain.CopyTree(filter_string.c_str());

  
  RooRealVar Mass("Mass", "Mass", 120, 600);
  RooRealVar p0("p0", "p0", 0, 0.007);
  RooRealVar p1("p1", "p1", 1400, 2200);
  RooRealVar p3("p3", "p3", 10, 100);
  RooRealVar p4("p4", "p4", 10, 100);
  RooRealVar p5("p5", "p5", 0, 10);
  RooRealVar p6("p6", "p6", -0.3, -0.0001);

  auto frame = Mass.frame();
  cout << "Retrieving data from tree" << endl;
  
  TH1F histo("histo", "histo", 300, 120, 600);
  tree->Draw("Mass>>histo", "");
  RooDataHist data("Roohisto", "Roohisto", RooArgList(Mass), RooFit::Import(histo));
  data.plotOn(frame);
  cout << "Plotting and fitting stuff" << endl;

  analysis::tools::super_novosibirsk pdf("super_novosibirsk", "super_novosibirsk", Mass, p0, p1, p3, p4, p5, p6);
  auto fitResult = pdf.fitTo(data);
  pdf.plotOn(frame);

  RooWorkspace w("w");
  w.import(pdf);
  frame->Draw();
  
  
  return 0;
}
