PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../../../trunk/l4

# the default is to build the listed directories, provided that they
# contain a Makefile. If you need to change this, uncomment the following
# line and adapt it.
TARGET =  $(PKGNAME)
INSTALLDIR_LOCAL = $(PKGDIR)
SRC_C = main.c
REQUIRES_LIBS  =
include $(L4DIR)/mk/prog.mk

all:: $(addprefix $(INSTALLDIR_LOCAL)/,$(INSTALL_TARGET)); @if [[ -n "$(INSTALL_TARGET)" ]]; then cp $(INSTALLDIR_LOCAL)/$(INSTALL_TARGET) $(PKGDIR)/ ; fi;
