#############################################
### BASH SCRIPT TO GENERATE MACHINE FILE  ###
### USING THE SLRUM ENVIRONMENT VARIABLES ###
#############################################

#SLURM_JOB_NODELIST=some
#SLURM_CPUS_ON_NODE=5
#SLURM_JOB_NUM_NODES=1

machinefile="machinefile"

echo > $machinefile
for NODE in $SLURM_JOB_NODELIST
do
  for (( i=1; i<= $SLURM_CPUS_ON_NODE; ++i ))
  do
    echo $NODE >> $machinefile
  done
done


