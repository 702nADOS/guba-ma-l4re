#!/bin/bash

# the top variables are same as in run.sh
# best case we should make a local.sh and 
# source it in all the files or have a better
# tooling around the build process

## Helper Functions 
##
print_help_and_exit() {
  echo "Usage: "
  echo "./run_nw.sh a/b"
  echo "a - dom0_server_only"
  echo "b - dom0_server_and_client"
  exit 0;
}


## If no arguments
##
if [ -z "$1" ]; then
  print_help_and_exit
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR/trunk/obj/l4/x86

LOCAL_PKG=$SCRIPT_DIR/pkg
MODULES_LIST=$LOCAL_PKG/conf/modules.list
MODULE_SEARCH_PATH=$LOCAL_PKG/conf:$LOCAL_PKG/conf/network:$SCRIPT_DIR/trunk/obj/fiasco

sudo ifconfig tap0 down;
sudo ifconfig tap0 up;

if [ $1 == "a" ]; then
  #QEMU_OPTIONS="-net socket,listen=:8010 -net nic,model=ne2k_pci,macaddr=52:54:00:00:00:01 -nographic -monitor stdio" \
  QEMU_OPTIONS="-net vde,sock=/tmp/switch1" \
  QEMU_OPTIONS+=" -nographic" \
  MAC="52:54:00:00:00:01" \
  make qemu E=dom0_server_only MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH &
elif [ $1 == "b" ]; then 
  #QEMU_OPTIONS="-net socket,connect=127.0.0.1:8010 -net nic,model=ne2k_pci,macaddr=52:54:00:00:00:02 -nographic" \
  QEMU_OPTIONS="-net vde,sock=/tmp/switch1" \
  QEMU_OPTIONS+=" -nographic" \
  MAC="52:54:00:00:00:02" \
  make qemu E=dom0_server_and_client MODULES_LIST=$MODULES_LIST MODULE_SEARCH_PATH=$MODULE_SEARCH_PATH &
fi
