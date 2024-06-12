#!/bin/bash

OPTION_FILE=./lib/common/option.h
MBUF_FILE=./lib/mbuf/common/mbuf_core.h
VHOST_FILE=./lib/vnwio/vhost.h
VIO_FILE=./lib/vnwio/vio.h

# BATCH_SIZE = 1024
sed -i -e 's/^#define BATCH_SIZE_DEFAULT 32$/#define BATCH_SIZE_DEFAULT 1024/' $OPTION_FILE

# VQ_SIZE = 2048
sed -i -e 's/^#define VQ_SIZE_DEFAULT 256$/#define VQ_SIZE_DEFAULT 2048/' $OPTION_FILE

# MOBJ_CACHE_NUM = 2048
sed -i -e 's/^#define MOBJ_CACHE_NUM_DEFAULT 512$/#define MOBJ_CACHE_NUM_DEFAULT 2048/' $OPTION_FILE

# MBUF_HEADROOM_SIZE = 0
sed -i -e 's/^#define MBUF_HEADROOM_SIZE 128$/#define MBUF_HEADROOM_SIZE 0/' $MBUF_FILE

# DATAROOM_SIZE = 64
sed -i -e 's/^#define DATAROOM_SIZE (MBUF_HEADROOM_SIZE + 2048)$/#define DATAROOM_SIZE (MBUF_HEADROOM_SIZE + 64)/' $MBUF_FILE

# METADATA_SIZE = 0
sed -i -e 's/^#define METADATA_SIZE 128$/#define METADATA_SIZE 0/' $MBUF_FILE

sed -i -e 's/.*reset_metadata(md);$//g' $MBUF_FILE

sed -i -e 's/.*md->.*//g' ./src/rx.c
sed -i -e 's/.*md->.*//g' $VHOST_FILE
sed -i -E 's/^(.*)vioqueue_enqueue_burst_tx\(vq, (.*), mbp->md->pkt_len\);$/\1vioqueue_enqueue_burst_tx\(vq, \2, 64\);/g' $VIO_FILE
sed -i -e 's/.*md->.*//g' $VIO_FILE
sed -i -e 's/.*dpdk_prefetch.*//g' $VIO_FILE
sed -i -E 's/^(.*)dpdk_prefetch.*/\1memcpy(NULL, NULL, 0);/g' $VHOST_FILE

# Skip packet copy
sed -i -E 's/memcpy\((.*), (.*), .*\);$/memcpy\(\1, \2, 0\);/g' $VHOST_FILE

# desc size = 8
# sed -i -e 's/^    int32_t md_idx;$//' ./lib/vioqueue.h
# sed -i -e 's/^    uint32_t len;$/    uint16_t len;/' ./lib/vioqueue.h
# sed -i -e 's/^    int16_t id;$//' ./lib/vioqueue.h

# No vio_hdr
sed -i -e 's/^    vhost_enqueue_offload((struct vio_hdr *)desc_addr - 1, mbp->md);$/    memset((struct vio_hdr *)desc_addr, 0, 0);/' $VHOST_FILE
sed -i -e 's/^        hdrs[i] = (struct vio_hdr *)desc_addrs[i] - 1;$/        hdrs[i] = (struct vio_hdr *)desc_addrs[i];/' $VHOST_FILE
sed -i -E 's/^(.*)vhost_enqueue_offload\((.*), .*\);$/\1memset\(\2, 0, 0\);/g' $VHOST_FILE
sed -i -e 's/^        vhost_dequeue_offload((struct vio_hdr *)desc_addr - 1);$/        memset((struct vio_hdr *)desc_addr, 0, 0);/' $VHOST_FILE
sed -i -e 's/^            hdr = (struct vio_hdr *)desc_addrs[i] - 1;$/            hdr = (struct vio_hdr *)desc_addrs[i];/' $VHOST_FILE
sed -i -E 's/^(.*)vhost_dequeue_offload\((.*)\);.*$/\1memset\(\2, 0, 0\);/g' $VHOST_FILE

sed -i -E 's/^(.*)vio_rx_offload\((.*) - 1\);$/\1memset\(\2, 0, 0\);/g' $VIO_FILE
sed -i -E 's/^(.*)vio_tx_clear_net_hdr\((.*) - 1\);$/\1memset\(\2, 0, 0\);/g' $VIO_FILE

