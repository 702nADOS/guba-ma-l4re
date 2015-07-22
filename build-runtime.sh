#!/bin/bash

cd trunk/src/l4
make B=../../obj/l4/x86
make O=../../obj/l4/x86 config
make O=../../obj/l4/x86
