#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../lib/shm.h"
#include "../lib/option.h"
#include "../lib/vio.h"
#include "../lib/pkt.h"

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
        d->buf_idx = memobj_get_stack(vq->mpool);
    }
    vq->vq_free_cnt = 0;
}

static void
init_descs_tx(struct vioqueue* vq)
{
    struct desc *d;
    uint16_t i = 0;
    for (i = 0; i < vq->nentries; i++) {
        d = &vq->descs[i];
        d->flags = USED_FLAG;
        d->buf_idx = -1;
    }
    vq->vq_free_cnt = 0;
}

int
main(int argc, char *argv[])
{
    struct vnwio_opt opt;
    int shm_fd;
    struct shm *shm;
    struct memobj_pool mpool;
    struct vioqueue vq_rx, vq_tx;
    uint16_t port_rx = 3, port_tx = 4;
    const size_t MEMOBJ_SIZE = METADATA_SIZE + DATAROOM_SIZE;
    struct mbuf_ptr mbptrs[MAX_BATCH_SIZE];

    opt = parse_opt(argc, argv);

    assert(opt.is_hugepage == false); // No impl for hugepage

    /* Init */
    shm_fd = create_shm(SHM_NAME, SHM_SIZE, FILE_MODE, opt.is_hugepage);
    if (opt.is_hugepage) {
        ;
    } else {
        shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    }

    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memset(shm, 0, sizeof(struct shm));
    if (init_mpool(&mpool, shm->memobjs, MEMOBJ_SIZE, BUF_NUM, MEMOBJ_CACHE_NUM) != 0) {
        fprintf(stderr, "init_mpool");
        exit(EXIT_FAILURE);
    }
    init_vq(&vq_rx, VQ_ENTRY_NUM, shm->desc_rx, port_rx, &mpool);
    init_descs_rx(&vq_rx);
    init_vq(&vq_tx, VQ_ENTRY_NUM, shm->desc_tx, port_tx, &mpool);
    init_descs_tx(&vq_tx);

    initialized_shm_assert(shm_fd, shm);
    assert(MEMOBJ_SIZE >= PKT_SIZE); // necessary
    assert(opt.batch_size <= VQ_ENTRY_NUM);
    assert((VQ_ENTRY_NUM & (VQ_ENTRY_NUM - 1)) == 0); // confirm if VQ_ENTRY_NUM is a power of two
    bind_core(1);

    /* I/O */
    uint32_t prev_pkt_id = 0;
    uint32_t pkt_counter = 0;
    bool is_poll = true;
    volatile bool *is_end = &shm->is_end;
    volatile bool *is_start = &shm->is_start;
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
    fin_mpool(&mpool);
    if (opt.is_hugepage) {
        ;
    } else {
        shm_unlink(SHM_NAME);
    }

    return 0;
}
