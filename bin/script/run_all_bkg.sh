#!/bin/bash

CORRECTION_LEVELS=(nothing_false regression_false regression_fsr_false regression_fsr_true)
OUTPUT_DIR_BASE="output/hists/bkg"
MAX_N=12
OUTPUT_DIR="$OUTPUT_DIR_BASE/test_new"


hash -r
for corr in ${CORRECTION_LEVELS[@]} ; do
    mkdir -p ${OUTPUT_DIR}
    for i in $(seq 0 ${MAX_N}) ; do 
        EXECUTABLE="bkg_${corr}_${i}"
        OUT_FILE="${OUTPUT_DIR}/${corr}_${i}.root"
        if ! [[ -f ${OUT_FILE} ]] ; then
            $EXECUTABLE ${OUT_FILE} > /dev/null &
        else
            echo -e "\033[0;31mFile already exists. Not overwriting. ${OUT_FILE}\033[0m"
        fi
    done
done


wait
echo -e "\n\nAll process exited. Now I merge"

for corr in ${CORRECTION_LEVELS[@]} ; do
    INFILES=$(ls ${OUTPUT_DIR} | grep ${corr})
    hadd ${corr}.root $INFILES
    rm $INFILES
done

echo -e "\n\nMerged everything"
        
