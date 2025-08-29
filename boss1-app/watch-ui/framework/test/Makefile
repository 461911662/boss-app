############################################################################
# apps/watch-ui/framework/test/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

PROGNAME  = $(CONFIG_FRAMEWORK_TEST_PROGNAME)
PRIORITY  = $(CONFIG_FRAMEWORK_TEST_PRIORITY)
STACKSIZE = $(CONFIG_FRAMEWORK_TEST_STACKSIZE)
ifeq ($(CONFIG_FRAMEWORK_TEST_MODULE),y)
    MODULE := m
else
    MODULE := $(CONFIG_FRAMEWORK_TEST_MODULE)
endif

MAINSRC = test_framework.c

# Framework test code directory
FRAMEWORK_TEST_DIR ?= ${shell pwd}

ifeq ($(CONFIG_BT_TEST_ENABLED),y) 
include $(FRAMEWORK_TEST_DIR)/test_bluedroid/test_bluedroid.mk
endif

# Added testing's fixture
CFLAGS += -DUNITY_INCLUDE_CONFIG_H
CFLAGS += -DUNITY_INCLUDE_DOUBLE

CSRCS += $(FRAMEWORK_TEST_DIR)/common/unity_fixture.c

include $(APPDIR)/Application.mk

include $(APPDIR)/tools/MultiDirectoryObj.mk