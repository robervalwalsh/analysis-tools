#!/usr/bin/env python3

import ctypes
import ROOT
from ROOT import TCanvas, TMultiGraph,TLine, TRatioPlot, gStyle, gROOT, gPad, TH1, TLegend
from ROOT import kRed, kBlue, kBlack, kMagenta, kGreen, kCyan, kYellow

import Analysis.Tools.tdrstyle as tdrstyle


class MultiRatioPlot:
    def __init__(self):
        tdrstyle.setTDRStyle()
        TH1.SetDefaultSumw2()
        self._histograms = []
        self._n_histos = 0
        self._ratio_plots = []
        self._lower_graphs = []
        self._canvases = []
        self._lower_pad = None
        self._upper_pad = None
        self._hmaxlist = []
        self._hminlist = []
        self._upper_max_y = -1111
        self._upper_min_y = -1111
        self._lower_max_y = 2.
        self._lower_min_y = 0.
        self._canvas_w = 800
        self._canvas_h = 700
        self._text_size = 0.04
        self._left_margin = 0.16
        self._right_margin = 0.08
        self._low_bottom_margin = 0.3
        self._low_top_margin = 0.01
        self._up_bottom_margin = 0.07
        self._label_offset_x = 0.008
        self._label_offset_y = 0.008
        self._title_offset_x = 1.25
        self._title_offset_y = 2.0
        self._up_pad = (0.0025,0.375,0.9975,0.9975)
        self._low_pad = (0.0025,0.0025,0.9975,0.375)
        self._low_title_y = ""
        self._colors = [kBlack, kRed, kBlue, kGreen, kMagenta, kCyan, kYellow]
        self._markers = [20,21,22,23,24,25,26]
        self._legend = None
        self._legend_position = (0.50,0.55,0.91,0.89)
        self._legend_header = None
      
    def add(self,h):
        h.SetLineColor(self._colors[self._n_histos])
        h.SetMarkerColor(self._colors[self._n_histos])
        h.SetMarkerStyle(self._markers[self._n_histos])
        self._histograms.append(h)
        self._n_histos += 1
        self._hmaxlist.append(h.GetMaximum())
        self._hminlist.append(h.GetMinimum())
        
    def upper_range_y(self,min=None,max=None):
        if min:
            self._upper_min_y = min
        if max:           
            self._upper_max_y = max
        
    def lower_range_y(self,min=None,max=None):
        if min:
            self._lower_min_y = min
        if max:
            self._lower_max_y = max
      
    def plot(self):        
        self._ratio_plots = [None]*(self._n_histos-1)
        self._lower_graphs = [None]*(self._n_histos-1)
        self._canvases = [None]*(self._n_histos-1)
        self._hmaxlist.sort()
        self._hminlist.sort()
        max = self._upper_max_y if self._upper_max_y > 0. else self._hmaxlist[-1]
        min = self._upper_min_y if self._upper_min_y > 0. else self._hminlist[0]
        for h in self._histograms:
            h.SetMinimum(min)
            h.SetMaximum(max)
        n_canvas = self._n_histos-1
        for i in range(0,self._n_histos-1):
            self._canvases[i] = TCanvas("c"+str(i),"",self._canvas_w,self._canvas_h)
            self._canvases[i].cd()
            self._ratio_plots[i]=TRatioPlot(self._histograms[i+1],self._histograms[0],'divsym')
            self._prepare_plots(self._ratio_plots[i])
            self._lower_graphs[i] = self._ratio_plots[i].GetLowerRefGraph()

        # close unnecessary canvases
        for i in range(1,len(self._canvases)): # close unnecessary canvases
            self._canvases[i].Close()
        # pads of the main canvas

        self._prepare_pads()
        
        # ranges
        self._ratio_plots[0].GetLowerRefYaxis().SetRangeUser(self._lower_min_y, self._lower_max_y)


        # draw additional histograms in the upper pad
        self._upper_pad.cd()
        for i in range(2,self._n_histos):
            self._histograms[i].Draw("same")
        
        # legend
        self._prepare_legend()
        
        # draw additional graphs in the lower pad
        self._lower_pad.cd()
        for i in range(1,len(self._lower_graphs)):
            self._lower_graphs[i].Draw("P")
            
        self._canvases[0].Update()
        self._canvases[0].Draw()
                    
        
        return self._canvases[0],self._ratio_plots[0]
        
    def canvas(self):
        return self._canvases[0]
    
    def canvas(self, width, height):
        self._canvas_w = width
        self._canvas_h = height
        if width/height != 0.875: # use optimal proportion + larger plot
            self._canvas_w = max(width,height)
            self._canvas_h = round(self._canvas_w*0.875)
    
    def ratio_plot(self):
        return self._ratio_plots[0]
    
    def lower_title_y(self,title):
        self._low_title_y = title
   
    def legend_header(self,header=None):
        if header:
            self._legend_header = header
    
    def _prepare_plots(self, ratio_plot):
        ratio_plot.SetH1DrawOpt("e")
        ratio_plot.SetH2DrawOpt("e")
        ratio_plot.GetLowYaxis().SetNdivisions(6,5,0)
        ratio_plot.SetSeparationMargin(0.025)
        
        # dashed lines in lower plot
        lines = ROOT.std.vector('double')()
        # lines.push_back(1)
        ratio_plot.SetGridlines(lines)
        
        # draw ratio plot
        ratio_plot.Draw()

        # LOWER X AXIS
        ratio_plot.GetLowerRefXaxis().SetLabelSize(self._text_size)
        ratio_plot.GetLowerRefXaxis().SetTitleSize(self._text_size)
        ratio_plot.GetLowerRefXaxis().SetLabelOffset(self._label_offset_x)
        ratio_plot.GetLowerRefXaxis().SetTitleOffset(self._title_offset_x)
        
        # LOWER Y AXIS
        ratio_plot.GetLowerRefYaxis().SetLabelSize(self._text_size)
        ratio_plot.GetLowerRefYaxis().SetTitleSize(self._text_size)
        ratio_plot.GetLowerRefYaxis().SetLabelOffset(self._label_offset_y)
        ratio_plot.GetLowerRefYaxis().SetTitleOffset(self._title_offset_y)
        ratio_plot.GetLowerRefYaxis().SetTitle(self._low_title_y)
        
        # UPPER Y AXIS
        ratio_plot.GetUpperRefYaxis().SetLabelSize(self._text_size)
        ratio_plot.GetUpperRefYaxis().SetTitleSize(self._text_size)
        ratio_plot.GetUpperRefYaxis().SetLabelOffset(self._label_offset_y)
        ratio_plot.GetUpperRefYaxis().SetTitleOffset(self._title_offset_y)
        
        ratio_plot.SetLowBottomMargin(self._low_bottom_margin)
        ratio_plot.SetUpBottomMargin(self._up_bottom_margin)
        ratio_plot.SetLowTopMargin(self._low_top_margin)
        
    def _prepare_pads(self):
        self._canvases[0].cd()
        gPad.Update()
        self._lower_pad = self._ratio_plots[0].GetLowerPad()
        self._upper_pad = self._ratio_plots[0].GetUpperPad()
        self._lower_pad.SetLeftMargin(self._left_margin)
        self._upper_pad.SetLeftMargin(self._left_margin)
        self._lower_pad.SetRightMargin(self._right_margin)
        self._upper_pad.SetRightMargin(self._right_margin)
        
        self._upper_pad.SetPad(self._up_pad[0], self._up_pad[1], self._up_pad[2], self._up_pad[3])
        self._lower_pad.SetPad(self._low_pad[0], self._low_pad[1], self._low_pad[2], self._low_pad[3])

    def _prepare_legend(self):
        # build legend
        self._legend = TLegend(self._legend_position[0],
                         self._legend_position[1],
                         self._legend_position[2],
                         self._legend_position[3])
        if self._legend_header:
            self._legend.SetHeader(self._legend_header)
        for h in self._histograms:
            self._legend.AddEntry(h,h.GetTitle(),"ep")
        self._legend.SetTextSize(self._text_size*1.25)
        self._legend.SetBorderSize(0)
        self._legend.Draw()
        
        
        
