#
# @file MultiDirectoryObj.mk
# @brief 这个文档的作用是：在Application.mk构建框架下，解决多目录OBJ无法正常清理的问题 
#

SUOBJS :=
ifneq ($(EXTRA),)
$(eval $(call SPLITVARIABLE,OBJS_SPILT,$(SORTOBJS),100))
$(foreach BATCH, $(OBJS_SPILT_TOTAL), \
	$(foreach obj, $(OBJS_SPILT_$(BATCH)), \
		$(foreach EXT, $(EXTRA), \
			$(eval substitute := $(patsubst %$(SUFFIX)$(OBJEXT),%$(SUFFIX)$(EXT),$(obj))) \
			$(eval SUOBJS += $(substitute)) \
		) \
	) \
)
endif

# 重写伪目标clean
clean::
	$(call DELFILE, .built)
	$(call CLEANAROBJS)
	$(call CLEAN)
	$(eval $(call SPLITVARIABLE,SUOBJS_SPILT,$(SUOBJS),100))
	$(foreach BATCH, $(SUOBJS_SPILT_TOTAL), \
		$(foreach obj, $(SUOBJS_SPILT_$(BATCH)), \
			$(shell rm -rf $(obj)) \
		) \
	)
