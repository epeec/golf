#!/bin/bash
#SBATCH --partition debug
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=36
#SBATCH --job-name gaspi_coll
#SBATCH --time=00:30:00
#SBATCH --no-requeue

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/external/DIST/lib64
export PATH=$PATH:$PWD/external/DIST/bin

./gen_machine_file.sh

# Usage ./build/testcoll <length bytes> <num iterations> [check]
gaspi_run -m machinefile ./build/testcoll 20 10 &> results/coll_res.out

