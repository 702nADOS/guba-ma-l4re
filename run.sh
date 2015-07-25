#!/bin/bash
# Shell script that facilitates running OC in qemu mode with modules preselected
# NOTES : 
# ./run.sh              - will start acstand module by default
# ./run.sh <moduleName> - will start the <moduleName>
# ./run.sh c            - will give the option to select the module
# ./run.sh n            - will start the network version, stark thesis

cd trunk/obj/l4/x86

LOCAL_PKG=../../pkg
MODULES_LIST=$LOCAL_PKG/conf/modules.list
MODULE_SEARCH_PATH=$LOCAL_PKG/conf:$LOCAL_PKG/conf/network:../obj/fiasco
E="E=acstand"

if [ -n "$1" ]; then
  if [ $1 == "c" ]; then
    E=""
  elif [ $1 == "n" ]; then
    ./run_nw.sh 
  else
    E="E=$1"
  fi
fi

#run the command with the right options
make qemu $E QEMU_OPTIONS=-nographic QEMU_OPTIONS+=-serial\ stdio QEMU_OPTIONS+="-net socket,listen=:8010 -net nic,model=ne2k_pci,macaddr=52:54:00:00:00:02" MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH;


#if [ -z "$1" ]
#	then
#		make qemu E=acstand QEMU_OPTIONS=-nographic QEMU_OPTIONS=-serial\ stdio MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH;
#	else
#		make qemu E=$1 QEMU_OPTIONS=-nographic QEMU_OPTIONS=-serial\ stdio MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH;
#fi
#
