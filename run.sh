#!/bin/bash

cd trunk/obj/l4/x86

LOCAL_PKG=../../pkg
MODULES_LIST=$LOCAL_PKG/conf/modules.list
MODULE_SEARCH_PATH=$LOCAL_PKG/conf:$LOCAL_PKG/conf/networking:../obj/fiasco
E=acstand

if [ -n "$1" ]
  then
  E=$1
fi

#run the command with the right options
make qemu E=$E QEMU_OPTIONS=-nographic QEMU_OPTIONS=-serial\ stdio MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH;


#if [ -z "$1" ]
#	then
#		make qemu E=acstand QEMU_OPTIONS=-nographic QEMU_OPTIONS=-serial\ stdio MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH;
#	else
#		make qemu E=$1 QEMU_OPTIONS=-nographic QEMU_OPTIONS=-serial\ stdio MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH;
#fi
#
