#!/bin/bash

USAGE="Usage: $0 --mdsz=<value>"

MD_SZ=""

for arg in "$@"; do
  case $arg in
    --mdsz=*)
      MD_SZ="${arg#*=}"
      ;;
    *)
      echo "Unknown option: $arg"
      echo $USAGE
      exit 1
      ;;
  esac
done

if [ -z "$MD_SZ" ]; then
  echo "Error: --mdsz option is required"
  echo $USAGE
  exit 1
fi

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
#sed -i -e 's/^#define BATCH_SIZE_DEFAULT 32$/#define BATCH_SIZE_DEFAULT 1024/' $OPTION_FILE

# VQ_SIZE = 2048
#sed -i -e 's/^#define VQ_SIZE_DEFAULT 256$/#define VQ_SIZE_DEFAULT 2048/' $OPTION_FILE

# MOBJ_CACHE_NUM = 2048
#sed -i -e 's/^#define MOBJ_CACHE_NUM_DEFAULT 512$/#define MOBJ_CACHE_NUM_DEFAULT 2048/' $OPTION_FILE

# Change impl dependent on METADATA_SIZE
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
    sed -i -e 's/.*avail_descs\[i\].len;//g' $VHOST_FILE
    sed -i -E 's/^(.*)lens\[i\](.*)$/\160\2/g' $VHOST_FILE
    sed -i -E 's/.*60 =.*//g' $VHOST_FILE
    sed -i -E 's/^(.*)memcpy\((.*), mps\[i\].md->pkt_len\);$/\1memcpy\(\2, 60\);/g' $VHOST_FILE
    
    sed -i -e 's/.*md->.*//g' $VIO_FILE
    sed -i -e 's/.*vio_reset_md_rx.*//g' $VIO_FILE
    sed -i -E 's/^(.*)vioqueue_set_len_tx.*$/\1d->len = 60;/g' $VIO_FILE

elif [ $MD_SZ -ge 2 ] && [ $MD_SZ -lt 8 ]; then
    sed -i -e 's/^.*md->nb_segs = 1;$//g' $MBUF_FILE
    sed -i -e 's/.*md->port.*//g' $VIO_RESET_MD_FILE
    sed -i -e 's/.*md->port.*//g' ./src/rx.c
fi

# Align Mbufs
if [ $(($MD_SZ % 64)) -ne 0 ]; then
	sed -i -E 's/^#define MBUF_DATAROOM_SIZE (.*)$/#define MBUF_DATAROOM_SIZE \(\1 + METADATA_SIZE % 64\)/' $MBUF_FILE
fi

# desc size = 8
# sed -i -e 's/^    int32_t md_idx;$//' ./lib/vioqueue.h
# sed -i -e 's/^    uint32_t len;$/    uint16_t len;/' ./lib/vioqueue.h
# sed -i -e 's/^    int16_t id;$//' ./lib/vioqueue.h
