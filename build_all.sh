#!/bin/bash

# required packages: libncurses5-dev make gawk g++ binutils pkg-config g++-multilib subversion qemu cmake libxml2-dev nodejs-legacy

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

cd pc_network_tool
npm install
cd ..

./build.sh kernel
./build.sh l4re
./build.sh mod
pkg/auto_build/make.sh

mkdir tms-sim-2014-12/build
cd tms-sim-2014-12/build
cmake ..
make
cd ..
build/bin/generator -o tasks.xml -n 5
cd ..
