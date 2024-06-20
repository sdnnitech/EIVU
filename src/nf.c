#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <shm.h>
#include <option.h>
#include <mpools.h>
#include <vio.h>
#include <pkt.h>

static int
create_shm(const char *shm_name, const uint64_t shm_size, const int file_mode, bool is_hugepage)
{
    int shm_fd;
    
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, file_mode);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (!is_hugepage) {
        if (ftruncate(shm_fd, shm_size) == -1) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
    }

    return shm_fd;
}

static void
init_descs_rx(struct vioqueue* vq)
{
    struct desc *d;
    uint16_t i = 0;
    for (i = 0; i < vq->nentries; i++) {
        d = &vq->descs[i];
        d->flags = AVAIL_FLAG;
        set_desc_mbuf_idx(d, mbuf_alloc(vq->mpools));
    }
    vq->vq_free_cnt = 0;
}

static void
init_descs_tx(struct vioqueue* vq)
{
    struct desc *d;
    struct mbuf_idx midx = init_midx();
    uint16_t i = 0;
    for (i = 0; i < vq->nentries; i++) {
        d = &vq->descs[i];
        d->flags = USED_FLAG;
        set_desc_mbuf_idx(d, midx);
    }
    vq->vq_free_cnt = 0;
}

int
main(int argc, char *argv[])
{
    struct vnwio_opt opt;
    int shm_fd;
    struct shm shm;
    struct mpools mpools;
    struct vioqueue vq_rx, vq_tx;
    uint16_t port_rx = 3, port_tx = 4;
    const size_t MEMOBJ_SIZE = MDBUF_SIZE + MBUF_PKTBUF_SIZE;
    struct mbuf_ptr mbptrs[MAX_BATCH_SIZE];

    opt = parse_opt(argc, argv);

    assert(opt.is_hugepage == false); // No impl for hugepage

    /* Init */
    shm_fd = create_shm(SHM_NAME, SHM_SIZE, FILE_MODE, opt.is_hugepage);
    if (opt.is_hugepage) {
        ;
    } else {
        shm.head = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    }

    if (shm.head == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    init_shm(&shm, shm.head, BUF_NUM * MEMOBJ_SIZE, sizeof(struct desc) * opt.vq_size);

    memset(shm.head, 0,
        BUF_NUM * MEMOBJ_SIZE + 2 * sizeof(struct desc) * opt.vq_size + 2 * sizeof(bool));

    init_mpools(&mpools, MEMOBJ_SIZE, BUF_NUM, opt.mobj_cache_num, true, memobjs(&shm));
    init_vq(&vq_rx, opt.vq_size, rxd(&shm), port_rx, &mpools);
    init_descs_rx(&vq_rx);
    init_vq(&vq_tx, opt.vq_size, txd(&shm), port_tx, &mpools);
    init_descs_tx(&vq_tx);

    initialized_shm_assert(shm_fd, &shm, opt.vq_size);
    assert(MEMOBJ_SIZE >= PKT_SIZE); // necessary
    assert(opt.batch_size <= opt.vq_size);
    assert((opt.vq_size & (opt.vq_size - 1)) == 0); // confirm if opt.vq_size is a power of two
    bind_core(1);

    for (int i = 0; i < MAX_BATCH_SIZE; i++) {
        mbptrs[i].mbuf_idx = init_midx();
    }

    /* I/O */
    uint32_t prev_pkt_id = 0;
    uint32_t pkt_counter = 0;
    bool is_poll = true;
    volatile bool *is_end = end_flag(&shm);
    volatile bool *is_start = start_flag(&shm);
    *is_end = false;
    *is_start = false;
    while (is_poll) {
        uint16_t nb_rx = vio_recv_pkts(&vq_rx, mbptrs, opt.batch_size);
        if (nb_rx == 0) {continue;}
        pkt_counter += nb_rx;

#ifdef DEBUG
        for (uint16_t i = 0; i < nb_rx; i++) {
            struct packet *pkt = (struct packet *)mbptrs[i].pkt;
            check_pkt(pkt, &prev_pkt_id);
        }
#endif

        uint16_t nb_tx = vio_xmit_pkts(&vq_tx, mbptrs, nb_rx);
        while (nb_tx < nb_rx) {
            nb_tx += vio_xmit_pkts(&vq_tx, &mbptrs[nb_tx], nb_rx - nb_tx);
        }

        if (pkt_counter >= opt.pkt_num) {
            is_poll = false;
        }
    }
    while (!*is_end){}

    /* Fin. */
    fin_mpools(&mpools, true);
    if (opt.is_hugepage) {
        ;
    } else {
        shm_unlink(SHM_NAME);
    }

    return 0;
}
