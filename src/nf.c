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
init_descs(struct vioqueue* vq)
{
    struct desc *d;
    uint16_t i = 0;
    for (i = 0; i < vq->nentries; i++) {
        d = &vq->descs[i];
        d->flags = AVAIL_FLAG;
        d->buf_idx = memobj_get_stack(vq->mpool);
    }
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
    if (init_mpool(&mpool, shm->memobjs, MEMOBJ_NUM, MEMOBJ_CACHE_NUM) != 0) {
        fprintf(stderr, "init_mpool");
        exit(EXIT_FAILURE);
    }
    init_vq(&vq_rx, VQ_ENTRY_NUM, shm->desc_rx, port_rx, &mpool);
    init_descs(&vq_rx);
    init_vq(&vq_tx, VQ_ENTRY_NUM, shm->desc_tx, port_tx, &mpool);
    init_descs(&vq_tx);

    initialized_shm_assert(shm_fd, shm, AVAIL_FLAG);
    // initialized_mpool_assert(&mpool);
    initialized_vq_assert(&vq_rx, VQ_ENTRY_NUM);
    initialized_vq_assert(&vq_tx, VQ_ENTRY_NUM);
    printf("sizeof(memobj)=%lu\n", sizeof(memobj));

    /* I/O */
    bool is_poll = true;
    while (is_poll) {
        uint16_t nb_rx = vio_recv_pkts(&vq_rx, mbptrs, opt.batch_size);
        if (nb_rx == 0) {continue;}

        for (uint16_t i = 0; i < nb_rx; i++) {
            /** DEBUG **/
            struct packet *pkt = (struct packet *)mbptrs[i].pkt;
            verif_pkt(pkt);
            printf("\n");
            /***********/

            if (pkt->id >= opt.pkt_num - 1) { // TODO: MUST make nf NOT access pkts
                is_poll = false;
                break;
            }
        }
    }

    /* Fin. */
    fin_mpool(&mpool);
    if (opt.is_hugepage) {
        ;
    } else {
        shm_unlink(SHM_NAME);
    }

    return 0;
}
