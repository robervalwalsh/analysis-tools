#!/bin/bash
# This script creates all the possible plots with
# all types of correction level for the AnalysisJetsSmrBTagRegFSR.cc

CORRECTION_LEVELS=(nothing only_smearing smearing_btag smearing_btag_regression smearing_btag_regression_fsr)
TRIGGER_FLAGS=("true" "false")
RANGE=(0 1 2)
MASSPOINTS=(120 350 1200)
PFILE=Parameters.cc

OUTPUT_DIR_BASE="output/hists/jets"
EXECUTABLE="AnalysisJetsSmrBTagRegFSRMatch"

hash -r

echo "Testing before start"
for f in ${INPUTFILES[@]} ; do
    if ! [[ -f $f ]] ; then
        echo "File $f is not readable. exiting"
        exit 1
    fi
done


mkdir -p _tmp
for i in ${RANGE[@]} ; do
    OUTPUT_DIR="${OUTPUT_DIR_BASE}/mass_${MASSPOINTS[i]}"
    mkdir -p $OUTPUT_DIR
    cp "params/mass_${MASSPOINTS[i]}.cc" _tmp/mass_point.cc
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
done
