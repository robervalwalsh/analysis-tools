#!/bin/bash

CORRECTION_LEVELS=(nothing_false regression_false regression_fsr_false regression_fsr_true)
OUTPUT_DIR_BASE="output/hists/bkg"

hash -r
for corr in ${CORRECTION_LEVELS[@]} ; do
    OUTPUT_DIR="$OUTPUT_DIR_BASE/test"
    mkdir -p ${OUTPUT_DIR}
    EXECUTABLE="bkg_${corr}"
    OUT_FILE="${OUTPUT_DIR}/${corr}.root"
    if ! [[ -f ${OUT_FILE} ]] ; then
        $EXECUTABLE ${OUT_FILE} > /dev/null &
    else
        echo -e "\033[0;31mFile already exists. Not overwriting. ${OUT_FILE}\033[0m"
    fi
done

wait
echo -e "\n\nAll process exited.\n\nSuccess.\n\n"
