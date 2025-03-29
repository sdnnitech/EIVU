#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <builddir> <batchsz> <vqsz> <output_file>"
    exit 1
fi

RESULT_FILE=$4


# Intel Core i9 11900K (Server1) 
opt_perf_svr1="-e L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,dTLB-loads,dTLB-load-misses,dTLB-stores,dTLB-store-misses,LLC-loads,LLC-stores,cache-misses,cache-references,l1d.replacement,l1d_pend_miss.fb_full,l1d_pend_miss.pending_cycles,l2_lines_in.all,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,l2_rqsts.all_demand_data_rd,l2_rqsts.all_demand_miss,l2_rqsts.all_rfo,l2_rqsts.demand_data_rd_hit,l2_rqsts.demand_data_rd_miss,l2_rqsts.miss,l2_rqsts.references,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,l2_rqsts.swpf_hit,l2_rqsts.swpf_miss,l2_trans.l2_wb,longest_lat_cache.miss,mem_load_l3_hit_retired.xsnp_hit,mem_load_l3_hit_retired.xsnp_hitm,mem_load_l3_hit_retired.xsnp_miss,mem_load_l3_hit_retired.xsnp_none,mem_load_retired.fb_hit,mem_load_retired.l1_hit,mem_load_retired.l1_miss,mem_load_retired.l2_hit,mem_load_retired.l2_miss,mem_load_retired.l3_hit,mem_load_retired.l3_miss,ocr.demand_data_rd.l3_hit.snoop_hitm,ocr.demand_rfo.l3_hit.snoop_hitm,offcore_requests.all_requests,offcore_requests.demand_data_rd,offcore_requests.demand_rfo,offcore_requests_outstanding.cycles_with_data_rd,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_requests_outstanding.demand_data_rd,sw_prefetch_access.any,sw_prefetch_access.nta,sw_prefetch_access.prefetchw,sw_prefetch_access.t0,sw_prefetch_access.t1_t2,cycle_activity.stalls_l3_miss,ocr.demand_data_rd.l3_miss,ocr.demand_rfo.l3_miss,offcore_requests.l3_miss_demand_data_rd,ocr.demand_data_rd.any_response,ocr.demand_data_rd.dram,ocr.demand_rfo.any_response,ocr.streaming_wr.any_response,cycle_activity.cycles_l1d_miss,cycle_activity.cycles_l2_miss,cycle_activity.cycles_mem_any,inst_retired.any,inst_retired.nop,load_hit_prefetch.swpf"


# Intel Core Ultra 9 285K (Server2)
opt_perf_svr2="-e L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,dTLB-loads,dTLB-load-misses,dTLB-stores,dTLB-store-misses,LLC-loads,LLC-stores,cache-misses,cache-references"


# Intel Xeon w7-2475X (Server3)
opt_perf_svr3="-e L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,dTLB-loads,dTLB-load-misses,dTLB-stores,dTLB-store-misses,LLC-loads,LLC-stores,cache-misses,cache-references,l1d.replacement,l1d_pend_miss.fb_full,l1d_pend_miss.pending_cycles,l2_lines_in.all,l2_lines_out.non_silent,l2_lines_out.silent,l2_lines_out.useless_hwpf,l2_rqsts.all_demand_data_rd,l2_rqsts.all_demand_miss,l2_rqsts.all_rfo,l2_rqsts.demand_data_rd_hit,l2_rqsts.demand_data_rd_miss,l2_rqsts.miss,l2_rqsts.references,l2_rqsts.rfo_hit,l2_rqsts.rfo_miss,l2_rqsts.swpf_hit,l2_rqsts.swpf_miss,l2_trans.l2_wb,longest_lat_cache.miss,mem_load_retired.fb_hit,mem_load_retired.l1_hit,mem_load_retired.l1_miss,mem_load_retired.l2_hit,mem_load_retired.l2_miss,mem_load_retired.l3_hit,mem_load_retired.l3_miss,ocr.demand_data_rd.l3_hit.snoop_hitm,ocr.demand_rfo.l3_hit.snoop_hitm,offcore_requests.all_requests,offcore_requests.demand_data_rd,offcore_requests.demand_rfo,offcore_requests_outstanding.cycles_with_data_rd,offcore_requests_outstanding.cycles_with_demand_rfo,offcore_requests_outstanding.demand_data_rd,sw_prefetch_access.any,sw_prefetch_access.nta,sw_prefetch_access.prefetchw,sw_prefetch_access.t0,sw_prefetch_access.t1_t2,cycle_activity.stalls_l3_miss,ocr.demand_data_rd.l3_miss,ocr.demand_rfo.l3_miss,offcore_requests.l3_miss_demand_data_rd,ocr.demand_data_rd.any_response,ocr.demand_data_rd.dram,ocr.demand_rfo.any_response,ocr.streaming_wr.any_response,cycle_activity.cycles_l1d_miss,cycle_activity.cycles_l2_miss,cycle_activity.cycles_mem_any,inst_retired.any,inst_retired.nop,load_hit_prefetch.swpf"


# AMD Threadripper 5965WX (Server4)
opt_perf_svr4=$opt_perf_svr2


# unsupported: l1d.hwpf_miss,l1d_pend_miss.l2_stalls,l2_request.all,l2_request.miss,l2_rqsts.all_core_rd,l2_rqsts.all_hwpf,l2_rqsts.hwpf_miss,longest_lat_cache.reference,mem_load_completed.l1_miss_any,mem_load_l3_hit_retired.xsnp_fwd,mem_load_l3_hit_retired.xsnp_no_fwd,mem_load_l3_miss_retired.local_dram,mem_store_retired.l2_hit,mem_uop_retired.any,ocr.demand_data_rd.l3_hit.snoop_hit_with_fwd,offcore_requests.data_rd,offcore_requests.demand_core_rd,offcore_requests_outstanding.cycles_with_demand_core_rd,offcore_requests_outstanding.cycles_with_demand_data_rd,offcore_requests_outstanding.data_rd,offcore_requests_outstanding.demand_core_rd,memory_trans_retired.store_sample,memory_activity.cycles_l1d_miss,memory_activity.stalls_l1d_miss,memory_activity.stalls_l2_miss,memory_activity.stalls_l3_miss,offcore_requests_outstanding.l3_miss_demand_data_rd,cycles_activity.stalls_l1d_miss,cycles_activity.stalls_l2_miss,cycles_activity.stalls_total



########## MODIFY HERE ##########
opt_perf=$opt_perf_svr1
#################################


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

