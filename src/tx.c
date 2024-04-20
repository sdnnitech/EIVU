#include <sys/mman.h>
#include <fcntl.h>

#include "../lib/shm.h"

int
main(int argc, char *argv[])
{
    int shm_fd;
    struct shm *shm;
    /* Init */
    shm_fd = shm_open(SHM_NAME, O_RDWR, FILE_MODE);

    shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // mempool
    // vioqueue
    initialized_shm_assert(shm_fd, shm, );


    /* I/O */

}
