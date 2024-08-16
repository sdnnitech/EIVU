#!/bin/bash

DIR_WORK=$(pwd)

DIR_BASE="${DIR_WORK}/result_fast"
mkdir $DIR_BASE

RESULT_FILE="${DIR_BASE}/result_fast.csv"
touch $RESULT_FILE

for ((MD_SZ=4; MD_SZ<=128; MD_SZ*=2)) {
    echo "${MD_SZ}," | tr -d '\n' >> $RESULT_FILE
    DIR="${DIR_BASE}/${MD_SZ}"

    git restore ./lib/vnwio/vhost.h ./lib/vnwio/vio.h \
        ./lib/common/option.h ./lib/mbuf/common/mbuf_core.h ./src/rx.c
    ./fasterize.sh $MD_SZ
    meson setup --reconfigure $DIR -Dseparated_md=false -Dfastest_mode=true \
        -Dc_args=-DMETADATA_SIZE=$MD_SZ
    ninja -C $DIR
    ./run.sh $DIR | sed -n 's/.*(\([0-9.]*\) Mpps.*/\1/p' >> $RESULT_FILE
}
