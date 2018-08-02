#!/bin/bash


CORRECTION_LEVELS=(nothing_false only_smearing_false smearing_btag_false smearing_btag_regression_false smearing_btag_regression_fsr_false smearing_btag_regression_fsr_true)
RANGE=(0 1 2)
MASSPOINTS=(120 350 1200)
OUTPUT_DIR_BASE="output/hists/jets"

hash -r
for i in ${RANGE[@]} ; do
    OUTPUT_DIR="${OUTPUT_DIR_BASE}/test_${MASSPOINTS[i]}"
    mkdir -p ${OUTPUT_DIR}
    for corr in ${CORRECTION_LEVELS[@]} ; do
        if ! [[ -f ${OUT_FILE} ]] ; then
            OUT_FILE="${OUTPUT_DIR}/${corr}.root"
            ${MASSPOINTS[i]}_${corr} ${OUT_FILE} & > /dev/null
        else
            echo -e "\033[0;31mFile already exists. Not overwriting. ${OUT_FILE}\033[0m"
        fi
    done
done

wait
echo -e "\n\nAll process exited.\n\nSuccess.\n\n"
