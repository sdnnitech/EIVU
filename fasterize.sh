#!/bin/bash

OPTION_FILE=./lib/common/option.h
MBUF_FILE=./lib/mbuf/common/mbuf_core.h
VHOST_FILE=./lib/vnwio/vhost.h
VIO_FILE=./lib/vnwio/vio.h
VIO_RESET_MD_FILE=./lib/mbuf/guest_led/vio_reset_md.h
AGGREGATED_INTEGRATED_MD_FILE=./lib/mbuf/md_buf/integrated_md/aggregated_md.h
AGGREGATED_SEPARATED_MD_FILE=./lib/mbuf/md_buf/separated_md/individual_md/common/aggregated_md.h

git restore \
$OPTION_FILE \
$MBUF_FILE \
$VHOST_FILE \
$VIO_FILE \
$VIO_RESET_MD_FILE \
$AGGREGATED_INTEGRATED_MD_FILE \
$AGGREGATED_SEPARATED_MD_FILE \
./src/rx.c

# BATCH_SIZE = 1024
sed -i -e 's/^#define BATCH_SIZE_DEFAULT 32$/#define BATCH_SIZE_DEFAULT 1024/' $OPTION_FILE

# VQ_SIZE = 2048
sed -i -e 's/^#define VQ_SIZE_DEFAULT 256$/#define VQ_SIZE_DEFAULT 2048/' $OPTION_FILE

# MOBJ_CACHE_NUM = 2048
sed -i -e 's/^#define MOBJ_CACHE_NUM_DEFAULT 512$/#define MOBJ_CACHE_NUM_DEFAULT 2048/' $OPTION_FILE

# MBUF_HEADROOM_SIZE = 0
sed -i -e 's/^#define MBUF_HEADROOM_SIZE .*$/#define MBUF_HEADROOM_SIZE 0/' $MBUF_FILE

# MBUF_DATAROOM_SIZE = 64
sed -i -e 's/^#define MBUF_DATAROOM_SIZE .*$/#define MBUF_DATAROOM_SIZE 64/' $MBUF_FILE

# METADATA_SIZE
MD_SZ=0
if [ "$#" -ge 1 ]; then
    MD_SZ=$1
fi

if [ $MD_SZ -lt 64 ]; then
    sed -i -e 's/.*dpdk_prefetch.*//g' $AGGREGATED_INTEGRATED_MD_FILE
    sed -i -e 's/.*dpdk_prefetch.*//g' $AGGREGATED_SEPARATED_MD_FILE
fi

if [ $MD_SZ -eq 0 ]; then # METADATA_SIZE = 0
    sed -i -e 's/^#define METADATA_SIZE.*$/#define METADATA_SIZE 0/' $MBUF_FILE

    #sed -i -e 's/.*reset_metadata(md);$//g' $MBUF_FILE
    sed -i -e 's/^.*memset(md, 0, .*);$//g' $MBUF_FILE
    sed -i -e 's/^.*md->nb_segs = 1;$//g' $MBUF_FILE

    sed -i -e 's/.*md->.*//g' ./src/rx.c
    sed -i -e 's/.*md->.*//g' $VHOST_FILE
    
    sed -i -e 's/.*md->.*//g' $VIO_FILE
    sed -i -e 's/.*vio_reset_md_rx.*//g' $VIO_FILE
    sed -i -E 's/^(.*)vioqueue_set_len_tx.*$/\1d->len = 64;/g' $VIO_FILE

elif [ $MD_SZ -ge 4 ] && [ $MD_SZ -lt 8 ]; then
    sed -i -e 's/^.*md->nb_segs = 1;$//g' $MBUF_FILE
    sed -i -e 's/.*md->port.*//g' $VIO_RESET_MD_FILE
    sed -i -e 's/.*md->port.*//g' ./src/rx.c
fi

# Skip packet copy
sed -i -E 's/^(.*)dpdk_prefetch.*/\1memcpy(NULL, NULL, 0);/g' $VHOST_FILE
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

sed -i -E 's/^(.*)vio_rx_offload\((.*) - 1\);$/\1memset\(\2, 0, 0\);/g' $VIO_RESET_MD_FILE
sed -i -E 's/^(.*)vio_tx_clear_net_hdr\((.*) - 1\);$/\1memset\(\2, 0, 0\);/g' $VIO_FILE

