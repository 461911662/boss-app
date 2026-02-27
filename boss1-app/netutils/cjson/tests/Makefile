#############################################################################
# apps/netutils/cjson/tests/Makefile
#
# cJSON Test Makefile - builds and runs cJSON tests
#
#############################################################################

include $(APPDIR)/Make.defs

# Test source files
CSRCS = test.c
CSRCS += parse_tests.c
CSRCS += print_tests.c
CSRCS += misc_tests.c

# cJSON source
CJSON_SRCDIR = $(APPDIR)/netutils/cjson/cJSON

# Include paths
CFLAGS += -I$(CJSON_SRCDIR)
CFLAGS += -I$(APPDIR)/netutils/cjson/tests

PROGNAME = cjson_test
PRIORITY = 100
STACKSIZE = 4096

include $(APPDIR)/Application.mk
