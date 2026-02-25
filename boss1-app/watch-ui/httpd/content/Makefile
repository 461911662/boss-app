############################################################################
# vendor/boss/app/boss1-app/watch-ui/httpd/content/Makefile
#
# Content build - HTML files and ROMFS generation
#
############################################################################

include $(APPDIR)/Make.defs

CONTENT_DIR := $(APPDIR)/watch-ui/httpd/content
HTML_DIR    := $(CONTENT_DIR)/html
ROMFS_DIR   := $(CONTENT_DIR)/romfs
ROMFS_IMG   := $(CONTENT_DIR)/romfs.img
ROMFS_SRC   := $(CONTENT_DIR)/romfs.c
ROMFS_FILES := $(wildcard $(HTML_DIR)/*)
ROMFS_TARGETS := $(patsubst $(HTML_DIR)/%,$(ROMFS_DIR)/%,$(ROMFS_FILES))

CSRCS = $(ROMFS_SRC)

$(ROMFS_SRC): $(ROMFS_IMG)
	$(Q) (cd $(CONTENT_DIR) && echo "#include <nuttx/compiler.h>" >$@ && \
	      xxd -i romfs.img | sed -e "s/^unsigned char/const unsigned char aligned_data(4)/g" >>$@)

$(ROMFS_DIR)/%: $(HTML_DIR)/%
	$(Q) mkdir -p $(ROMFS_DIR)
	$(Q) cp -a $< $@

$(ROMFS_IMG): $(ROMFS_TARGETS)
	$(Q) genromfs -f $@.tmp -d $(ROMFS_DIR) -V "HTTPDCONTENT"
	$(Q) $(call TESTANDREPLACEFILE, $@.tmp, $@)

all: $(ROMFS_SRC)

context:
	@

clean:
distclean:
	$(call DELFILE, $(ROMFS_SRC))
	$(call DELFILE, $(ROMFS_IMG))
	$(Q) rm -rf $(ROMFS_DIR)

.PHONY: all context clean distclean
