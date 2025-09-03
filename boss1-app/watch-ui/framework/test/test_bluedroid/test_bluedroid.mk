FRAMEWORK_TEST_DIR ?= ${shell pwd}

FW_BT_TEST_DIR ?= $(FRAMEWORK_TEST_DIR)$(DELIM)test_bluedroid

BLUEDROID_DIR = $(FRAMEWORK_TEST_DIR)$(DELIM)..$(DELIM)bluedroid

ifeq ($(CONFIG_BT_TEST_OSI_ENABLED),y)
# include dir
CFLAGS += $(INCDIR_PREFIX)$(BLUEDROID_DIR)$(DELIM)porting$(DELIM)include
CFLAGS += $(INCDIR_PREFIX)$(BLUEDROID_DIR)$(DELIM)common$(DELIM)include
CFLAGS += $(INCDIR_PREFIX)$(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)include
CFLAGS += $(INCDIR_PREFIX)$(BLUEDROID_DIR)$(DELIM)common$(DELIM)btc$(DELIM)include
CFLAGS += $(INCDIR_PREFIX)$(BLUEDROID_DIR)$(DELIM)host$(DELIM)bluedroid$(DELIM)common$(DELIM)include
CFLAGS += $(INCDIR_PREFIX)$(BLUEDROID_DIR)$(DELIM)host$(DELIM)bluedroid$(DELIM)api$(DELIM)include$(DELIM)api

# source dir 使用静态库方式进行测试验证
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)alarm.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)allocator.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)buffer.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)config.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)fixed_queue.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)pkt_queue.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)fixed_pkt_queue.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)future.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)hash_functions.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)hash_map.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)list.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)mutex.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)thread.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)osi.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)semaphore.c
#CSRCS += $(BLUEDROID_DIR)$(DELIM)common$(DELIM)osi$(DELIM)event_bits_group.c

# test source dir
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_main.c

ifeq ($(CONFIG_BT_TEST_OSI_MEMORY_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_memory.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_ALLOCATOR_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_allocator.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_BUFFER_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_buffer.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_LIST_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_list.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_CONFIG_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_config.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_SEM_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_sem.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_FUTURE_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_future.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_THREAD_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_thread.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_EVENT_BITS_GROUPS_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_event_bits_group.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_MUTEX_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_mutex.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_HASH_MAP_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_hash_map.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_FIXED_QUEUE_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_fixed_queue.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_PKT_QUEUE_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_pkt_queue.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_FIXED_PKT_QUEUE_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_fixed_pkt_queue.c
endif

ifeq ($(CONFIG_BT_TEST_OSI_ALARM_ENABLED),y)
CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_osi$(DELIM)test_osi_alarm.c
endif

VPATH += $(FW_BT_TEST_DIR)$(DELIM)test_osi

endif # CONFIG_BT_TEST_OSI_ENABLED

ifeq ($(CONFIG_BT_TEST_BT_ENABLED),y)

VPATH += :test_bluedroid$(DELIM)test_bt

# CFLAGS += $(INCDIR_PREFIX)$(BLUEDROID_DIR)$(DELIM)host$(DELIM)bluedroid$(DELIM)api$(DELIM)include$(DELIM)api
#CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_bt$(DELIM)test_bt_main.c
#CSRCS += $(FW_BT_TEST_DIR)$(DELIM)test_bt$(DELIM)test_bt_esp_hal.c

CSRCS += test_bt_main.c
CSRCS += test_bt_esp_hal.c

endif