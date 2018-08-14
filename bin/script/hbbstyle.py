# -*- coding:utf-8 -*-
from ROOT import TStyle, TPaveText, kBlack, kBlue, kFALSE



def create_style():
    HbbStyle = TStyle("Htt-Style","The Perfect Style for Plots ;-)")
    gStyle = HbbStyle

    # Canvas
    HbbStyle.SetCanvasColor     (0)
    HbbStyle.SetCanvasBorderSize(10)
    HbbStyle.SetCanvasBorderMode(0)
    HbbStyle.SetCanvasDefH      (700)
    HbbStyle.SetCanvasDefW      (700)
    HbbStyle.SetCanvasDefX      (100)
    HbbStyle.SetCanvasDefY      (100)

    # color palette for 2D temperature plots
    # HbbStyle.SetPalette(1,0)
    
    # Pads
    HbbStyle.SetPadColor       (0)
    HbbStyle.SetPadBorderSize  (10)
    HbbStyle.SetPadBorderMode  (0)
    HbbStyle.SetPadBottomMargin(0.1)
    HbbStyle.SetPadTopMargin   (0.05)
    HbbStyle.SetPadLeftMargin  (0.1)
    HbbStyle.SetPadRightMargin (0.05)
    HbbStyle.SetPadGridX       (0)
    HbbStyle.SetPadGridY       (0)
    HbbStyle.SetPadTickX       (1)
    HbbStyle.SetPadTickY       (1)
    
    # Frames
    HbbStyle.SetLineWidth(3)
    HbbStyle.SetFrameFillStyle ( 0)
    HbbStyle.SetFrameFillColor ( 0)
    HbbStyle.SetFrameLineColor ( 1)
    HbbStyle.SetFrameLineStyle ( 0)
    HbbStyle.SetFrameLineWidth ( 2)
    HbbStyle.SetFrameBorderSize(10)
    HbbStyle.SetFrameBorderMode( 0)
    
    # Histograms
    HbbStyle.SetHistFillColor(2)
    HbbStyle.SetHistFillStyle(0)
    HbbStyle.SetHistLineColor(1)
    HbbStyle.SetHistLineStyle(0)
    HbbStyle.SetHistLineWidth(3)
    HbbStyle.SetNdivisions(505)
    
    # Functions
    HbbStyle.SetFuncColor(1)
    HbbStyle.SetFuncStyle(0)
    HbbStyle.SetFuncWidth(2)

    # Various
    HbbStyle.SetMarkerStyle(20)
    HbbStyle.SetMarkerColor(kBlack)
    HbbStyle.SetMarkerSize (1.4)

    HbbStyle.SetTitleBorderSize(0)
    HbbStyle.SetTitleFillColor (0)
    HbbStyle.SetTitleX         (0.2)
    
    HbbStyle.SetTitleSize  (0.055,"X")
    HbbStyle.SetTitleOffset(1.200,"X")
    HbbStyle.SetLabelOffset(0.005,"X")
    HbbStyle.SetLabelSize  (0.050,"X")
    HbbStyle.SetLabelFont  (42   ,"X")

    HbbStyle.SetStripDecimals(kFALSE)
    HbbStyle.SetLineStyleString(11,"20 10")
    
    HbbStyle.SetTitleSize  (0.055,"Y")
    HbbStyle.SetTitleOffset(1.000,"Y")
    HbbStyle.SetLabelOffset(0.010,"Y")
    HbbStyle.SetLabelSize  (0.050,"Y")
    HbbStyle.SetLabelFont  (42   ,"Y")
    
    HbbStyle.SetTextSize   (0.055)
    HbbStyle.SetTextFont   (42)
    
    HbbStyle.SetStatFont   (42)
    HbbStyle.SetTitleFont  (42)
    HbbStyle.SetTitleFont  (42,"X")
    HbbStyle.SetTitleFont  (42,"Y")
    
    HbbStyle.SetOptStat    (0)



def init_hist(hist, xtit, ytit, color=kBlack, style=0):
    hist.SetXTitle(xtit);
    hist.SetYTitle(ytit);
    hist.SetLineColor(color);
    hist.SetLineWidth(2);
    hist.SetFillColor(color);
    hist.SetFillStyle(style);
    #  hist.GetYaxis().SetRangeUser(0.1,100000);
    hist.SetTitleSize  (0.055,"Y");
    hist.SetTitleOffset(1.200,"Y");
    hist.SetLabelOffset(0.014,"Y");
    hist.SetLabelSize  (0.040,"Y");
    hist.SetLabelFont  (42   ,"Y");
    hist.SetTitleSize  (0.055,"X");
    hist.SetTitleOffset(1.300,"X");
    hist.SetLabelOffset(0.014,"X");
    hist.SetLabelSize  (0.050,"X");
    hist.SetLabelFont  (42   ,"X");
    hist.SetMarkerStyle(20);
    hist.SetMarkerColor(color);
    hist.SetMarkerSize (0.6);
    hist.GetYaxis().SetTitleFont(42);
    hist.GetXaxis().SetTitleFont(42);
    hist.SetTitle("");


def CMS_prelim(MC, dataset, lowX, lowY):
    cmsprel = TPaveText(lowX, lowY+0.06, lowX+0.30, lowY+0.16, "NDC")
    cmsprel.SetBorderSize(   0 )
    cmsprel.SetFillStyle(    0 )
    cmsprel.SetTextAlign(   12 )
    cmsprel.SetTextSize ( 0.05 )
    cmsprel.SetTextColor(    1 )
    cmsprel.SetTextFont (   62 )
    if not MC:
        cmsprel.AddText("CMS           ")
    else:
        cmsprel.AddText("CMS Simulation")
    cmsprel.Draw()

    lowXlumi  = lowX+0.59
    lowYlumi  = lowY+0.158
    highXlumi = lowX+0.72
    highYlumi = lowY+0.161

    if not MC:
        lowXlumi  = lowX+0.48
        lowYlumi  = lowY+0.155
        highXlumi = lowX+0.75
        highYlumi = lowY+0.232
    lumi = TPaveText(lowXlumi, lowYlumi, highXlumi, highYlumi, "NDC") 
    lumi.SetBorderSize(   0 )
    lumi.SetFillStyle(    0 )
    lumi.SetTextAlign(   12 )
    lumi.SetTextSize ( 0.05 )
    lumi.SetTextColor(    1 )
    lumi.SetTextFont (   62 )
    if not MC:
        lumi.AddText(dataset)
    else:
        lumi.AddText("13 TeV")
    lumi.Draw()
    
    wip = TPaveText(lowX-0.005, lowY+0.05, lowX+0.4, lowY+0.06, "NDC")
    wip.SetBorderSize(   0 )
    wip.SetFillStyle(    0 )
    wip.SetTextAlign(   12 )
    wip.SetTextSize ( 0.04 )
    wip.SetTextColor(    1 )
    wip.SetTextFont (   52 )
    wip.AddText("Work in Progress")
    wip.Draw()


def init_signal(hist):
    hist.SetFillStyle(0.);
    hist.SetLineStyle(11);
    hist.SetLineWidth(2.);
    hist.SetLineColor(kBlue+3);
