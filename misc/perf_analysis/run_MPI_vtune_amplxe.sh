#!/bin/bash
#SBATCH -J amplxe
#SBATCH -N 1
#SBATCH -p DevQ
#SBATCH -t 01:00:00
#SBATCH -A "ichec001"
#no extra settings

START_TIME=`date '+%Y-%b-%d_%H.%M.%S'`

PROFILING_RESULTS_PATH=PROFILING_RESULTS
VTUNE_RESULTS_PATH=${PROFILING_RESULTS_PATH}/VTUNE_RESULTS
EXPERIMENT_RESULTS_DIR=VTUNE_RESULTS_${START_TIME}_job${SLURM_JOBID}

################################################
### NUMA Bindings
################################################

NUMA_CPU_BIND="0"
NUMA_MEM_BIND="0"

NUMA_CTL_CMD_ARGS="numactl --cpubind=${NUMA_CPU_BIND} --membind=${NUMA_MEM_BIND}"

#################################################
### MPI job configuration.
###
### Note: User may need to modify.
#################################################

NNODES=1
NTASKSPERNODE=16
NTHREADS=1
NPROCS=$(( NTASKSPERNODE*NNODES ))

export OMP_NUM_THREADS=${NTHREADS}
export AFF_THREAD=${NTHREADS}
export KMP_AFFINITY=compact

#################################################
### Application configuration.
###
### Note: User may need to modify.
#################################################

PATH_TO_EXECUTABLE=${QNLP_ROOT}/build/demos/hamming_RotY
EXECUTABLE=exe_demo_hamming_RotY

EXE_VERBOSE=0
EXE_TEST_PATTERN=0
EXE_NUM_EXP=2
EXE_LEN_PATTERNS=12

EXECUTABLE_ARGS="${EXE_VERBOSE} ${EXE_TEST_PATTERN} ${EXE_NUM_EXP} ${EXE_LEN_PATTERNS}"


#################################################
### Load relevant module files.
#################################################
module load  gcc/8.2.0 intel/2019u5

# Increases performance
export I_MPI_TUNING_BIN=${I_MPI_ROOT}/intel64/etc/tuning_skx_shm-ofi_efa.dat

#################################################
### Set-up Command line variables and Environment
### for VTUNE.
###
### Note: User may need to modify.
#################################################

source ${VTUNE_AMPLIFIER_XE_2019_DIR}/amplxe-vars.sh

VTUNE_ANALYSIS_TYPE=hpc-performance

VTUNE_ARGS="-knob sampling-interval=5 -knob stack-size=0 -knob collect-memory-bandwidth=true"
            #-knob  sampling-interval=5                 \
            #-knob enable-stack-collection=-true        \
            #-knob stack-size=0                         \ 
            #-knob collect-memory-bandwidth=true        \
            #-knob collect-affinity=true                \

#################################################
### Set-up directory for VTUNE results.
#################################################

[ ! -d "${PROFILING_RESULTS_PATH}" ] && mkdir -p "${PROFILING_RESULTS_PATH}"
[ ! -d "${VTUNE_RESULTS_PATH}" ] && mkdir -p "${VTUNE_RESULTS_PATH}"
[ ! -d "${VTUNE_RESULTS_PATH}/${EXPERIMENT_RESULTS_DIR}" ] && mkdir -p "${VTUNE_RESULTS_PATH}/${EXPERIMENT_RESULTS_DIR}"

#################################################
### Run application using VTUNE 
### (also collects timing metrics).
#################################################

start_time=`date +%s`

# Collect HPC-Performance Metrics
srun --ntasks ${NPROCS} --ntasks-per-node ${NTASKSPERNODE} -l ${NUMA_CTL_CMD_ARGS} amplxe-cl -collect ${VTUNE_ANALYSIS_TYPE} ${VTUNE_ARGS}  ${SYMBOL_FILES_SEARCH_COMMAND} ${SOURCE_FILES_SEARCH_COMMAND} -result-dir ${VTUNE_RESULTS_PATH}/${EXPERIMENT_RESULTS_DIR}/profile -- ${PATH_TO_EXECUTABLE}/${EXECUTABLE} ${EXECUTABLE_ARGS}
echo "Completeted hpc-performance"


# Collect Threading Metrics
#srun --ntasks ${NPROCS} --ntasks-per-node ${NTASKSPERNODE} amplxe-cl -collect threading -r ${VTUNE_RESULTS_PATH}/${EXPERIMENT_RESULTS_DIR}/profile -- ${PATH_TO_EXECUTABLE}/${EXECUTABLE} ${EXECUTABLE_ARGS}
echo "Completed threading"

# Collect Threading with Hardware counters Metrics
#srun --ntasks ${NPROCS} --ntasks-per-node ${NTASKSPERNODE} amplxe-cl -collect threading -knob sampling-and-waits=hw -r ${VTUNE_RESULTS_PATH}/${EXPERIMENT_RESULTS_DIR}/profile -- ${PATH_TO_EXECUTABLE}/${EXECUTABLE} ${EXECUTABLE_ARGS}
echo "Completed threading with Hardware"

end_time=`date +%s`
runtime=$((end_time-start_time))

echo "Execution time: ${runtime}"

#################################################
### Copy slurm output to log file location
#################################################

cp slurm-${SLURM_JOBID}.out ${VTUNE_RESULTS_PATH}/${EXPERIMENT_RESULTS_DIR}/slurm-${SLURM_JOBID}.out
