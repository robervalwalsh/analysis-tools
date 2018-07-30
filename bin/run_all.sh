#!/bin/bash
# This script creates all the possible plots with
# all types of correction level for the AnalysisJetsSmrBTagRegFSR.cc

CORRECTION_LEVELS=(nothing only_smearing smearing_btag smearing_btag_regression smearing_btag_regression_fsr)
TRIGGER_FLAGS=("true" "false")
PFILE=Parameters.cc


if [[ $# -ne 1 ]] ; then
    echo "Program called with wrong number of arguments."
    exit 1
fi

case $1 in
    "base")
        OUTPUT_DIR="output/hists/jets/script"
        EXECUTABLE="AnalysisJetsSmrBTagRegFSR"
    ;;
    "match")
        OUTPUT_DIR="output/hists/jets/script_match"
        EXECUTABLE="AnalysisJetsSmrBTagRegFSRMatch"
    ;;
    *)
        echo "./run_all {base|match}"
        exit 2
    ;;
esac
    

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
            ${EXECUTABLE} ${OUT_FILE}
        else
            echo -e "\033[0;31mFile already exists. Not overwriting\033[0m"
        fi
    done
done
