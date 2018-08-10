#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ROOT import TMVA, TFile, TTree, TCut

output_filename = "bdt_output.root"
input_signal_filename = "signal.root"
input_bkg_filename = "bkg.root"



variables_f = ['Jet_newPt[0]', 'Jet_newPt[1]', "Jet_newPt[2]",'Jet_newEta[0]', 'Jet_newEta[1]', 'Jet_newEta[2]', 'Jet_newPhi[0]', 'Jet_newPhi[1]', 'Jet_newPhi[1]', "Jet_btagDeepB[0]", 'Jet_btagDeepB[1]', 'Jet_btagDeepB[2]']
variables_ui = list()  # ["Jet_nElectrons", "Jet_nMuons"]
variables_i = ["Jet_puId", ]



input_signal_file = TFile(input_signal_filename, "read")
if input_signal_file.IsZombie():
    raise RuntimeError("Error opening signal file " + input_signal_filename)

signal_tree = input_signal_file.Get("output_tree")
if signal_tree.IsZombie():
    raise RuntimeError("Error opening signal_tree")


input_bkg_file = TFile(input_bkg_filename, "read")
if input_bkg_file.IsZombie():
    raise RuntimeError("Error opening bkg file " + input_bkg_filename)

bkg_tree = input_bkg_file.Get("output_tree")
if bkg_tree.IsZombie():
    raise RuntimeError("Error opening bkg_tree")

output_file = TFile(output_filename, "recreate")
if output_file.IsZombie():
    raise RuntimeError("Error opening output_file " + output_filename)


factory = TMVA.Factory("", output_file, "AnalysisType=Classification")
dataloader = TMVA.DataLoader("dataset")


for v in variables_f:
    dataloader.AddVariable(v, "F")
for v in variables_i:
    dataloader.AddVariable(v, 'I')
for v in variables_ui:
    dataloader.AddVariable(v, 'i')

dataloader.AddSignalTree(signal_tree, 1.)
dataloader.AddBackgroundTree(bkg_tree, 1.)

cut_no_nan_deep = "!(Jet_btagDeepB != Jet_btagDeepB)"
cut_leptonic = "Leptonic_event"

cuts = TCut(" && ".join([cut_leptonic, cut_no_nan_deep]))

dataloader.PrepareTrainingAndTestTree(cuts, cuts, "")

factory.BookMethod(dataloader, TMVA.Types.kBDT, "BDT", "")

factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()

output_file.Close()


