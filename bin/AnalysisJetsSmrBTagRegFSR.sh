#!/bin/bash
# This script creates all the possible plots with
# all types of correction level for the AnalysisJetsSmrBTagRegFSR.cc

OUTPUT_DIR=output/hists/jets/script
CORRECTION_LEVELS=(nothing only_smearing smearing_btag smearing_btag_regression smearing_btag_regression_fsr)
TRIGGER_FLAGS=("true" "false")
PFILE=Parameters.cc

cmsenv
hash -r
mkdir -p $OUTPUT_DIR

for corr in ${CORRECTION_LEVELS[@]} ; do
    for trig in ${TRIGGER_FLAGS[@]} ; do
        OUT_FILE="${OUTPUT_DIR}/${corr}_${trig}.root"
        if ! [[ -f ${OUT_FILE} ]] ; then
            echo "const CorrectionLevel correction_level = CorrectionLevel::${corr};" > $PFILE
            echo "const bool apply_trigger_cut = ${trig};" >> $PFILE
            scram b
            AnalysisJetsSmrBTagRegFSR ${OUT_FILE}
        else
            echo -e "\033[0;31mFile already exists. Not overwriting\033[0m"
        fi
    done
done
