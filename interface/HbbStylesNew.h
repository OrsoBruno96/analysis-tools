#ifndef MSSMHBB_MACROS_DRAWER_HBBSTYLESNEW_C_
#define MSSMHBB_MACROS_DRAWER_HBBSTYLESNEW_C_

#include "TPad.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TPaveText.h"
#include "TLatex.h"
#include "TGraphErrors.h"

class HbbStylesNew {
public:

  HbbStylesNew();
  ~HbbStylesNew();
  
  /// define common styles
  void SetStyle();
  /// create a canveas in common style (default is square)
  TCanvas* MakeCanvas(const char* name, const char* title, int dX=600, int dY=600);
  /// define common style of pads
  void InitSubPad(TPad* pad, int i);
  /// define a histogram file (not yet including poly markers, which are used for data)
  void InitHist(TH1* hist, const char* xtit, const char* ytit="Number of Entries", int color=kBlack, int style=0);
  /// define poly markers and line width for data
  void InitData(TH1* hist);
  /// define line style, width and color for signal sample
  void InitSignal(TH1* hist);

  void InitGraph(TGraphErrors* graph, const char* xtit, const char* ytit="Number of Entries", int color=kBlack, int style=0);
  
  /// define common legend style
  void SetLegendStyle(TLegend* leg);
  /// add CMSPreliminary logo, lumi and channel
  void CMSPrelim(bool MC, const char* dataset, double lowX=0.4, double lowY=0.74);
  void plotchannel(TString channel);

private:
  static double textSize_;
  TLatex* tex;
  TPaveText* wip;
  TPaveText* lumi;
  TPaveText* cmsprel;
};



#endif /* MSSMHBB_MACROS_DRAWER_HBBSTYLESNEW_C_ */
