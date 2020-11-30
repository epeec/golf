#############################################
### BASH SCRIPT TO GENERATE MACHINE FILE  ###
###         USING LOCAL HOST NAME         ###
#############################################

machinefile="machinefile"

echo > $machinefile
for ((i=0; i<$1; i++ ));
do
 hostname >> $machinefile
done

