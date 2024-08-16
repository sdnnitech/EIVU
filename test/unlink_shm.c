#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

#include <shm.h>

int main(void)
{
    if (shm_unlink(SHM_NAME)) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
    return 0;
}
