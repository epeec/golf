
CC=gcc
CXX=g++

##############################################
############  DO NOT MODIFY #################
##############################################

INSTALL_PATH=$PWD/DIST
ROOT_DIR=$PWD
tar -jxvf GPI-2-git.tar.bz2
cd GPI-2
./autogen.sh
CC=$CC CXX=$CXX ./configure --prefix=$INSTALL_PATH
cd ..
rm -rf GPI-2
