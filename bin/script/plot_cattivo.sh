#!/bin/bash
# This script creates all the possible plots with
# all types of correction level for the AnalysisJetsSmrBTagRegFSR.cc

CORRECTION_LEVELS=(nothing only_smearing smearing_btag smearing_btag_regression smearing_btag_regression_fsr)
TRIGGER_FLAGS=("true" "false")
TYPE=("lep" "chr")
MASS_POINTS=(120 350 1200)
OUTPUT_DIR="output/hists/jets"

for mass in ${MASS_POINTS[@]} ; do
    for corr in ${CORRECTION_LEVELS[@]} ; do
        for trig in ${TRIGGER_FLAGS[@]} ; do
            for l in ${TYPE[@]} ; do
                FILE_NAME="${OUTPUT_DIR}/before_new_mass_${mass}/${corr}_${trig}"
                echo "Doing ${FILE_NAME}"
                cd ..
                PlotStackStyle "${FILE_NAME}_${l}.json" "${FILE_NAME}_${l}.png" &
                PlotStackStyle "${FILE_NAME}_${l}.json" "${FILE_NAME}_${l}.pdf" &
                PlotStackStyle "${FILE_NAME}_${l}.json" "${FILE_NAME}_${l}.eps" &
                PlotStackStyle "${FILE_NAME}_${l}.json" "${FILE_NAME}_${l}_root.root" &
                cd output
            done
        done
    done
done

wait
echo "All process exited."
