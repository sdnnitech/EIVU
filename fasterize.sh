#!/bin/bash

# BATCH_SIZE = 1024
sed -i -e 's/^#define BATCH_SIZE_DEFAULT 32$/#define BATCH_SIZE_DEFAULT 1024/' ./lib/option.h

# VQ_SIZE = 2048
sed -i -e 's/^#define VQ_SIZE_DEFAULT 256$/#define VQ_SIZE_DEFAULT 2048/' ./lib/option.h

# MOBJ_CACHE_NUM = 2048
sed -i -e 's/^#define MOBJ_CACHE_NUM_DEFAULT 512$/#define MOBJ_CACHE_NUM_DEFAULT 2048/' ./lib/option.h

# MBUF_HEADROOM_SIZE = 0
sed -i -e 's/^#define MBUF_HEADROOM_SIZE 128$/#define MBUF_HEADROOM_SIZE 0/' ./lib/mbuf.h

# DATAROOM_SIZE = 64
sed -i -e 's/^#define DATAROOM_SIZE (MBUF_HEADROOM_SIZE + 2048)$/#define DATAROOM_SIZE (MBUF_HEADROOM_SIZE + 64)/' ./lib/mbuf.h

# METADATA_SIZE = 0
sed -i -e 's/^#define METADATA_SIZE 128$/#define METADATA_SIZE 0/' ./lib/mbuf.h

sed -i -e 's/.*reset_metadata(md);$//g' ./lib/mbuf.h

sed -i -e 's/.*md->.*//g' ./src/rx.c
sed -i -e 's/.*md->.*//g' ./lib/vhost.h
sed -i -e 's/^        vioqueue_enqueue_burst_tx(vq, mbp->mbuf_idx, mbp->md->pkt_len);$/        vioqueue_enqueue_burst_tx(vq, mbp->mbuf_idx, 64);/g' ./lib/vio.h
sed -i -e 's/.*md->.*//g' ./lib/vio.h

# Skip packet copy
sed -i -E 's/memcpy\((.*), (.*), .*\);$/memcpy\(\1, \2, 0\);/g' ./lib/vhost.h

# desc size = 8
sed -i -e 's/^    int32_t md_idx;$//' ./lib/vioqueue.h
sed -i -e 's/^    uint32_t len;$/    uint16_t len;/' ./lib/vioqueue.h
sed -i -e 's/^    int16_t id;$//' ./lib/vioqueue.h

# No vio_hdr
sed -i -e 's/^    vhost_enqueue_offload((struct vio_hdr *)desc_addr - 1, mbp->md);$/    memset((struct vio_hdr *)desc_addr, 0, 0);/' ./lib/vhost.h
sed -i -e 's/^        hdrs[i] = (struct vio_hdr *)desc_addrs[i] - 1;$/        hdrs[i] = (struct vio_hdr *)desc_addrs[i];/' ./lib/vhost.h
sed -i -E 's/^(.*)vhost_enqueue_offload\((.*), .*\);$/\1memset\(\2, 0, 0\);/g' ./lib/vhost.h
sed -i -e 's/^        vhost_dequeue_offload((struct vio_hdr *)desc_addr - 1);$/        memset((struct vio_hdr *)desc_addr, 0, 0);/' ./lib/vhost.h
sed -i -e 's/^            hdr = (struct vio_hdr *)desc_addrs[i] - 1;$/            hdr = (struct vio_hdr *)desc_addrs[i];/' ./lib/vhost.h
sed -i -E 's/^(.*)vhost_dequeue_offload\((.*)\);.*$/\1memset\(\2, 0, 0\);/g' ./lib/vhost.h

sed -i -E 's/^(.*)vio_rx_offload\((.*) - 1\);$/\1memset\(\2, 0, 0\);/g' ./lib/vio.h
sed -i -E 's/^(.*)vio_tx_clear_net_hdr\((.*) - 1\);$/\1memset\(\2, 0, 0\);/g' ./lib/vio.h

