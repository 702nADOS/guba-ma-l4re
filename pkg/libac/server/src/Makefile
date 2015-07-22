PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../trunk/l4

TARGET		= acstand

SRC_CC		= main.cc global.cc\
		  sched_proxy.cc factory_proxy.cc strategy.cc
SRC_C 		= jsmn.c
LDFLAGS +=
#CPPFLAGS += -fPIC

REQUIRES_LIBS  := libkproxy cxx_libc_io cxx_io l4re_c-util

include $(L4DIR)/mk/prog.mk
