# EIVU v2 (Essential Implementation of Vhost-User ver. 2)
(EIVU v1: https://github.com/sdnnitech/EIVUv1)

***
## Introduction

EIVU is an easy-to-customize evaluation platform for deeply analyzing high-performance interprocess communications. 
Specifically, an efficiency of packet forwarding throughout multiple processes is evaluated as packet-per-second throughput.
The core design and implementation of EIVU are equivalent to that of vhost-user with Data Plane Development Kit (DPDK).
EIVU shows comparable performance characteristics with vhost-user/DPDK in that the throughput and the CPU cache usage; however, 
it provides far easier analyzability and customizability to the users.

EIVU consists of three programs (processes), receiver (Rx), network function (NF), and transmitter (Tx).
The Rx process generates the specified number of packets, and sends them to the NF process.
The NF process simply forwards them to the Tx process by default, but the users can add arbitrary packet processing.
The Tx process copies the packets from the NF process, and consumes them (do not transfer them to another).

Note that the EIVU platform does not require any NIC, and therfore, the users can analyze vhost-user/DPDK on a standalone machine.

***
## How to use

### Requirements

- Linux-based commodity server (The CPU should have 4 cores or more)
- 1 GB hugepage (optional)
- gcc compilers
- Meson
- Ninja
- perf (optional)

---
### Build

You can use Meson & Ninja to build EIVU.
Execute the following commands, and then,
three binaries are generated: "rx.out", "nf.out", and "tx.out".

`meson setup <builddir> [options]`

`ninja -C <builddir>`

Example:
```
$ meson setup output/bin/base/coupled/128
$ ninja -C output/bin/base/coupled/128
```


#### Options
- Fast type†

†D. Takeya et al., ”Understanding Roadblocks in Virtual Network I/O: A Comprehensive Analysis of CPU Cache Usage”, Proc. IEEE NetSoft 2023.

Example:
```
$ meson setup output/bin/fast/coupled/0 -Dmetadata_size=0 -Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=64 -Dzero_copy=rx,tx
```
</br></br>

- Resizing of the metadata area (STEP-I)

`meson setup <builddir> -Dmetadata_size=<val: 2-256> ...`

Example:
````
$ meson setup output/bin/fast/coupled/16 -Dmetadata_size=16 -Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=64 -Dzero_copy=rx,tx
````
</br></br>

- Decoupling and Aggregating of the metadata area (STEP-II)

`meson setup <builddir>  -Daggregated_md=true  -Dhost_aggregated_md=true -Daggregation_num=<val:1 - batch length>`

Example:
```
$ meson setup output/bin/fast/decoupled/aggr-512/16 -Dmetadata_size=16 -Dvio_header=false -Dtiny_descs=true -Dheadroom_size=0 -Ddataroom_size=64 -Dzero_copy=rx,tx -Daggregated_md=true -Dhost_aggregated_md=true -Daggregation_num=512
```
</br></br>

---
### Run
#### Measuring throughput

The "run.sh" script file internally launches the aforementioned three programs ("rx.out", "nf.out", and "tx.out"), and 
automatically starts a measurement of packet forwarding efficiency (throughput).

`./run.sh <builddir> <batchsz> <vqsz>`

Example:
```
$ ./run.sh output/bin/base/coupled/128 32 256 | tee output/eval/base/coupled/batch-32/result-128.txt
```
  
  
#### Measuring cache usage

The "perfrun.sh" script file outputs the profiling results for each program.

<ins>The "opt_perf" variable in the script file must be set before the execution.</ins>

`./perfrun.sh <builddir> <batchsz> <vqsz> <output_file>`

Example: 
```
$ ./perfrun.sh output/bin/fast/decoupled/aggr-512/16 512 1024 output/eval/fast/decoupled/batch-512/aggr-512/perf-16.txt
```


***
## Evaluation
Result: https://sdnnitech.github.io/EIVU/eval/index.html


***
## Academic Results
### Papers
- A. Yamada, R. Kawashima, H. Nakayama, T. Hayashi, and H. Matsuo, "Rethinking Message Buffer Structures for 100 Mpps Cloud-native Network Functions", Proc. 2024 IEEE Future Networks World Forum (FNWF), 2024.
- A. Yamada, R. Kawashima, H. Nakayama, T. Hayashi, and H. Matsuo, "Cache Optimization for Real CNF through Message Buffer Redesign", IEICE Technical Report, vol. 124, no. 179, NS2024-76, pp. 6-11, 2024. (in Japanese)
### Awards
- A. Yamada, R. Kawashima, H. Nakayama, T. Hayashi, and H. Matsuo, "IEEE FNWF 2024 Best Paper Award", IEEE Future Networks, 2024.
- A. Yamada, "Young Researcher Award", IEICE Tech. Committee on NS, 2024.
