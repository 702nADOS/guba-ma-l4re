#!/bin/bash

# List of packages we have created or modified
#dom0-main  l4re_kernel  l4sys  libac  libedft  libkproxy  moe  ned

cd pkg

#stark changes
#Mr. stark had changes in l4sys as well, thus building dom0-main first
make O=../../trunk/obj/l4/x86 -C pkg/dom0-main
make O=../../trunk/obj/l4/x86 -C pkg/ned

#hauner changes
make O=../../trunk/obj/l4/x86 -C pkg/l4sys
make O=../../trunk/obj/l4/x86 -C pkg/libedft

#bachmeier changes
make O=../../trunk/obj/l4/x86 -C pkg/libkproxy
make O=../../trunk/obj/l4/x86 -C pkg/libac
make O=../../trunk/obj/l4/x86 -C pkg/moe
make O=../../trunk/obj/l4/x86 -C pkg/l4re_kernel

#kundaliya changes
