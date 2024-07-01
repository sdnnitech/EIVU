#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <builddir>"
    exit 1
fi

pn=100000000

$1/nf --pktnum=$pn &
sleep 3
$1/rx --pktnum=$pn &
sleep 3
$1/tx --pktnum=$pn
