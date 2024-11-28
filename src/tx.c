#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <shm.h>
#include <option.h>
#include <mpools.h>
#include <vhost.h>
#include <pkt.h>

int
main(int argc, char *argv[])
{
    struct vnwio_opt opt;
    int shm_fd;
    struct shm shm;
    struct vhost_queue vhq_tx;
    struct vioqueue vq_tx;
    uint16_t port_tx = 4;
    struct mpools mpools_host, mpools_guest;
    struct mbuf_ptr mbptrs[MAX_BATCH_SIZE];

    opt = parse_opt(argc, argv);

    /* Init */

    if (opt.is_hugepage) {
        char hugepage_path[CACHE_LINE_SIZE] = HUGEPAGE_PATH;
        char *hugepage_shm_name = strcat(hugepage_path, SHM_NAME);
        shm_fd = open(hugepage_shm_name, O_RDWR, FILE_MODE);
        shm.head = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_POPULATE | MAP_HUGETLB, shm_fd, 0);
    } else {
        shm_fd = shm_open(SHM_NAME, O_RDWR, FILE_MODE);
        shm.head = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE,
            MAP_SHARED, shm_fd, 0);
    }
    if (shm.head == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    init_shm(&shm, shm.head, (size_t)BUF_NUM * (size_t)MDBUF_SIZE, (size_t)BUF_NUM * (size_t)MBUF_PKTBUF_SIZE, sizeof(struct desc) * opt.vq_size);

    init_mpools(&mpools_host, MDBUF_SIZE, MBUF_PKTBUF_SIZE, PKTBUF_NUM, MDBUF_NUM, opt.mobj_cache_num,
        shm.head + shm.end_offset + MBUF_DATAROOM_SIZE_DEFAULT * 16 + MDBUF_NUM * MDBUF_SIZE + PKTBUF_NUM * MBUF_PKTBUF_SIZE, NULL);
    init_mpools(&mpools_guest, MDBUF_SIZE, MBUF_PKTBUF_SIZE, PKTBUF_NUM, MDBUF_NUM, opt.mobj_cache_num,
        shm.head, &vq_tx);
    init_vq(&vq_tx, opt.vq_size, txd(&shm), port_tx, &mpools_guest);
    vhq_tx.vq = &vq_tx;
    vhq_tx.host_mpools = &mpools_host;
    bind_core(2);

    for (int i = 0; i < MAX_BATCH_SIZE; i++) {
        mbptrs[i].mbuf_idx.dmidx = init_midx();
    }

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
            mbuf_free(&mpools_host, mbptrs[i].mbuf_idx.dmidx);
        }
        free_aggregated_md_local(&mpools_host, mbptrs, nb_rx, 0);
        free_md_bulk(mbptrs, nb_rx);

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
    fin_mpools(&mpools_host, true);
    fin_mpools(&mpools_guest, true);
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    return 0;
}
