
##############################################
############  DO NOT MODIFY #################
##############################################

INSTALL_PATH=$PWD/DIST
ROOT_DIR=$PWD
tar -zxvf GPI-2-1.3.1.tar.gz
cd GPI-2-1.3.1
./install.sh -p $INSTALL_PATH
cd ..
rm -rf GPI-2-1.3.1
