#!/bin/bash
make
tc qdisc add dev eth0 root netem loss gemodel 1% 10% 70% 0.1%
rm Proton-5.21-GE-1.tar.gz
./lab1_file_transer udp recv 0.0.0.0 10001
sha1sum Proton-5.21-GE-1.tar.gz
tc qdisc del dev eth0 root

