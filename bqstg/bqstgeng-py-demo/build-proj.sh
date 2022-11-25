#!/bin/bash
set -u
set -e

readonly SOLUTION_ROOT_DIR=/mnt/storage/work/betterquant

black stgeng-10000.py
cp stgeng-10000.py $SOLUTION_ROOT_DIR/bin/
