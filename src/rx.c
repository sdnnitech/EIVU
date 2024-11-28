#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

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
    struct vioqueue vq_rx;
    uint16_t port_rx = 3;
    struct mpools mpools_host, mpools_guest;

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
        shm.head + shm.end_offset + MBUF_DATAROOM_SIZE_DEFAULT * 8, NULL);
    init_mpools(&mpools_guest, MDBUF_SIZE, MBUF_PKTBUF_SIZE, PKTBUF_NUM, MDBUF_NUM, opt.mobj_cache_num,
        shm.head, &vq_rx);
    init_vq(&vq_rx, opt.vq_size, rxd(&shm), port_rx, &mpools_guest);
    bind_core(0);

    initialized_shm_assert(shm_fd, &shm, opt.vq_size);

    /* I/O */
    volatile bool *is_start = start_flag(&shm);
    struct mbuf_ptr mbptrs[MAX_BATCH_SIZE];
    struct mbuf_ptr *mbp;
    struct packet *pkt;
    uint16_t nb_tx = 0;
    uint16_t nb_rx = 0;
    uint32_t pkt_id;
    uint32_t pkt_counter = 0;

    for (int i = 0; i < MAX_BATCH_SIZE; i++) {
        mbptrs[i].mbuf_idx.dmidx = init_midx();
    }

    while (!*is_start) {}
    for (pkt_counter = 0; pkt_counter < opt.pkt_num; pkt_counter += nb_tx) {
        for (nb_rx = 0; nb_rx < opt.batch_size; nb_rx++) {
            pkt_id = pkt_counter + nb_rx + 1;
            if (pkt_id > opt.pkt_num) {break;}

            mbp = &mbptrs[nb_rx];
            reset_mbptr(mbp, mbuf_alloc(&mpools_host), &mpools_host);

#ifdef DEBUG
            pkt = (struct packet *)mbp->pkt;
            init_pkt(pkt, pkt_id);
#endif
        }

        alloc_aggregated_md_local(&mpools_host, mbptrs, nb_rx);
        for (uint16_t i = 0; i < nb_rx; i++) {
            mbp = &mbptrs[i];
            mbp->md->pkt_len = PKT_SIZE;
            mbp->md->port = port_rx;
        }

        nb_tx = vhost_enqueue_burst(&vq_rx, mbptrs, nb_rx);

        if (nb_tx < nb_rx) {
            nb_tx += vhost_enqueue_burst(&vq_rx, &mbptrs[nb_tx], nb_rx - nb_tx);
        }

        for (uint32_t k = 0; k < nb_rx; k++) {
            mbuf_free(&mpools_host, mbptrs[k].mbuf_idx.dmidx);
        }
        free_aggregated_md_local(&mpools_host, mbptrs, nb_rx, 0);
        free_md_bulk(mbptrs, nb_rx);
    }

    /* Fin */
    fin_mpools(&mpools_host, true);
    fin_mpools(&mpools_guest, true);
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    return 0;
}
