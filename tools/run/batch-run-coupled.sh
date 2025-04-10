#!/bin/sh

cd "$(dirname $0)/../.."


function make_dir() {
	mkdir -p output/eval/$1/coupled/batch-$2
}

function run() {
	RUN="./run.sh output/bin/$1/coupled/$2 $3 $4 | tee output/eval/$1/coupled/batch-$3/result-$2.txt"
	echo $RUN
	eval $RUN
}

function perfrun() {
	PERFRUN="./perfrun.sh output/bin/$1/coupled/$2 $3 $4 output/eval/$1/coupled/batch-$3/perf-$2.txt"
	echo $PERFRUN
	eval $PERFRUN
}


TYPES=('base' 'fast')
METADATA_SIZES=(0 2 4 8 16 32 64 128 256)
BATCH_SIZES=(32 256 512 1024)
VQ_SIZES=(256 512 1024 2048)

for tp in ${TYPES[@]}; do
    for meta in ${METADATA_SIZES[@]}; do
        meta=$((meta))

		for i in ${!BATCH_SIZES[@]}; do
			batch=${BATCH_SIZES[$i]}
			vq=${VQ_SIZES[$i]}

			batch=$((batch))
			vq=$((vq))

			make_dir $tp $batch

	        run $tp $meta $batch $vq
			sleep 2
			perfrun $tp $meta $batch $vq
			sleep 2
		done
    done
done

