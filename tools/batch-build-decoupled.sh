#!/bin/sh

cd "$(dirname $0)/.."


MESON_OPT_BASE=""
MESON_OPT_FAST="-Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=64 -Dzero_copy=rx,tx"

function setup() {
	if [ $1 = 'base' ]; then
		MESON_OPT=$MESON_OPT_BASE
	else
		MESON_OPT=$MESON_OPT_FAST
	fi

	meson setup output/bin/$1/decoupled/aggr-$2/$3 -Dmetadata_size=$3 $MESON_OPT -Daggregated_md=true -Dhost_aggregated_md=true -Daggregation_num=$2
}

function build() {
	ninja -C output/bin/$1/decoupled/aggr-$2/$3
}

function setup_and_build() {
	setup $1 $2 $3
	build $1 $2 $3
}


METADATA_SIZES=(2 4 8 16 32 64 128 256)
NUM_OF_AGGREGATIONS=(1 256 512 1024)
TYPES=('base' 'fast')

for tp in ${TYPES[@]}; do
	for aggr in ${NUM_OF_AGGREGATIONS[@]}; do
		aggr=$((aggr))
		for meta in ${METADATA_SIZES[@]}; do
			meta=$((meta))
			setup_and_build $tp $aggr $meta
		done
	done
done

