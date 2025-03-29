#!/bin/sh

cd "$(dirname $0)/.."


function make_dir() {
	mkdir -p output/eval/$1/decoupled/batch-$3/aggr-$2
}

function run() {
	./run.sh output/bin/$1/decoupled/aggr-$2/$3 $4 $5 | tee output/eval/$1/decoupled/batch-$4/aggr-$2/result-$3.txt
}

function perfrun() {
	./perfrun.sh output/bin/$1/decoupled/aggr-$2/$3 $4 $5 output/eval/$1/decoupled/batch-$4/aggr-$2/perf-$3.txt
}


TYPES=('fast')
NUM_OF_AGGREGATIONS=(256 512 1024)
METADATA_SIZES=(2 4 8 16 32 64 128 256)
BATCH_SIZES=(32 256 512 1024)
VQ_SIZES=(256 512 1024 2048)

for tp in ${TYPES[@]}; do
	for aggr in ${NUM_OF_AGGREGATIONS[@]}; do
		aggr=$((aggr))

    	for meta in ${METADATA_SIZES[@]}; do
        	meta=$((meta))

			for i in ${!BATCH_SIZES[@]}; do
				batch=${BATCH_SIZES[$i]}
				vq=${VQ_SIZES[$i]}

				batch=$((batch))
				vq=$((vq))

				make_dir $tp $aggr $batch

	        	run $tp $aggr $meta $batch $vq
				sleep 2
				perfrun $tp $aggr $meta $batch $vq
				sleep 2
			done
    	done
	done
done

