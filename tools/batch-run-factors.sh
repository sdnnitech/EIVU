#!/bin/sh

cd "$(dirname $0)/.."


function make_dir() {
	mkdir -p output/eval/$1/coupled/batch-$3/factors/$4
	mkdir -p output/eval/$1/decoupled/batch-$3/aggr-$2/factors/$4
}

function run() {
	RUNS=("./run.sh output/bin/$1/coupled/factors/$6/$3           $4 $5 | tee output/eval/$1/coupled/batch-$4/factors/$6/result-$3.txt"
	      "./run.sh output/bin/$1/decoupled/aggr-$2/factors/$6/$3 $4 $5 | tee output/eval/$1/decoupled/batch-$4/aggr-$2/factors/$6/result-$3.txt")
	for RUN in "${RUNS[@]}"; do
		echo $RUN
		eval $RUN
	done
}

function perfrun() {
	PERFRUNS=("./perfrun.sh output/bin/$1/coupled/factors/$6/$3           $4 $5 output/eval/$1/coupled/batch-$4/factors/$6/perf-$3.txt"
	          "./perfrun.sh output/bin/$1/decoupled/aggr-$2/factors/$6/$3 $4 $5 output/eval/$1/decoupled/batch-$4/aggr-$2/factors/$6/perf-$3.txt")
	for PERFRUN in "${PERFRUNS[@]}"; do
		echo $PERFRUN
		eval $PERFRUN
	done
}


TYPES=('fast')
NUM_OF_AGGREGATIONS=(512)
METADATA_SIZES=(0 2 4 8 16 32 64 128 256)
BATCH_SIZES=(512)
VQ_SIZES=(1024)
FACTORS=('a' 'b' 'd/droom-256' 'd/droom-2048' 'e/rx-copy' 'e/tx-copy' 'e/full-copy')

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

				for factor in ${FACTORS[@]}; do
					make_dir $tp $aggr $batch $factor

	        		run $tp $aggr $meta $batch $vq $factor
					sleep 2
					perfrun $tp $aggr $meta $batch $vq $factor
					sleep 2
				done
    		done
		done
	done
done
