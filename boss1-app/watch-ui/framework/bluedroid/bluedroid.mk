BLUEDROID_PATH ?= ${shell pwd}

# common include dirs
COMMON_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)common$(DELIM)osi$(DELIM)include
COMMON_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)common$(DELIM)api$(DELIM)include$(DELIM)api
COMMON_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)common$(DELIM)btc$(DELIM)profile$(DELIM)esp$(DELIM)blufi$(DELIM)include
COMMON_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)common$(DELIM)btc$(DELIM)profile$(DELIM)esp$(DELIM)include
COMMON_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)common$(DELIM)hci_log$(DELIM)include

# private include dirs
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)common$(DELIM)btc$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)common$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)porting$(DELIM)include

# Common Source
VPATH += :common$(DELIM)btc$(DELIM)core
VPATH += :common$(DELIM)btc$(DELIM)profile$(DELIM)esp$(DELIM)blufi
VPATH += :common$(DELIM)btc$(DELIM)profile$(DELIM)esp$(DELIM)blufi$(DELIM)bluedroid_host

CSRCS += btc_alarm.c btc_manage.c btc_task.c
CSRCS += blufi_prf.c blufi_protocol.c
CSRCS += esp_blufi.c

VPATH += :common$(DELIM)api
CSRCS += esp_blufi_api.c

VPATH += :common$(DELIM)osi
CSRCS += alarm.c allocator.c buffer.c config.c fixed_queue.c pkt_queue.c fixed_pkt_queue.c \
         future.c hash_functions.c hash_map.c list.c mutex.c thread.c osi.c semaphore.c event_bits_group.c

VPATH += :porting$(DELIM)mem
CSRCS += bt_osi_mem.c

ifeq ($(CONFIG_BT_BLUEDROID_ENABLED),y)

# bluedroid include dirs
BLUEDROID_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)api$(DELIM)include
BLUEDROID_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)api$(DELIM)include$(DELIM)api
BLUEDROID_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)hci$(DELIM)include

PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)ar$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)av$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)dm$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)gatt$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hf_ag$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hf_client$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hd$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hh$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)jv$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)sdp$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)bta$(DELIM)sys$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)device$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)external$(DELIM)sbc$(DELIM)decoder$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)external$(DELIM)sbc$(DELIM)encoder$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)external$(DELIM)sbc$(DELIM)plc$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)esp$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)a2dp$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)btc$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)btm$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)gap$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)gatt$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)hid$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)l2cap$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)sdp$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)smp$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)avct$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)avrc$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)avdt$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)a2dp$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)rfcomm$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)stack$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)common$(DELIM)include
PRIV_INCDIR += $(INCDIR_PREFIX)$(BLUEDROID_PATH)$(DELIM)host$(DELIM)bluedroid$(DELIM)config$(DELIM)include

VPATH += :host$(DELIM)bluedroid$(DELIM)api
CSRCS += esp_a2dp_api.c esp_avrc_api.c esp_bt_device.c esp_bt_main.c esp_gap_ble_api.c \
         esp_gap_bt_api.c esp_gatt_common_api.c esp_gattc_api.c esp_gatts_api.c esp_hidd_api.c esp_hidh_api.c \
         esp_hf_ag_api.c esp_hf_client_api.c esp_spp_api.c esp_sdp_api.c esp_l2cap_bt_api.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)ar
CSRCS += bta_ar.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)av
CSRCS += bta_av_aact.c bta_av_act.c bta_av_api.c bta_av_cfg.c bta_av_ci.c bta_av_main.c bta_av_sbc.c bta_av_ssm.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)dm
CSRCS += bta_dm_act.c bta_dm_api.c bta_dm_cfg.c bta_dm_ci.c bta_dm_co.c bta_dm_main.c bta_dm_pm.c bta_dm_sco.c \
         bta_dm_qos.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)gatt
CSRCS += bta_gatt_common.c bta_gattc_act.c bta_gattc_api.c bta_gattc_cache.c bta_gattc_ci.c bta_gattc_co.c \
         bta_gattc_main.c bta_gattc_utils.c bta_gatts_act.c bta_gatts_api.c bta_gatts_co.c \
         bta_gatts_main.c bta_gatts_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hd
CSRCS += bta_hd_api.c bta_hd_act.c bta_hd_main.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hh
CSRCS += bta_hh_act.c bta_hh_api.c bta_hh_cfg.c bta_hh_le.c bta_hh_main.c bta_hh_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)jv
CSRCS += bta_jv_act.c bta_jv_api.c bta_jv_cfg.c bta_jv_main.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hf_ag
CSRCS += bta_ag_act.c bta_ag_api.c bta_ag_at.c bta_ag_cfg.c bta_ag_cmd.c bta_ag_main.c bta_ag_rfc.c bta_ag_sco.c \
         bta_ag_sdp.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)hf_client
CSRCS += bta_hf_client_act.c bta_hf_client_api.c bta_hf_client_at.c bta_hf_client_cmd.c bta_hf_client_main.c \
         bta_hf_client_rfc.c bta_hf_client_sco.c bta_hf_client_sdp.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)sdp
CSRCS += bta_sdp.c bta_sdp_act.c bta_sdp_api.c bta_sdp_cfg.c

VPATH += :host$(DELIM)bluedroid$(DELIM)bta$(DELIM)sys
CSRCS += bta_sys_conn.c bta_sys_main.c utl.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)core
CSRCS += btc_ble_storage.c btc_config.c btc_dev.c btc_dm.c btc_main.c btc_profile_queue.c btc_sec.c \
         btc_sm.c btc_storage.c btc_util.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)a2dp
CSRCS += bta_av_co.c btc_a2dp.c btc_a2dp_control.c btc_a2dp_sink.c btc_a2dp_source.c btc_av.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)avrc
CSRCS += bta_avrc_co.c btc_avrc.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)hf_ag
CSRCS += bta_ag_co.c btc_hf_ag.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)hf_client
CSRCS += btc_hf_client.c bta_hf_client_co.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)hid
CSRCS += btc_hd.c btc_hh.c bta_hh_co.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)gap
CSRCS += btc_gap_ble.c btc_gap_bt.c bta_gap_bt_co.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)gatt
CSRCS += btc_gatt_common.c btc_gatt_util.c btc_gattc.c btc_gatts.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)spp
CSRCS += btc_spp.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)sdp
CSRCS += btc_sdp.c

VPATH += :host$(DELIM)bluedroid$(DELIM)btc$(DELIM)profile$(DELIM)std$(DELIM)l2cap
CSRCS += btc_l2cap.c

VPATH += :host$(DELIM)bluedroid$(DELIM)device
CSRCS += bdaddr.c controller.c interop.c

VPATH += :host$(DELIM)bluedroid$(DELIM)external$(DELIM)sbc$(DELIM)decoder$(DELIM)srce
CSRCS += alloc.c bitalloc-sbc.c bitalloc.c bitstream-decode.c decoder-oina.c decoder-private.c decoder-sbc.c dequant.c \
         framing-sbc.c framing.c oi_codec_version.c synthesis-8-generated.c synthesis-dct8.c synthesis-sbc.c 

VPATH += :host$(DELIM)bluedroid$(DELIM)external$(DELIM)sbc$(DELIM)encoder$(DELIM)srce
CSRCS += sbc_analysis.c sbc_dct.c sbc_dct_coeffs.c sbc_enc_bit_alloc_mono.c sbc_enc_bit_alloc_ste.c sbc_enc_coeffs.c \
		 sbc_encoder.c sbc_packing.c

VPATH += :host$(DELIM)bluedroid$(DELIM)external$(DELIM)sbc$(DELIM)plc
CSRCS += sbc_plc.c

VPATH += :host$(DELIM)bluedroid$(DELIM)hci$(DELIM)vendor
CSRCS += esp_hci_hal_h4.c hw_hci_hal_h4.c

VPATH += :host$(DELIM)bluedroid$(DELIM)hci
CSRCS += hci_audio.c hci_hal_h4.c hci_layer.c hci_packet_factory.c hci_packet_parser.c packet_fragmenter.c

VPATH += :host$(DELIM)bluedroid$(DELIM)main
CSRCS += bte_init.c bte_main.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)a2dp
CSRCS += a2d_api.c a2d_sbc.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)avct
CSRCS += avct_api.c avct_ccb.c avct_l2c.c avct_lcb.c avct_lcb_act.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)avdt
CSRCS += avdt_ad.c avdt_api.c avdt_ccb.c avdt_ccb_act.c avdt_l2c.c avdt_msg.c avdt_scb.c avdt_scb_act.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)avrc
CSRCS += avrc_api.c avrc_bld_ct.c avrc_bld_tg.c avrc_opt.c avrc_pars_ct.c avrc_pars_tg.c avrc_sdp.c \
         avrc_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)hid
CSRCS += hidd_api.c hidd_conn.c hidh_api.c hidh_conn.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)btm
CSRCS += btm_acl.c btm_ble.c btm_ble_addr.c btm_ble_adv_filter.c btm_ble_batchscan.c btm_ble_bgconn.c \
         btm_ble_cont_energy.c btm_ble_gap.c btm_ble_5_gap.c btm_ble_multi_adv.c btm_ble_privacy.c btm_dev.c \
         btm_devctl.c btm_inq.c btm_main.c btm_pm.c btm_sco.c btm_sec.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)btu
CSRCS += btu_hcif.c btu_init.c btu_task.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)gap
CSRCS += gap_api.c gap_ble.c gap_conn.c gap_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)gatt
CSRCS += att_protocol.c gatt_api.c gatt_attr.c gatt_auth.c gatt_cl.c gatt_db.c gatt_main.c gatt_sr.c \
         gatt_sr_hash.c gatt_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)hcic
CSRCS += hciblecmds.c hcicmds.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)l2cap
CSRCS += l2c_api.c l2c_ble.c l2c_csm.c l2c_fcr.c l2c_link.c l2c_main.c l2c_ucd.c l2c_utils.c l2cap_client.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)rfcomm
CSRCS += port_api.c port_rfc.c port_utils.c rfc_l2cap_if.c rfc_mx_fsm.c rfc_port_fsm.c rfc_port_if.c \
         rfc_ts_frames.c rfc_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)sdp
CSRCS += sdp_api.c sdp_db.c sdp_discovery.c sdp_main.c sdp_server.c sdp_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)stack$(DELIM)smp
CSRCS += aes.c p_256_curvepara.c p_256_ecc_pp.c p_256_multprecision.c smp_act.c smp_api.c smp_br_main.c \
         smp_cmac.c smp_keys.c smp_l2c.c smp_main.c smp_utils.c

VPATH += :host$(DELIM)bluedroid$(DELIM)config
CSRCS += stack_config.c

endif

CFLAGS += $(COMMON_INCDIR)
CFLAGS += $(PRIV_INCDIR)
CFLAGS += $(BLUEDROID_INCDIR) -Wall