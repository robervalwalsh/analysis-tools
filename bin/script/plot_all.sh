#!/bin/bash
# This script creates all the possible plots with
# all types of correction level for the AnalysisJetsSmrBTagRegFSR.cc

CORRECTION_LEVELS=(nothing only_smearing smearing_btag smearing_btag_regression smearing_btag_regression_fsr)
TRIGGER_FLAGS=("true" "false")
TYPE=("lep" "chr")

if [[ $# -ne 1 ]] ; then
    echo "Program called with wrong number of arguments."
    exit 1
fi

case $1 in
    "base")
        OUTPUT_DIR="output/hists/jets/script"
    ;;
    "match")
        OUTPUT_DIR="output/hists/jets/script_match_1200"
    ;;
    *)
        echo "./plot_all {base|match}"
        exit 2
    ;;
esac


for corr in ${CORRECTION_LEVELS[@]} ; do
    for trig in ${TRIGGER_FLAGS[@]} ; do
        if [[ $1 == "match" ]] ; then
            for l in ${TYPE[@]} ; do
                FILE_NAME="${OUTPUT_DIR}/${corr}_${trig}"
                cd ..
                PlotStackStyle "${FILE_NAME}_${l}.json" "${FILE_NAME}_${l}.png"
                PlotStackStyle "${FILE_NAME}_${l}.json" "${FILE_NAME}_${l}.pdf"
                PlotStackStyle "${FILE_NAME}_${l}.json" "${FILE_NAME}_${l}_root.root"
                cd output
            done
        else
            FILE_NAME="${OUTPUT_DIR}/${corr}_${trig}"
            cd ..
            PlotStackStyle "${FILE_NAME}.json" "${FILE_NAME}.png"
            PlotStackStyle "${FILE_NAME}.json" "${FILE_NAME}.pdf"
            cd output
        fi
    done
done
