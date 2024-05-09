#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "../lib/shm.h"
#include "../lib/option.h"
#include "../lib/vhost.h"
#include "../lib/pkt.h"

int
main(int argc, char *argv[])
{
    struct vnwio_opt opt;
    int shm_fd;
    struct shm shm;
    struct vioqueue vq_rx;
    uint16_t port_rx = 3;
    struct memobj_pool mpool_host, mpool_guest;
    void *memobjs_host = NULL;
    const size_t MEMOBJ_SIZE = METADATA_SIZE + DATAROOM_SIZE;

    opt = parse_opt(argc, argv);

    assert(opt.is_hugepage == false); // No impl for hugepage

    /* Init */
    shm_fd = shm_open(SHM_NAME, O_RDWR, FILE_MODE);

    shm.head = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm.head == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    init_shm(&shm, shm.head, BUF_NUM * MEMOBJ_SIZE, sizeof(struct desc) * VQ_ENTRY_NUM);

    memobjs_host = calloc(BUF_NUM, MEMOBJ_SIZE);
    if (memobjs_host == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    init_mpool(&mpool_host, memobjs_host, MEMOBJ_SIZE, BUF_NUM, MEMOBJ_CACHE_NUM);
    init_mpool(&mpool_guest, memobjs(&shm), MEMOBJ_SIZE, BUF_NUM, MEMOBJ_CACHE_NUM);
    init_vq(&vq_rx, VQ_ENTRY_NUM, rxd(&shm), port_rx, &mpool_guest);
    bind_core(0);

    initialized_shm_assert(shm_fd, &shm);

    /* I/O */
    volatile bool *is_start = start_flag(&shm);
    struct mbuf_ptr mbptrs[MAX_BATCH_SIZE];
    struct mbuf_ptr *mbp;
    struct packet *pkt;
    uint16_t nb_tx = 0;
    uint16_t nb_rx = 0;
    uint32_t pkt_id;
    uint32_t pkt_counter = 0;

    while (!*is_start) {}
    for (pkt_counter = 0; pkt_counter < opt.pkt_num; pkt_counter += nb_tx) {
        for (nb_rx = 0; nb_rx < opt.batch_size; nb_rx++) {
            pkt_id = pkt_counter + nb_rx + 1;
            if (pkt_id > opt.pkt_num) {break;}

            mbp = &mbptrs[nb_rx];
            reset_mbptr(mbp, mbuf_alloc(&mpool_host), &mpool_host);
            mbp->md->pkt_len = PKT_SIZE;
            mbp->md->port = port_rx;

#ifdef DEBUG
            pkt = (struct packet *)mbp->pkt;
            init_pkt(pkt, pkt_id);
#endif
        }
        nb_tx = vhost_enqueue_burst(&vq_rx, mbptrs, nb_rx);

        // if (nb_tx < nb_rx) {
        //     nb_tx += vhost_enqueue_burst(&vq_rx, &mbptrs[nb_tx], nb_rx - nb_tx);
        // }

        for (uint32_t k = 0; k < nb_rx; k++) {
            mbuf_free(&mpool_host, &mbptrs[k]);
        }
    }

    /* Fin */
    fin_mpool(&mpool_host);
    free(memobjs_host);
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    return 0;
}
