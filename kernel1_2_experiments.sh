#!/usr/bin/env bash
#SBATCH --account="projet0041"
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

mkdir -p ~/runs/$USER/$SLURM_JOBID/
cp -r $SLURM_SUBMIT_DIR/ ~/runs/$USER/$SLURM_JOBID/
cd ~/runs/$USER/$SLURM_JOBID/pistolet-agrafes-500
spack load mpich
make -B


# Run `./main` with different OMP thread counts and save per-run logs
THREADS_LIST="1 2 4 8 16 32 64"
for t in $THREADS_LIST; do
	echo "=== Running with $t threads ===" | tee -a job.$SLURM_JOBID.out
	OMP_NUM_THREADS=$t srun -n1 -c $t ./main \
		> runs_${SLURM_JOBID}_threads_${t}.out 2> runs_${SLURM_JOBID}_threads_${t}.err
done