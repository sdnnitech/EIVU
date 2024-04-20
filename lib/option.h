#ifndef _OPTION_H_
#define _OPTION_H_

#include <getopt.h>
#include <stdlib.h>

#define MAX_BATCH_SIZE 1024

struct vnwio_opt {
    bool is_hugepage;
    uint32_t pkt_num;
    uint32_t batch_size;
};

struct vnwio_opt
parse_opt(int argc, char *const argv[])
{
    int parser;
    int longidx;
    struct vnwio_opt vnwio_opt = {false, 100000000, 32};
    const struct option longopts[] = {
        {"help", no_argument, NULL, 'h'},
        {"hugepage", no_argument, NULL, 'H'},
        {"pktnum", required_argument, NULL, 'n'},
        {"batchsz", required_argument, NULL, 'b'},
        {0, 0, 0, 0},
    };
    
    int pktnum, batchsz;
    while ((parser = getopt_long(argc, argv, "hHn:b:", longopts, &longidx)) != -1) {
        switch (parser) {
            case 'h':
                printf("Usage:\n");
                exit(EXIT_SUCCESS);
            case 'H':
                vnwio_opt.is_hugepage = true;
                break;
            case 'n':
                pktnum = atoi(optarg);
                if (pktnum < 1) {
                    fprintf(stderr, "pktnum: invalid value\n");
                    exit(EXIT_FAILURE);
                }
                vnwio_opt.pkt_num = pktnum;
                break;
            case 'b':
                batchsz = atoi(optarg);
                if (batchsz < 1) {
                    fprintf(stderr, "batchsz: invalid value\n");
                    exit(EXIT_FAILURE);
                }
                vnwio_opt.batch_size = batchsz;
                break;
            default:
                fprintf(stderr, "Usage error\n");
                exit(EXIT_FAILURE);
        }
    }

    return vnwio_opt;
}


#endif /* _OPTION_H_ */
