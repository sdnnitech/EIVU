#!/bin/bash

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <builddir> <batchsz> <vqsz>"
    exit 1
fi

pn=100000000

batchsz=$2
vqsz=$3
mobjcache=$vqsz

echo "batchsz=$batchsz"
echo "vqsz=$vqsz"
echo "mobjcache=${mobjcache}"

sudo $1/nf --pktnum=$pn -H --batchsz=$2 --vqsz=$vqsz --mobjcache=$mobjcache &
sleep 3
sudo $1/rx --pktnum=$pn -H --batchsz=$2 --vqsz=$vqsz --mobjcache=$mobjcache &
sleep 3
sudo $1/tx --pktnum=$pn -H --batchsz=$2 --vqsz=$vqsz --mobjcache=$mobjcache

