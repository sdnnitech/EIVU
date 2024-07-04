#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <builddir>"
    exit 1
fi

pn=102400000

sudo $1/nf --pktnum=$pn -H &
sleep 3
sudo $1/rx --pktnum=$pn -H &
sleep 3
sudo $1/tx --pktnum=$pn -H

