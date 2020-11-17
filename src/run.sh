#!/bin/bash

# run.sh

# --
# Make data

python pack-target.py data/jhu.mtx
python pack-query.py

# --

# mkdir bin
# make clean
# make