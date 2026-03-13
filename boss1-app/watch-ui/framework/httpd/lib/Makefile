############################################################################
# vendor/boss/app/boss1-app/watch-ui/framework/httpd/lib/Makefile
#
# HTTPD library sources - thttpd library and HTTP server API
#
############################################################################

include $(APPDIR)/Make.defs

CSRCS = libhttpd.c
CSRCS += thttpd_cgi.c
CSRCS += thttpd_alloc.c
CSRCS += thttpd_strings.c
CSRCS += timers.c
CSRCS += fdwatch.c
CSRCS += tdate_parse.c
CSRCS += thttpd.c
CSRCS += httpd_server.c

# Content source files
CONTENT_DIR := $(APPDIR)/watch-ui/framework/httpd/content
CONTENT_MAKE := $(MAKE) -C $(CONTENT_DIR)

VPATH += :$(CONTENT_DIR)
DEPPATH += --dep-path $(CONTENT_DIR)

CSRCS += romfs.c


$(CONTENT_DIR)/romfs.c: build

build::
	+$(Q) $(CONTENT_MAKE) TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

# Common build

context::
	+$(Q) $(CONTENT_MAKE) context TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

clean::
	+$(Q) $(CONTENT_MAKE) clean TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

distclean::
	+$(Q) $(CONTENT_MAKE) distclean TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)


include $(APPDIR)/Application.mk
