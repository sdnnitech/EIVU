#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <builddir> <batchsz>"
    exit 1
fi

pn=102400000

if [ $2 -lt 128 ]; then
    vqsz=256
else
    vqsz=$(expr 2 \* $2)
fi

echo "pktnum=${pn}"
echo "batchsz=$2"
echo "vqsz=$vqsz"

sudo $1/nf --pktnum=$pn -H --batchsz=$2 --vqsz=$vqsz --mobjcache=$vqsz &
sleep 3
sudo $1/rx --pktnum=$pn -H --batchsz=$2 --vqsz=$vqsz --mobjcache=$vqsz &
sleep 3
sudo $1/tx --pktnum=$pn -H --batchsz=$2 --vqsz=$vqsz --mobjcache=$vqsz

