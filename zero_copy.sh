#!/bin/bash

# Skip packet copy
sed -i -E 's/^(.*)dpdk_prefetch.*/\1memcpy\(NULL, NULL, 0\);/g' lib/vnwio/vhost.h
sed -i -E 's/memcpy\((.*), (.*), .*\);$/memcpy\(\1, \2, 0\);/g' lib/vnwio/vhost.h
