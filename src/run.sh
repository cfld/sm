#!/bin/bash

# run.sh

python pack-query.py
for N in 1000 1500 2000 2500 3000 3500 4000 4500 5000; do
  python pack-target.py $N
  OMP_NUM_THREADS=40 ./bin/vf2
done
