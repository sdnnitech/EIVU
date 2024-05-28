#!/bin/bash
pn=100000000
./build/nf --pktnum=$pn &
sleep 1
./build/rx --pktnum=$pn &
sleep 1
./build/tx --pktnum=$pn
