#!/usr/bin/env bash
#SBATCH --account="r250142"
#SBATCH --time=05:00:00
#SBATCH --mem=128G
#SBATCH --constraint=x64cpu
#SBATCH --nodes=1
#SBATCH --cpus-per-task=64
#SBATCH --job-name "Graph500_k1_k2"
#SBATCH --comment "Benchmark nos algos de graph500"
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out

romeo_load_x64cpu_env

spack load mpich tbb

TBB_DIR=$(spack location -i tbb 2>/dev/null || spack location -i intel-tbb 2>/dev/null)
export CPPFLAGS="-I$TBB_DIR/include"
export CXXFLAGS="-I$TBB_DIR/include"
export LDFLAGS="-L$TBB_DIR/lib -ltbb"
make -B -j$(nproc) CPPFLAGS="$CPPFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS"

# Run `./main` with different OMP thread counts and save per-run logs
THREADS_LIST="1 2 4 8 16 32 64"
for t in $THREADS_LIST; do
        echo "=== Running with $t threads ===" | tee -a job.$SLURM_JOBID.out
        OMP_NUM_THREADS=$t srun -n1 -c $t ./main
done