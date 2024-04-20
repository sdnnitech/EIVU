#ifndef _PKT_H_
#define _PKT_H_

#include <stdint.h>

// #define MAX_BATCH_SIZE 1024
#define PKT_SIZE 64
#define VERIF_FLAG (0xffffffff)

struct packet {
    uint32_t id;
    uint32_t len;
    char dummy[PKT_SIZE - sizeof(uint32_t) * 3];
    uint32_t verif;
};

void
init_pkt(struct packet *pkt, uint32_t id)
{
    pkt->id = id;
    pkt->verif = id ^ VERIF_FLAG;
}

void
print_pkt(struct packet *pkt)
{
    printf("pkt->id=%u\n",pkt->id);

    printf("pkt->verif=%u\n", pkt->verif);
}

void
verif_pkt(struct packet *pkt)
{
    if ((pkt->id ^ pkt->verif) != VERIF_FLAG) {
        fprintf(stderr, "pkt %u: invalid packet\n", pkt->id);
    }
}

void
check_pkt(struct packet *pkt, struct packet *prev_pkt)
{
    // print_pkt(pkt);
    verif_pkt(pkt);
    if (prev_pkt == NULL) {
        return;
    }
    if (pkt->id != prev_pkt->id + 1) {
        fprintf(stderr, "pkt %u: out of order\n", pkt->id);
    }
}

#endif
