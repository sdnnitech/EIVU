#!/bin/sh

cd "$(dirname $0)/.."


function setup_a() {
	meson setup output/bin/fast/coupled/factors/a/$2           -Dmetadata_size=$2 -Dtiny_descs=true -Dheadroom_size=64 -Ddataroom_size=64 -Dzero_copy=rx,tx
	meson setup output/bin/fast/decoupled/aggr-$1/factors/a/$2 -Dmetadata_size=$2 -Dtiny_descs=true -Dheadroom_size=64 -Ddataroom_size=64 -Dzero_copy=rx,tx -Daggregated_md=true -Dhost_aggregated_md=true -Daggregation_num=$1
}

function build_a() {
	ninja -C output/bin/fast/coupled/factors/a/$2
	ninja -C output/bin/fast/decoupled/aggr-$1/factors/a/$2
}

function setup_and_build_a() {
	setup_a $1 $2
	build_a $1 $2 
}


function setup_b() {
	meson setup output/bin/fast/coupled/factors/b/$2           -Dmetadata_size=$2 -Dvio_header=false -Dheadroom_size=0 -Ddataroom_size=64 -Dzero_copy=rx,tx
	meson setup output/bin/fast/decoupled/aggr-$1/factors/b/$2 -Dmetadata_size=$2 -Dvio_header=false -Dheadroom_size=0 -Ddataroom_size=64 -Dzero_copy=rx,tx -Daggregated_md=true -Dhost_aggregated_md=true -Daggregation_num=$1
}

function build_b() {
	ninja -C output/bin/fast/coupled/factors/b/$2
	ninja -C output/bin/fast/decoupled/aggr-$1/factors/b/$2
}

function setup_and_build_b() {
	setup_b $1 $2
	build_b $1 $2
}


function setup_d() {
	meson setup output/bin/fast/coupled/factors/d/droom-$3/$2           -Dmetadata_size=$2 -Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=$3 -Dzero_copy=rx,tx
	meson setup output/bin/fast/decoupled/aggr-$1/factors/d/droom-$3/$2 -Dmetadata_size=$2 -Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=$3 -Dzero_copy=rx,tx -Daggregated_md=true -Dhost_aggregated_md=true -Daggregation_num=$1
}

function build_d() {
	ninja -C output/bin/fast/coupled/factors/d/droom-$3/$2
	ninja -C output/bin/fast/decoupled/aggr-$1/factors/d/droom-$3/$2
}

function setup_and_build_d() {
	DATAROOM_SIZES=(256 2048)

	for droom_size in ${DATAROOM_SIZES[@]}; do
		droom_size=$((droom_size))

		setup_d $1 $2 $droom_size
		build_d $1 $2 $droom_size
	done
}


function setup_e() {
	if [ $3 = 'rx' ]; then
		ZCOPY_OPT='-Dzero_copy=tx'
	elif [ $3 = 'tx' ]; then
		ZCOPY_OPT='-Dzero_copy=rx'
	else
		ZCOPY_OPT=''
	fi

	meson setup output/bin/fast/coupled/factors/e/$3-copy/$2           -Dmetadata_size=$2 -Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=64 $ZCOPY_OPT
	meson setup output/bin/fast/decoupled/aggr-$1/factors/e/$3-copy/$2 -Dmetadata_size=$2 -Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=64 $ZCOPY_OPT -Daggregated_md=true -Dhost_aggregated_md=true -Daggregation_num=$1
}

function build_e() {
	ninja -C output/bin/fast/coupled/factors/e/$3-copy/$2
	ninja -C output/bin/fast/decoupled/aggr-$1/factors/e/$3-copy/$2
}

function setup_and_build_e() {
	COPY_TYPE=('rx' 'tx' 'full')

	for tp in ${COPY_TYPE[@]}; do
		setup_e $1 $2 $tp
		build_e $1 $2 $tp
	done
}



METADATA_SIZES=(0 2 4 8 16 32 64 128 256)
NUM_OF_AGGREGATIONS=(512)

for aggr in ${NUM_OF_AGGREGATIONS[@]}; do
	for meta in ${METADATA_SIZES[@]}; do
		aggr=$((aggr))
		meta=$((meta))

		setup_and_build_a $aggr $meta
		setup_and_build_b $aggr $meta
		setup_and_build_d $aggr $meta
		setup_and_build_e $aggr $meta
	done
done

