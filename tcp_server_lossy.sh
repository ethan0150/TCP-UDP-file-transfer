#!/bin/bash
tc qdisc add dev eth0 root netem loss 10%
make
rm Proton-5.21-GE-1.tar.gz
./lab1_file_transer tcp recv 0.0.0.0 10001
sha1sum Proton-5.21-GE-1.tar.gz
tc qdisc del dev eth0 root

