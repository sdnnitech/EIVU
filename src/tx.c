#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "../lib/shm.h"
#include "../lib/option.h"
#include "../lib/vhost.h"
#include "../lib/pkt.h"

int
main(int argc, char *argv[])
{
    struct vnwio_opt opt;
    int shm_fd;
    struct shm *shm;
    struct vhost_queue vhq_tx;
    struct vioqueue vq_tx;
    uint16_t port_tx = 4;
    struct memobj_pool mpool_host, mpool_guest;
    memobj *memobjs_host = NULL;
    struct mbuf_ptr mbptrs[MAX_BATCH_SIZE];

    opt = parse_opt(argc, argv);

    assert(opt.is_hugepage == false); // No impl for hugepage

    /* Init */
    shm_fd = shm_open(SHM_NAME, O_RDWR, FILE_MODE);

    shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memobjs_host = calloc(MEMOBJ_NUM, sizeof(memobj));
    if (memobjs_host == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    init_mpool(&mpool_host, memobjs_host, MEMOBJ_NUM, MEMOBJ_CACHE_NUM);
    init_mpool(&mpool_guest, shm->memobjs, MEMOBJ_NUM, MEMOBJ_CACHE_NUM);
    init_vq(&vq_tx, VQ_ENTRY_NUM, shm->desc_tx, port_tx, &mpool_guest);
    vhq_tx.vq = &vq_tx;
    vhq_tx.host_mpool = &mpool_host;

    // initialized_shm_assert(shm_fd, shm, AVAIL_FLAG);

    /* I/O */
    struct packet *prev_pkt = NULL;
    uint32_t pkt_counter = 0;
    bool is_poll = true;
    volatile bool *is_end = &shm->is_end;
    while (is_poll) {
        uint16_t nb_rx = vhost_dequeue_burst(&vhq_tx, mbptrs, opt.batch_size);
        if (nb_rx == 0) {continue;}
        pkt_counter += nb_rx;

        /** DEBUG **/
        for (uint16_t i = 0; i < nb_rx; i++) {
            struct packet *pkt = (struct packet *)mbptrs[i].pkt;
            check_pkt(pkt, prev_pkt);
            prev_pkt = pkt;
        }
        prev_pkt = NULL;
        /***********/

        for (uint16_t i = 0; i < nb_rx; i++) {
            mbuf_free(&mpool_host, &mbptrs[i]);
        }

       if (pkt_counter >= opt.pkt_num) {
            is_poll = false;
        }
    }
    *is_end = true;

    /* Fin */
    fin_mpool(&mpool_host);
    free(memobjs_host);
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    return 0;
}
