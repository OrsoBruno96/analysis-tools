#!/bin/bash
# -*- coding:utf-8 -*-

BASE_DIR="/nfs/dust/cms/user/zorattif/output/raw_files"
SPECIFIC_DIR="distribution_jets_3_FSR_pt_60_deltar_08/medium_wp"

CASES=(bkg signal)
TRIG=(true false)
SCRIPTS_DIR="${HOME}/workdir/bin/_tmp"
NEW_SCRIPT_DIR="${HOME}/workdir/bin"

COMMANDS=()

for c in ${CASES[@]} ; do
    if [[ $c == "bkg" ]] ; then
        SCRIPT_FILENAME="${SCRIPTS_DIR}/run_condorbkg.sh"
        LIMIT_STRING="bkg"
    else
        SCRIPT_FILENAME="${SCRIPTS_DIR}/run_condorsig.sh"
        LIMIT_STRING="sig"
    fi
    cd ${BASE_DIR}/${c}/${SPECIFIC_DIR}
    for t in ${TRIG[@]} ; do
        ls -la | grep ${t}.root |
            while IFS= read -r line ; do
                size=$(echo ${line} | cut --delimiter ' ' -f 5)
                if [[ ${size} -lt 1000 ]] ; then
                    name=$(echo ${line} | cut --delimiter ' ' -f 9)
                    name="${name%.*}"
                    name="${name##*${LIMIT_STRING}_}"
                    era="${name:0:1}"
                    corr=$(echo $name | cut -c 3-)
                    cd ${NEW_SCRIPT_DIR}
                    cat $SCRIPT_FILENAME | grep $era | grep $corr | bash
                    cd ${BASE_DIR}/${c}/${SPECIFIC_DIR}
                fi
            done
    done
done
