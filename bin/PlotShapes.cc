/**
 * @file PlotShapes.cc
 * @date Created on: Sep 1, 2018
 * @author: Fabio Zoratti <fabio.zoratti96@gmail.com>
 */


#include <string>
#include <iostream>
#include <fstream>
#include <exception>

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
#include "TTree.h"

#include "tdrstyle.C"
#include "Analysis/Tools/interface/HbbStylesNew.h"


int main(int argc, char* argv[]) {
  using std::cerr; using std::endl; using std::string; using std::vector; using std::cout;
  using boost::property_tree::read_json; using boost::property_tree::ptree; using std::vector;
  namespace bp = boost::program_options;
  bp::options_description cmdline_options("Command line arguments");
  cmdline_options.add_options()
    ("help,h", "Produce help message")
    ("output,o", bp::value<vector<string>>()->multitoken()->required()->composing(), "Output plot files")
    ("input,i", bp::value<string>(), "Input json file")
    ;
  bp::variables_map vm;
  try {
    bp::store(bp::parse_command_line(argc, argv, cmdline_options), vm);
  } catch (bp::error& e) {
    cerr << e.what() << endl;
    return -1;
  }
  if (vm.count("help")) {
    cout << cmdline_options << endl;
    return 0;
  }
  ptree root;
  read_json(vm["input"].as<string>().c_str(), root);
  vector<TFile*> to_delete_later_file;
  vector<TTree*> to_delete_later_tree;
  vector<TH1F*> to_delete_later_histos;

  HbbStylesNew style;
  style.SetStyle();
  gStyle->SetOptStat(0);
  setTDRStyle();

  TCanvas* c1 = style.MakeCanvas("c1", "", 700, 700);
  TLegend leg(0.58, 0.63, 0.98, 0.93);

  Float_t maximum = 0;
  for (ptree::value_type& it : root.get_child("histos")) {
    string mass(it.first);
    vector<int> rgb;
    for (auto & colore : it.second.get_child("color")) {
      rgb.push_back(colore.second.get_value<int>());
    }
    int bins = it.second.get<int>("bins");
    TColor* color = new TColor(TColor::GetFreeColorIndex(),
                               (Float_t) rgb[0]/255, (Float_t) rgb[1]/255,
                               (Float_t) rgb[2]/255) ;
    TFile* file_before = new TFile(it.second.get_child("before").get<string>("root_file").c_str(),
                                   "read");
    TTree* tree_before = reinterpret_cast<TTree*>(file_before->Get("output_tree"));
    TFile* file_after = new TFile(it.second.get_child("after").get<string>("root_file").c_str(),
                                  "read");
    TTree* tree_after = reinterpret_cast<TTree*>(file_after->Get("output_tree"));
    TH1F* histo_before = new TH1F((mass + "_before").c_str(), "", bins,
                                  root.get<int>("minx"), root.get<int>("maxx"));
    TH1F* histo_after =  new TH1F((mass + "_after").c_str(), "", bins,
                                  root.get<int>("minx"), root.get<int>("maxx"));
    style.InitHist(histo_before, root.get<string>("x_axis").c_str(),
                   root.get<string>("y_axis").c_str(), color->GetNumber(), 0);
    style.InitHist(histo_after, root.get<string>("x_axis").c_str(),
                   root.get<string>("y_axis").c_str(), color->GetNumber(), 0);
    histo_before->SetMarkerStyle(21);
    histo_after->SetMarkerStyle(22);
    histo_before->SetLineStyle(1);
    histo_after->SetLineStyle(7);
    histo_before->SetLineColor(color->GetNumber());
    gStyle->SetLineStyle(1);
    tree_before->Draw(("Mass>>" + mass + "_before").c_str(),
                      it.second.get_child("before").get<string>("selection").c_str(),
                      "hist same norm");
    histo_after->SetLineColor(color->GetNumber());
    gStyle->SetLineStyle(4);
    tree_after->Draw(("Mass>>" + mass + "_after").c_str(),
                      it.second.get_child("after").get<string>("selection").c_str(),
                      "hist same norm");
    histo_after->Scale(bins);
    histo_before->Scale(bins);
    auto appo = histo_before->GetMaximum()*1.5;
    maximum = appo > maximum ? appo : maximum;
    appo = histo_after->GetMaximum()*1.5;
    maximum = appo > maximum ? appo : maximum;
    histo_after->SetMaximum(maximum);
    histo_before->SetMaximum(maximum);
    to_delete_later_file.push_back(file_before);
    to_delete_later_file.push_back(file_after);
    to_delete_later_tree.push_back(tree_before);
    to_delete_later_tree.push_back(tree_after);
    to_delete_later_histos.push_back(histo_before);
    to_delete_later_histos.push_back(histo_after);
  }

  leg.AddEntry(to_delete_later_histos[0],
               root.get<string>("legend_before").c_str(), "lp");
  leg.AddEntry(to_delete_later_histos[1],
               root.get<string>("legend_after").c_str(), "lp");
  style.SetLegendStyle(&leg);
  leg.Draw("same");

  style.CMSPrelim(true, "MC", 0.14, 0.78);
  c1->Update();
  for (auto it: vm["output"].as<vector<string>>()) {
    c1->Print(it.c_str());
  }

  delete c1;
  c1 = nullptr;
  return 0;
}
