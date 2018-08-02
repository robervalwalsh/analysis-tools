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
CMS_BIN_DIR="${HOME}/Documents/cmssw/CMSSW_9_4_9/bin/slc6_amd64_gcc630"

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
    OUTPUT_DIR="${OUTPUT_DIR_BASE}/before_new_mass_${MASSPOINTS[i]}"
    NEW_BIN_DIR="_tmp_bin/mass_${MASSPOINTS[i]}"
    mkdir -p $NEW_BIN_DIR
    mkdir -p $OUTPUT_DIR
    cp "params/mass_${MASSPOINTS[i]}.cc" _tmp/mass_point.cc
    for corr in ${CORRECTION_LEVELS[@]} ; do
        for trig in ${TRIGGER_FLAGS[@]} ; do
            OUT_FILE="${OUTPUT_DIR}/${corr}_${trig}.root"
            echo "const CorrectionLevel correction_level = CorrectionLevel::${corr};" > $PFILE
            echo "const bool apply_trigger_cut = ${trig};" >> $PFILE
            scram b -f
            cp "${CMS_BIN_DIR}/${EXECUTABLE}" "${NEW_BIN_DIR}/${corr}_${trig}"
        done
    done
done

for i in ${RANGE[@]} ; do
    NEW_BIN_DIR="_tmp_bin/mass_${MASSPOINTS[i]}"
    OUTPUT_DIR="${OUTPUT_DIR_BASE}/before_new_mass_${MASSPOINTS[i]}"
    for corr in ${CORRECTION_LEVELS[@]} ; do
        for trig in ${TRIGGER_FLAGS[@]} ; do
            if ! [[ -f ${OUT_FILE} ]] ; then
                OUT_FILE="${OUTPUT_DIR}/${corr}_${trig}.root"
                ${NEW_BIN_DIR}/${corr}_${trig} ${OUT_FILE} & > /dev/null
            else
                echo -e "\033[0;31mFile already exists. Not overwriting. ${OUTPUT_DIR}/${corr}_${trig}.root\033[0m"
            fi
        done
    done
done


wait
echo -e "\n\nAll process exited.\n\nSuccess.\n\n"
