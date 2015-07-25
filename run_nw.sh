#!/bin/sh

# the top variables are same as in run.sh
# best case we should make a local.sh and 
# source it in all the files or have a better
# tooling around the build process

cd trunk/obj/l4/x86

LOCAL_PKG=../../pkg
MODULES_LIST=$LOCAL_PKG/conf/modules.list
MODULE_SEARCH_PATH=$LOCAL_PKG/conf:$LOCAL_PKG/conf/network:../obj/fiasco


QEMU_OPTIONS="-net socket,listen=:8010 -net nic,model=ne2k_pci,macaddr=52:54:00:00:00:01 -nographic" \
MAC="52:54:00:00:00:01" \
make qemu E=dom0_server_only MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH &

sleep 1

QEMU_OPTIONS="-net socket,connect=127.0.0.1:8010 -net nic,model=ne2k_pci,macaddr=52:54:00:00:00:02 -nographic" \
MAC="52:54:00:00:00:02" \
make qemu E=dom0_server_and_client MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH &
