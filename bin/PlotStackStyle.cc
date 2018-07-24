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

Float_t lumi = -1.;

int main(int argc, char* argv[]) {
  using std::cerr; using std::endl; using std::string;
  TFile file("output/chromo_lepton.root", "read");
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
  style.InitHist(lepto, "Leptons", "Entries / 20 GeV", kRed, 0);
  style.InitHist(chromo, "Chromo", "Entries / 20 GeV", kBlue, 0);
  // style.InitHist(reinterpret_cast<TH1*>(&stack), "Sum", "Entries / 20 GeV", kBlack, 0);

  stack.Add(lepto);
  stack.Add(chromo);
  stack.Draw();
  TLegend* leg = new TLegend(0.58, 0.63, 0.98, 0.93);
  style.SetLegendStyle(leg);
  leg->AddEntry(&stack, "Titolo, Prompt", "L");
  leg->Draw("same");

  CMSPrelim(Form("%.1f fb^{-1} (13 TeV)", lumi), "", 0.15, 0.78);
  c1->Update();
  c1->SaveAs("output/plots/stack_lepto_chromo.pdf");
  return 0;
}
