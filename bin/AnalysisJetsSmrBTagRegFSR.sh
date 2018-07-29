#!/bin/bash
# This script creates all the possible plots with
# all types of correction level for the AnalysisJetsSmrBTagRegFSR.cc

OUTPUT_DIR=output/hists/jets/script
CORRECTION_LEVELS=(nothing only_smearing smearing_btag smearing_btag_regression smearing_btag_regression_fsr)
TRIGGER_FLAGS=("true" "false")
PFILE=Parameters.cc

set -v
cmsenv
hash -r

for corr in ${CORRECTION_LEVELS[@]} ; do
    for trig in ${TRIGGER_FLAGS[@]} ; do
        echo "const CorrectionLevel correction_level = CorrectionLevel::${corr};" > $PFILE
        echo "const bool apply_trigger_cut = ${trig};" >> $PFILE
        scram b
        AnalysisJetsSmrBTagRegFSR ${OUTPUT_DIR}/${CORRECTION_LEVELS}_${TRIGGER_FLAGS}.root
    done
done
