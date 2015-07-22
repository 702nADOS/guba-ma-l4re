PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../trunk/l4



TARGET                   	= test_Server test_Fahrassistent test_Motorregelung
SRC_CC_test_Server 		= server.cc
SRC_CC_test_Fahrassistent 	= Fclient.cc
SRC_CC_test_Motorregelung	= Mclient.cc

REQUIRES_LIBS           	= libedft cxx_libc_io cxx_io l4re_c-util  



include $(L4DIR)/mk/prog.mk
