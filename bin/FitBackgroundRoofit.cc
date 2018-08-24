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
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooArgList.h"
#include "RooWorkspace.h"
#include "RooPlot.h"

#include "tdrstyle.C"
#include "Analysis/Tools/interface/HbbStylesNew.h"

#include "Analysis/Tools/src/classes.h"



int main(int argc, char* argv[]) {
  namespace bp = boost::program_options;
  using std::string; using std::cout; using std::endl; using std::cerr; using std::vector;
  bp::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help,h", "Produce help message")
    ("output,o", bp::value<string>()->required(), "Output ROOT file")
    ("input,i", bp::value<vector<string>>()->required()->multitoken()->composing(), "Input ROOT files")
    ("print", bp::value<vector<string>>()->multitoken()->composing(), "Optional: files where to print the canvas with fitted function")
    ("min-x", bp::value<Float_t>()->default_value(0), "Min x to choose for the fit")
    ("max-x", bp::value<Float_t>()->default_value(1500), "Max x to choose for the fit")
    ("bins", bp::value<UInt_t>()->required(), "Number of bins for fit")
    ("initial-pars", bp::value<string>()->required(), "Initial parameters for the fit. Give a filename")
    ("model", bp::value<string>()->required(), "Model to choose for the fit")
    ;
  
  bp::variables_map vm;
  bp::store(bp::parse_command_line(argc, argv, cmdline_options), vm);

  bool print = false;
  vector<string> print_filename;
  Float_t minx = vm["min-x"].as<Float_t>();
  Float_t maxx = vm["max-x"].as<Float_t>();
  UInt_t bins = vm["bins"].as<UInt_t>();

  
  if (vm.count("help")) {
    cout << cmdline_options << endl;
  }
  if (vm.count("print")) {
    print = true;
  }
  if (print) {
    print_filename = vm["print"].as<vector<string>>();
  }
  string model_name(vm["model"].as<string>());  
  vector<string> input_files = vm["input"].as<vector<string>>();
  string output_file = vm["output"].as<string>();
  UInt_t npars = 0;
  string filename_pars = vm["initial-pars"].as<string>();
  TChain chain("output_tree");
  string filter_string("Leptonic_event");
  TCanvas c1;
  for (auto it: input_files) {
    chain.Add(it.c_str());
  }
  TTree* tree = chain.CopyTree(filter_string.c_str());
  
  RooRealVar Mass("Mass", "Mass", minx, maxx);

  vector<Float_t> init_pars;
  {
    std::ifstream infile(filename_pars.c_str());
    Float_t appo;
    while (infile >> appo) {
      init_pars.push_back(appo);
    }
    infile.close();
  }
  
  auto frame = Mass.frame();
  
  TH1F histo("histo", "histo", bins, minx, maxx);
  tree->Draw("Mass>>histo", "");
  RooDataHist data("Roohisto", "Roohisto", RooArgList(Mass), RooFit::Import(histo));
  data.plotOn(frame);

  RooAbsPdf* pdf = nullptr;
  vector<RooRealVar*> variables_to_delete;
  
  if (model_name == "super_novosibirsk") {
    RooRealVar* p0 = new RooRealVar("p0", "p0", 0, 0.007);
    RooRealVar* p1 = new RooRealVar("p1", "p1", 1400, 2200);
    RooRealVar* p3 = new RooRealVar("p3", "p3", 10, 100);
    RooRealVar* p4 = new RooRealVar("p4", "p4", 10, 100);
    RooRealVar* p5 = new RooRealVar("p5", "p5", 0, 10);
    RooRealVar* p6 = new RooRealVar("p6", "p6", -0.3, -0.0001);
    pdf = new super_novosibirsk("super_novosibirsk", "super_novosibirsk",
                                Mass, *p0, *p1, *p3, *p4, *p5, *p6);
    vector<RooRealVar*> variables( {p0, p1, p3, p4, p5, p6} );
    for (unsigned int i = 0; i < init_pars.size(); i++) {
      variables[i]->setVal(init_pars[i]);
      variables_to_delete.push_back(variables[i]);
    }
    npars = 6;
  } else if (model_name == "bukin") {
    RooRealVar* p1 = new RooRealVar("Xp", "Xp", 10, 500);
    RooRealVar* p3 = new RooRealVar("sp", "sp", 1, 100);
    RooRealVar* p4 = new RooRealVar("rho1", "rho1", 0, 10);
    RooRealVar* p5 = new RooRealVar("rho2", "rho2", 0, 10);
    RooRealVar* p6 = new RooRealVar("xi", "xi", -0.99, 0.99);
    pdf = new bukin("bukin", "bukin",
                                Mass, *p1, *p3, *p4, *p5, *p6);
    vector<RooRealVar*> variables( {p1, p3, p4, p5, p6} );
    for (unsigned int i = 0; i < init_pars.size(); i++) {
      variables[i]->setVal(init_pars[i]);
    }
    variables_to_delete.push_back(p1);
    variables_to_delete.push_back(p3);
    variables_to_delete.push_back(p4);
    variables_to_delete.push_back(p5);
    variables_to_delete.push_back(p6);
    npars = 5;
  } else {
    cerr << "No matching model found. Aborting" << endl;
    return -8;
  }


  if (init_pars.size() != npars) {
    cerr << "The initial parameters given have not the right size." << endl;
    cerr << "I want this size: " << npars << " and i got " << init_pars.size() << endl;
    return -6;
  }

  
  auto fitResult = pdf->fitTo(data);
  pdf->plotOn(frame);

  RooWorkspace w("w");
  w.import(*pdf);
  w.writeToFile(output_file.c_str(), true);
  c1.cd();
  frame->Draw();
  if (print) {
    for (auto it : print_filename) {
      c1.Print(it.c_str());
    }
  }

  for (auto it: variables_to_delete) {
    delete it;
  }
  return 0;
}
