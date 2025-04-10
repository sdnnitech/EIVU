#!/bin/sh

cd "$(dirname $0)/../.."


MESON_OPT_BASE=""
MESON_OPT_FAST="-Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=64 -Dzero_copy=rx,tx"


function setup() {
	if [ $1 = 'base' ]; then
		MESON_OPT=$MESON_OPT_BASE
	else
		MESON_OPT=$MESON_OPT_FAST
	fi

	meson setup output/bin/$1/coupled/$2 -Dmetadata_size=$2 $MESON_OPT
}

function build() {
	ninja -C output/bin/$1/coupled/$2
}

function setup_and_build() {
	setup $1 $2
	build $1 $2
}


METADATA_SIZES=(0 2 4 8 16 32 64 128 256)
TYPES=('base' 'fast')

for tp in ${TYPES[@]}; do
	for meta in ${METADATA_SIZES[@]}; do
		meta=$((meta))
		setup_and_build $tp $meta
	done
done

