/**
 *
 */

#include <string>
#include <iostream>

#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "tdrstyle.C"
#include "HbbStylesNew.cc"

Float_t lumi = -1.;

int main(int argc, char* argv[]) {
  using std::cerr; using std::endl; using std::string;
  if (argc != 4) {
    cerr << "Program run with wrong number of parameters."<< \
      " Usage: PlotWithStyle file.root histname histtitle" << endl;
    return -1;
  }
  TFile file(argv[1], "read");
  if (file.IsZombie()) {
    cerr << "Error opening file. Aborting" << endl;
    return -2;
  }

  TH1F* histo = reinterpret_cast<TH1F*>(file.Get(argv[2]));
  if (histo->IsZombie()) {
    cerr << "Error in getting histogram. Aborting." << endl;
    return -3;
  }
  HbbStylesNew style;
  style.SetStyle();
  gStyle->SetOptStat(0);
  setTDRStyle();

  TCanvas* c1 = style.MakeCanvas("c1", "", 700, 700);
  style.InitHist(histo, argv[3], "Entries / 20 GeV", kBlack, 0);

  histo->Draw();
  TLegend* leg = new TLegend(0.58, 0.63, 0.98, 0.93);
  style.SetLegendStyle(leg);
  leg->AddEntry(histo, "Titolo, Prompt", "L");
  leg->Draw("same");

  CMSPrelim(Form("%.1f fb^{-1} (13 TeV)", lumi), "", 0.15, 0.78);
  c1->Update();
  c1->SaveAs(("output/plots/" + string(argv[2]) + ".pdf").c_str());
  return 0;
}
