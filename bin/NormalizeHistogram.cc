/**
 * "Macro" to normalize histograms and plot them on the same canvas
 * to see differences in shape
 */

#include <iostream>
#include <vector>

#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"


int main(int argc, char* argv[]) {
  using std::cout; using std::endl; using std::cerr; using std::vector;
  if (argc < 4) {
    cerr << "Usage: NormalizeHistogram file.root output.root hist1" << \
      "hist2 ... histn" << endl;
    return -1;
  }
  TFile file(argv[1], "read");
  if (file.IsZombie()) {
    cerr << "Error opening file. Aborting." << endl;
    return -2;
  }
  TFile output(argv[2], "create");
  if (output.IsZombie()) {
    cerr << "Error creating file output " << argv[2] << endl;
    return -2;
  }
  vector<TH1F*> histograms;
  for (int i = 3; i < argc; i++) {
    histograms.push_back(reinterpret_cast<TH1F*>(file.Get(argv[i])));
    if (histograms[i - 3]->IsZombie()) {
      cerr << "Error opening " << argv[i] << "hist." << endl;
      return -3;
    }
  }
  TCanvas* canvas = new TCanvas();
  canvas->cd();
  output.cd();
  for (unsigned int i = 0; i < histograms.size(); i++) {
    Double_t value = histograms[i]->GetEntries();
    histograms[i]->Scale(1/value);
    histograms[i]->Write();
    histograms[i]->SetLineColor(i);
    histograms[i]->Draw("same");
  }
  return 0;
}

