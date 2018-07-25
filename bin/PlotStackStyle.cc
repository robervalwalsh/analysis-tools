/**
 *
 */

#include <string>
#include <iostream>
#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#pragma GCC diagnostic pop

#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"
#include "TColor.h"
#include "TROOT.h"

#include "tdrstyle.C"
#include "HbbStylesNew.cc"

const Float_t lumi = 2.;


class TH1InStack {
public:
  TH1InStack(const std::string filename, const std::string inputname,   \
             const std::string legendname, TColor* color) :                           \
    color_(color), legend_name_(legendname) , input_name_(inputname) ,  \
    input_(filename.c_str(), "read") {
    using std::endl; using std::cerr;
    if (input_.IsZombie()) {
      // Raise exception
      cerr << "Error opening file." << endl;
    }
    histo_ = reinterpret_cast<TH1*>(input_.Get(input_name_.c_str()));
    if (histo_->IsZombie()) {
      cerr << "Bad histogram" << endl;
    }
    histo_->SetFillColor(color_->GetNumber());
  }
  ~TH1InStack() { delete histo_; }
  
  TH1* Hist() const { return histo_; }
  
private:
  TColor* color_;
  std::string legend_name_;
  std::string input_name_;
  TH1* histo_;
  TFile input_;
};


/**
 * Reads json file and returns pointer to THStack
 */

class THStackBetter {
 public:
  explicit THStackBetter(const char* name);
  void Draw();
  
 private:
  std::vector<TH1InStack*> histos_;
  std::string name_;
  TLegend leg;
  THStack stack_hist_;
  std::string x_axis_;
  std::string y_axis_;
  std::string title_;
};


THStackBetter::THStackBetter(const char* filename) : \
  leg(0.58, 0.63, 0.98, 0.93), \
  stack_hist_("stack_hist_", "") {
  using std::string; using boost::property_tree::ptree;
  using boost::property_tree::read_json; using std::cerr; using std::endl;
  ptree root;
  read_json(filename, root);
  for (ptree::value_type& it : root.get_child("histograms")) {
    auto json = it.second;
    string input_filename(json.get<string>("file_name"));
    std::vector<int> rgb;
    for (auto & colore : json.get_child("color")) {
      rgb.push_back(colore.second.get_value<int>());
    }
    TColor* color = new TColor(TColor::GetFreeColorIndex(), rgb[0], rgb[1], rgb[2]) ;
    TH1InStack* appo = new TH1InStack(json.get<string>("file_name"), \
                                      json.get<string>("name"), \
                                      json.get<string>("legend"), color);
    histos_.push_back(appo);
    leg.AddEntry(appo->Hist(), json.get<string>("legend").c_str(), "pf");
    stack_hist_.Add(appo->Hist());
  }
  x_axis_ = root.get<string>("x_axis");
  y_axis_ = root.get<string>("y_axis");
  title_ = root.get<string>("title");    
}


void THStackBetter::Draw() {
  stack_hist_.Draw("hist");
  stack_hist_.GetXaxis()->SetTitle(x_axis_.c_str());
  stack_hist_.GetXaxis()->SetTitle(y_axis_.c_str());
  stack_hist_.SetTitle(title_.c_str());
  leg.Draw("same");
}


int main(int argc, char* argv[]) {
  using std::cerr; using std::endl; using std::string;  
  HbbStylesNew style;
  style.SetStyle();
  gStyle->SetOptStat(0);
  setTDRStyle();
  TCanvas* c1 = style.MakeCanvas("c1", "", 700, 700);

  if (argc != 3) {
    cerr << "./PlotStackStyle input.json output.pdf" << endl;
    return -1;
  }
  THStackBetter stack(argv[1]);
  stack.Draw();
  CMSPrelim(true, "MC", 0.15, 0.79);
  c1->Update();
  c1->SaveAs(argv[2]);
  return 0;
}
