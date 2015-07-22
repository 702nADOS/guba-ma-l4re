#!/bin/bash

cd trunk/kernel/fiasco
make BUILDDIR=../../obj/fiasco
cd ../../obj/fiasco
make config
make
