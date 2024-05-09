#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

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
    struct vhost_queue vhq_tx;
    struct vioqueue vq_tx;
    uint16_t port_tx = 4;
    struct memobj_pool mpool_host, mpool_guest;
    void *memobjs_host = NULL;
    const size_t MEMOBJ_SIZE = METADATA_SIZE + DATAROOM_SIZE;
    struct mbuf_ptr mbptrs[MAX_BATCH_SIZE];

    opt = parse_opt(argc, argv);

    assert(opt.is_hugepage == false); // No impl for hugepage

    /* Init */
    shm_fd = shm_open(SHM_NAME, O_RDWR, FILE_MODE);

    shm.head = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm.head == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    init_shm(&shm, shm.head, BUF_NUM * MEMOBJ_SIZE, sizeof(struct desc) * opt.vq_size);

    memobjs_host = calloc(BUF_NUM, MEMOBJ_SIZE);
    if (memobjs_host == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    init_mpool(&mpool_host, memobjs_host, MEMOBJ_SIZE, BUF_NUM, opt.mobj_cache_num);
    init_mpool(&mpool_guest, memobjs(&shm), MEMOBJ_SIZE, BUF_NUM, opt.mobj_cache_num);
    init_vq(&vq_tx, opt.vq_size, txd(&shm), port_tx, &mpool_guest);
    vhq_tx.vq = &vq_tx;
    vhq_tx.host_mpool = &mpool_host;
    bind_core(2);

    initialized_shm_assert(shm_fd, &shm, opt.vq_size);

    /* I/O */
    uint32_t prev_pkt_id = 0;
    uint32_t pkt_counter = 0;
    bool is_poll = true;
    volatile bool *is_end = end_flag(&shm);
    volatile bool *is_start = start_flag(&shm);
    struct timespec start, end;

    *is_start = true;
    timespec_get(&start, TIME_UTC);
    while (is_poll) {
        uint16_t nb_rx = vhost_dequeue_burst(&vhq_tx, mbptrs, opt.batch_size);
        if (nb_rx == 0) {continue;}
        pkt_counter += nb_rx;

#ifdef DEBUG
        for (uint16_t i = 0; i < nb_rx; i++) {
            struct packet *pkt = (struct packet *)mbptrs[i].pkt;
            check_pkt(pkt, &prev_pkt_id);
        }
#endif

        for (uint16_t i = 0; i < nb_rx; i++) {
            mbuf_free(&mpool_host, &mbptrs[i]);
        }

       if (pkt_counter >= opt.pkt_num) {
            is_poll = false;
        }
    }
    timespec_get(&end, TIME_UTC);
    *is_end = true;

    double time_taken = end.tv_sec - start.tv_sec;
    long nsec_diff = end.tv_nsec - start.tv_nsec;
    if (nsec_diff < 0) {
        time_taken -= 1.0;
        nsec_diff += 1000000000;
    }
    time_taken += nsec_diff / 1000000000.0;
    printf("Time taken by program is : %f seconds (%.3f Mpps)\n", time_taken, (double)opt.pkt_num / time_taken / 1000000);

    /* Fin */
    fin_mpool(&mpool_host);
    free(memobjs_host);
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    return 0;
}
