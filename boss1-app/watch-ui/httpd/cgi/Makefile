############################################################################
# vendor/boss/app/boss1-app/watch-ui/httpd/cgi/Makefile
#
# CGI programs for captive portal - runs as BINFS application
#
############################################################################

include $(APPDIR)/Make.defs

MAINSRC = cgi_main.c

CSRCS += auth.c
CSRCS += session.c
CSRCS += utils.c
CSRCS += wifi_status.c

PROGNAME = cgi
PRIORITY = $(CONFIG_MY_HTTPD_CGI_PRIORITY)
STACKSIZE = $(CONFIG_MY_HTTPD_CGI_STACKSIZE)

VPATH += :cgi-src
VPATH += :cgi-src/common
VPATH += :cgi-src/login
VPATH += :cgi-src/status

DEPPATH += --dep-path cgi-src
DEPPATH += --dep-path cgi-src/login
DEPPATH += --dep-path cgi-src/common
DEPPATH += --dep-path cgi-src/status

include $(APPDIR)/Application.mk
