/**
 *
 */

#include <string>
#include <iostream>

#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"

#include "tdrstyle.C"
#include "HbbStylesNew.cc"

Float_t lumi = 2.;

int main(int argc, char* argv[]) {
  using std::cerr; using std::endl; using std::string;
  TFile file("output/hists/chromo_lepton.root", "read");
  if (file.IsZombie()) {
    cerr << "Error opening file. Aborting" << endl;
    return -2;
  }

  TH1F* lepto = reinterpret_cast<TH1F*>(file.Get("mass_histo_lepton"));
  TH1F* chromo = reinterpret_cast<TH1F*>(file.Get("mass_histo_chromo;1"));
  if (lepto->IsZombie() || chromo->IsZombie()) {
    cerr << "Error in getting histogram. Aborting." << endl;
    return -3;
  }
  HbbStylesNew style;
  style.SetStyle();
  gStyle->SetOptStat(0);
  setTDRStyle();
  THStack stack("stack", "");
  TCanvas* c1 = style.MakeCanvas("c1", "", 700, 700);

  lepto->SetFillColor(kRed);
  chromo->SetFillColor(kBlue);
  stack.Add(lepto);
  stack.Add(chromo);
  stack.Draw("hist");
  TLegend leg(0.58, 0.63, 0.98, 0.93);
  style.SetLegendStyle(&leg);
  stack.GetXaxis()->SetTitle("m_{12} [GeV]");
  stack.GetYaxis()->SetTitle("Events / 16 GeV");
  // TLegendEntry* l1 = leg->AddEntry(lepto, "Leptonic event", "lp");
  leg.AddEntry(lepto, "Leptonic", "pf");
  leg.AddEntry(chromo, "Non-leptonic", "pf");
  // CMSPrelim(Form("%.1f fb^{-1} (13 TeV)", lumi), "", 0.15, 0.78);
  CMSPrelim(true, "MC", 0.15, 0.79);
  leg.Draw("same");
  c1->Update();
  c1->SaveAs("output/plots/stack_lepto_chromo.pdf");
  return 0;
}
