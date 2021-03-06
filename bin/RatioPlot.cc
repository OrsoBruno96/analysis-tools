/**
 * @file RatioPlot.cc
 * @date Created on: Aug 16, 2018
 * @author: Fabio Zoratti <fabio.zoratti96@gmail.com>
 */


/*
 * DOCS: Here I copy an example input file that you can use as input.json



{
    "histograms": [
        {
            "file_name": "/afs/desy.de/user/z/zorattif/dust/output/raw_files/MC/no_reranking/medium_wp/350_nothing_true.root",
            "name": "mass_histo_chromo_0_match;2",
            "legend": "No correction",
            "color": [255, 0, 0]
        },
        {
            "file_name": "/afs/desy.de/user/z/zorattif/dust/output/raw_files/MC/no_reranking/medium_wp/350_only_smearing_true.root",
            "name": "mass_histo_chromo_0_match;2",
            "legend": "Smearing",
            "color": [0, 0, 0]
        }
    ],
    "title": "Invariant mass of bb jets",
    "x_axis": "m_{12} [GeV]",
    "y_axis": "Events / 20 GeV"
}


 */




#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <sstream>
#include <iomanip>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#pragma GCC diagnostic pop
#include <boost/program_options.hpp>



#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"
#include "TColor.h"
#include "TROOT.h"
#include "TRatioPlot.h"
#include "TGaxis.h"

#include "tdrstyle.C"
#include "Analysis/Tools/interface/HbbStylesNew.h"



namespace bpo = boost::program_options;

int main(int argc, char* argv[]) {
  using std::string; using boost::property_tree::ptree;
  using boost::property_tree::read_json; using std::cerr; using std::endl;
  using std::cout;

  bpo::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help,h", "Produce help message")
    ("output,o", bpo::value<string>()->default_value("out.pdf"), "Output file (can be every extension)")
    ("input,i", bpo::value<string>(), "Input file in json format")
    ("fix-ratio,F", "Fix the min-max ratio at 3")
    ("min-ratio,m", bpo::value<Float_t>(), "Fix the minimun ratio at this value")
    ("max-ratio,M", bpo::value<Float_t>(), "Fix the max ratio at this value")
    ("log-y,L", "Set log scale for y axis")
    ("lumi,l", bpo::value<Float_t>(), "Luminosity in fb-1 to be shown. If not provided, the dataset is assumed to be a MC")
    ;
  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, cmdline_options), vm);
  if (vm.count("help")) {
    cout << cmdline_options << endl;
    return 0;
  }
  if (vm.count("input") != 1) {
    cerr << "Please specify input file." << endl;
    return -1;
  }
  if (vm.count("output") != 1) {
    cerr << "Please specify output file" << endl;
    return -2;
  }
  bool logy = false;
  Float_t min = 0, max = 0;
  bool set_min = false;
  bool set_max = false;
  bool mc = true;
  Float_t lumi = 0;
  if (vm.count("lumi")) {
    mc = false;
    lumi = vm["lumi"].as<Float_t>();
  }

  if (vm.count("log-y")) {
    logy = true;
  }
  if (vm.count("min-ratio")) {
    min = vm["min-ratio"].as<Float_t>();
    set_min = true;
  }
  if (vm.count("max-ratio")) {
    max = vm["max-ratio"].as<Float_t>();
    set_max = true;
  }
  

  string config_file(vm["input"].as<string>());
  string output_file(vm["output"].as<string>());
  // Setting style
  HbbStylesNew style;
  style.SetStyle();
  gStyle->SetOptStat(0);
  setTDRStyle();
  TCanvas* c1 = style.MakeCanvas("c1", "", 800, 800);
  TLegend leg(0.58, 0.63, 0.98, 0.93);
  
  ptree root;
  read_json(config_file, root);

  TColor* colors[2];
  TH1F* histos[2];
  TFile* files[2];
  unsigned int i = 0;
  for (ptree::value_type& it : root.get_child("histograms")) {
    auto json = it.second;    
    std::vector<int> rgb;
    for (auto & colore : json.get_child("color")) {
      rgb.push_back(colore.second.get_value<int>());
    }
    colors[i] = new TColor(TColor::GetFreeColorIndex(), rgb[0], rgb[1], rgb[2]);
    files[i] = new TFile(json.get<string>("file_name").c_str(), "read");
    if (files[i]->IsZombie()) {
      throw std::runtime_error("Error opening file " + json.get<string>("file_name"));
    }

    histos[i] = reinterpret_cast<TH1F*>(files[i]->Get(json.get<string>("name").c_str()));
    if (histos[i] == nullptr || histos[i]->IsZombie()) {
      throw std::runtime_error(("Error opening histogram " + \
                                json.get<string>("name") + " in file " + \
                                json.get<string>("file_name")).c_str());
    }
    style.InitHist(histos[i], root.get<string>("x_axis").c_str(),
                   root.get<string>("y_axis").c_str(), colors[i]->GetNumber(), 0);
    histos[i]->SetTitle(root.get<string>("title").c_str());
    leg.AddEntry(histos[i], json.get<string>("legend").c_str(), "pf");
    i++;
  }
  TRatioPlot ratio(histos[0], histos[1]);
  ratio.GetXaxis()->SetTitle(root.get<string>("x_axis").c_str());
  ratio.GetUpYaxis()->SetTitle(root.get<string>("y_axis").c_str());
  TGaxis::SetMaxDigits(3);
  ratio.GetLowYaxis()->SetNdivisions(606);
  ratio.GetUpperPad()->SetLeftMargin(0.14);
  style.SetLegendStyle(&leg);
  ratio.Draw();
  
  string appo;
  Float_t bottom = 0, left = 0;
  if (mc) {
    appo = string("MC");
    bottom = 0.79;
    left = 0.15;
  } else {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << lumi;
    appo = ss.str() + " fb^{-1} (13 TeV)";
    bottom = 0.76;
    left = 0.14;
  }
  style.CMSPrelim(mc, appo.c_str(), left, bottom);
  if (set_min) {
    ratio.GetLowerRefGraph()->SetMinimum(min);
  }
  if (set_max) {
    ratio.GetLowerRefGraph()->SetMaximum(max);
  }
  if (logy) {
    c1->SetLogy();
  }
  leg.Draw("same");
  c1->Update();
  c1->Print(output_file.c_str());
  delete c1;
  delete histos[0];
  delete histos[1];
  delete files[0];
  delete files[1];
  delete colors[0];
  delete colors[1];
  return 0;
}
