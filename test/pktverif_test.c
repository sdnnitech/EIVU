#include <stdio.h>

#include <pkt.h>

int main(void)
{
    struct packet pkt;
    uint32_t pkt_id = 2;

    printf("sizeof(struct packet)=%lu\n", sizeof(struct packet));
    init_pkt(&pkt, pkt_id);
    print_pkt(&pkt);
    verif_pkt(&pkt);
    return 0;
}
