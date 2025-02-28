#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <builddir> <batchsz> <vqsz> <output_file>"
    exit 1
fi

RESULT_FILE=$4

#opt_perf="-M L1MPKI,L2MPKI,L3MPKI -e mem_load_retired.l1_hit,mem_load_retired.l2_hit,mem_load_retired.l3_hit,l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_response.all_rfo.any_response,offcore_response.demand_rfo.any_response,offcore_response.all_pf_rfo.any_response,offcore_response.pf_l2_rfo.any_response,offcore_response.pf_l3_rfo.any_response,LLC-loads,LLC-stores,l1d_pend_miss.fb_full,l1d_pend_miss.pending_cycles"

opt_perf="-M L1MPKI,L2MPKI,L3MPKI -e mem_load_retired.l1_hit,mem_load_retired.l2_hit,mem_load_retired.l3_hit,cache-misses,cache-references,l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,ocr.demand_rfo.any_response,ocr.hwpf_l2_rfo.any_response,LLC-loads,LLC-stores,l1d_pend_miss.fb_full,l1d_pend_miss.pending_cycles"

#sudo perf stat -M L1MPKI,L2MPKI,L3MPKI -e mem_load_retired.l1_hit,mem_load_retired.l2_hit,mem_load_retired.l3_hit,cache-misses,cache-references,l1d.replacement,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,offcore_requests.demand_rfo,offcore_requests_outstanding.demand_rfo,offcore_requests_outstanding.cycles_with_demand_rfo,ocr.demand_rfo.any_response,ocr.hwpf_l2_rfo.any_response,LLC-loads,LLC-stores,l1d_pend_miss.fb_full,l1d_pend_miss.pending_cycles -- ./build/nf --batchsz=32 --pktnum=100000000 &>> result_vhost-wait-batch.txt

perf="perf stat $opt_perf"

run() {
  pn=100000000

  sudo $2 $1/nf --pktnum=$pn -H --batchsz=$5 --vqsz=$6 --mobjcache=$7 &
  sleep 1
  sudo $3 $1/rx --pktnum=$pn -H --batchsz=$5 --vqsz=$6 --mobjcache=$7 &
  sleep 1
  sudo $4 $1/tx --pktnum=$pn -H --batchsz=$5 --vqsz=$6 --mobjcache=$7
}

batchsz=$2
vqsz=$3
mobjcache=$((2*$vqsz))

echo "batchsz=$batchsz"
echo "vqsz=$vqsz"
echo "mobjcache=$mobjcache"

# perf nf
echo "start profiling of nf"
echo "NF Thread" > $RESULT_FILE
run $1 "${perf}" "" "" $batchsz $vqsz $mobjcache &>> $RESULT_FILE

# perf rx
echo "start profiling of rx"
echo "RX Thread" >> $RESULT_FILE
run $1 "" "${perf}" "" $batchsz $vqsz $mobjcache &>> $RESULT_FILE

# perf tx
echo "start profiling of tx"
echo "TX Thread" >> $RESULT_FILE
run $1 "" "" "${perf}" $batchsz $vqsz $mobjcache &>> $RESULT_FILE
cat $RESULT_FILE | sed -n 's/.*(\([0-9.]*\) Mpps.*/\1/p'

echo "Finish!"

