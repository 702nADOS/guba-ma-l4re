PKGDIR  ?= ../..
L4DIR	?= $(PKGDIR)/../../trunk/l4

TARGET	 = libkproxy.so libkproxy.a
SRC_CC	 = factory_svr.cc


CXXFLAGS += -DL4_NO_RTTI -fno-rtti

include $(L4DIR)/mk/lib.mk
