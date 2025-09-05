/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef _BT_TRACE_H_
#define _BT_TRACE_H_

#include <assert.h>
#include <stdio.h>
#include <syslog.h>
#include "bt_common.h"
#include "bluedroid_user_config.h"
#include "stack/bt_types.h"
 
static inline void trc_dump_buffer(const char *prefix, uint8_t *data, uint16_t len)
{
    uint16_t i;

    if (!data || !len) {
        return;
    }

    if (prefix) {
        syslog(LOG_INFO, "%s: len %d\n", prefix, len);
    }

    for (i = 0; i < len; i+=16) {
        syslog(LOG_INFO, "    %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                i >= len ? 0xFF : *(data + i),
                i+1 >= len ? 0xFF : *(data + i + 1),
                i+2 >= len ? 0xFF : *(data + i + 2),
                i+3 >= len ? 0xFF : *(data + i + 3),
                i+4 >= len ? 0xFF : *(data + i + 4),
                i+5 >= len ? 0xFF : *(data + i + 5),
                i+6 >= len ? 0xFF : *(data + i + 6),
                i+7 >= len ? 0xFF : *(data + i + 7),
                i+8 >= len ? 0xFF : *(data + i + 8),
                i+9 >= len ? 0xFF : *(data + i + 9),
                i+10 >= len ? 0xFF : *(data + i + 10),
                i+11 >= len ? 0xFF : *(data + i + 11),
                i+12 >= len ? 0xFF : *(data + i + 12),
                i+13 >= len ? 0xFF : *(data + i + 13),
                i+14 >= len ? 0xFF : *(data + i + 14),
                i+15 >= len ? 0xFF : *(data + i + 15)
            );
    }
}

#ifndef BTTRC_DUMP_BUFFER
#define BTTRC_DUMP_BUFFER(_prefix, _data, _len) trc_dump_buffer(_prefix, _data, _len)
#else
#define BTTRC_DUMP_BUFFER(_prefix, _data, _len)
#endif

//static const char BTE_LOGMSG_MODULE[] = "bte_logmsg_module";

/* BTrgs);E tracing IDs for debug purposes */
/* LayerIDs for stack */
#define BTTRC_ID_STK_GKI                   1
#define BTTRC_ID_STK_BTU                   2
#define BTTRC_ID_STK_HCI                   3
#define BTTRC_ID_STK_L2CAP                 4
#define BTTRC_ID_STK_RFCM_MX               5
#define BTTRC_ID_STK_RFCM_PRT              6
#define BTTRC_ID_STK_OBEX_C                7
#define BTTRC_ID_STK_OBEX_S                8
#define BTTRC_ID_STK_AVCT                  9
#define BTTRC_ID_STK_AVDT                  10
#define BTTRC_ID_STK_AVRC                  11
#define BTTRC_ID_STK_BIC                   12
#define BTTRC_ID_STK_BIS                   13
#define BTTRC_ID_STK_BNEP                  14
#define BTTRC_ID_STK_BPP                   15
#define BTTRC_ID_STK_BTM_ACL               16
#define BTTRC_ID_STK_BTM_PM                17
#define BTTRC_ID_STK_BTM_DEV_CTRL          18
#define BTTRC_ID_STK_BTM_SVC_DSC           19
#define BTTRC_ID_STK_BTM_INQ               20
#define BTTRC_ID_STK_BTM_SCO               21
#define BTTRC_ID_STK_BTM_SEC               22
#define BTTRC_ID_STK_HID                   24
#define BTTRC_ID_STK_HSP2                  25
#define BTTRC_ID_STK_CTP                   26
#define BTTRC_ID_STK_FTC                   27
#define BTTRC_ID_STK_FTS                   28
#define BTTRC_ID_STK_GAP                   29
#define BTTRC_ID_STK_HCRP                  31
#define BTTRC_ID_STK_ICP                   32
#define BTTRC_ID_STK_OPC                   33
#define BTTRC_ID_STK_OPS                   34
#define BTTRC_ID_STK_PAN                   35
#define BTTRC_ID_STK_SAP                   36
#define BTTRC_ID_STK_SDP                   37
#define BTTRC_ID_STK_SLIP                  38
#define BTTRC_ID_STK_SPP                   39
#define BTTRC_ID_STK_TCS                   40
#define BTTRC_ID_STK_VDP                   41
#define BTTRC_ID_STK_MCAP                  42
#define BTTRC_ID_STK_GATT                  43
#define BTTRC_ID_STK_SMP                   44
#define BTTRC_ID_STK_NFC                   45
#define BTTRC_ID_STK_NCI                   46
#define BTTRC_ID_STK_IDEP                  47
#define BTTRC_ID_STK_NDEP                  48
#define BTTRC_ID_STK_LLCP                  49
#define BTTRC_ID_STK_RW                    50
#define BTTRC_ID_STK_CE                    51
#define BTTRC_ID_STK_SNEP                  52
#define BTTRC_ID_STK_NDEF                  53
#define BTTRC_ID_STK_HIDD                  54

/* LayerIDs for BTA */
#define BTTRC_ID_BTA_ACC                   55         /* Advanced Camera Client */
#define BTTRC_ID_BTA_AG                    56         /* audio gateway */
#define BTTRC_ID_BTA_AV                    57         /* Advanced audio */
#define BTTRC_ID_BTA_BIC                   58         /* Basic Imaging Client */
#define BTTRC_ID_BTA_BIS                   59         /* Basic Imaging Server */
#define BTTRC_ID_BTA_BP                    60         /* Basic Printing Client */
#define BTTRC_ID_BTA_CG                    61
#define BTTRC_ID_BTA_CT                    62         /* cordless telephony terminal */
#define BTTRC_ID_BTA_DG                    63         /* data gateway */
#define BTTRC_ID_BTA_DM                    64         /* device manager */
#define BTTRC_ID_BTA_DM_SRCH               65         /* device manager search */
#define BTTRC_ID_BTA_DM_SEC                66         /* device manager security */
#define BTTRC_ID_BTA_FM                    67
#define BTTRC_ID_BTA_FTC                   68         /* file transfer client */
#define BTTRC_ID_BTA_FTS                   69         /* file transfer server */
#define BTTRC_ID_BTA_HIDH                  70
#define BTTRC_ID_BTA_HIDD                  71
#define BTTRC_ID_BTA_JV                    72
#define BTTRC_ID_BTA_OPC                   73         /* object push client */
#define BTTRC_ID_BTA_OPS                   74         /* object push server */
#define BTTRC_ID_BTA_PAN                   75         /* Personal Area Networking */
#define BTTRC_ID_BTA_PR                    76         /* Printer client */
#define BTTRC_ID_BTA_SC                    77         /* SIM Card Access server */
#define BTTRC_ID_BTA_SS                    78         /* synchronization server */
#define BTTRC_ID_BTA_SYS                   79         /* system manager */
#define BTTRC_ID_AVDT_SCB                  80         /* avdt scb */
#define BTTRC_ID_AVDT_CCB                  81         /* avdt ccb */

// btla-specific ++
/* LayerIDs added for BTL-A. Probably should modify bte_logmsg.c in future. */
#define BTTRC_ID_STK_RFCOMM                82
#define BTTRC_ID_STK_RFCOMM_DATA           83
#define BTTRC_ID_STK_OBEX                  84
#define BTTRC_ID_STK_A2D                   85
#define BTTRC_ID_STK_BIP                   86

/* LayerIDs for BT APP */
#define BTTRC_ID_BTAPP                     87
#define BTTRC_ID_BT_PROTOCOL               88         /* this is a temporary solution to allow dynamic
                                                         enable/disable of BT_PROTOCOL_TRACE */
#define BTTRC_ID_MAX_ID                    BTTRC_ID_BT_PROTOCOL
// btla-specific --
#define BTTRC_ID_ALL_LAYERS                0xFF       /* all trace layers */
/* Parameter datatypes used in Trace APIs */
#define BTTRC_PARAM_UINT8                  1
#define BTTRC_PARAM_UINT16                 2
#define BTTRC_PARAM_UINT32                 3

/* Enables or disables verbose trace information. */
#ifndef BT_TRACE_VERBOSE
#define BT_TRACE_VERBOSE    FALSE
#endif

/* Enables or disables all trace messages. */
#ifndef BT_USE_TRACES
#define BT_USE_TRACES       FALSE
#endif

#ifndef BT_TRACE_APPL
#define BT_TRACE_APPL   BT_USE_TRACES
#endif

/******************************************************************************
**
** Trace Levels
**
** The following values may be used for different levels:
**      BT_TRACE_LEVEL_NONE    0        * No trace messages to be generated
**      BT_TRACE_LEVEL_ERROR   1        * Error condition trace messages
**      BT_TRACE_LEVEL_WARNING 2        * Warning condition trace messages
**      BT_TRACE_LEVEL_API     3        * API traces
**      BT_TRACE_LEVEL_EVENT   4        * Debug messages for events
**      BT_TRACE_LEVEL_DEBUG   5        * Debug messages (general)
******************************************************************************/

// btla-specific ++
/* Core Stack default trace levels */
#define HCI_INITIAL_TRACE_LEVEL             UC_BT_LOG_HCI_TRACE_LEVEL
#define BTM_INITIAL_TRACE_LEVEL             UC_BT_LOG_BTM_TRACE_LEVEL
#define L2CAP_INITIAL_TRACE_LEVEL           UC_BT_LOG_L2CAP_TRACE_LEVEL
#define RFCOMM_INITIAL_TRACE_LEVEL          UC_BT_LOG_RFCOMM_TRACE_LEVEL
#define SDP_INITIAL_TRACE_LEVEL             UC_BT_LOG_SDP_TRACE_LEVEL
#define GAP_INITIAL_TRACE_LEVEL             UC_BT_LOG_GAP_TRACE_LEVEL
#define BNEP_INITIAL_TRACE_LEVEL            UC_BT_LOG_BNEP_TRACE_LEVEL
#define PAN_INITIAL_TRACE_LEVEL             UC_BT_LOG_PAN_TRACE_LEVEL
#define A2D_INITIAL_TRACE_LEVEL             UC_BT_LOG_A2D_TRACE_LEVEL
#define AVDT_INITIAL_TRACE_LEVEL            UC_BT_LOG_AVDT_TRACE_LEVEL
#define AVCT_INITIAL_TRACE_LEVEL            UC_BT_LOG_AVCT_TRACE_LEVEL
#define AVRC_INITIAL_TRACE_LEVEL            UC_BT_LOG_AVRC_TRACE_LEVEL
#define MCA_INITIAL_TRACE_LEVEL             UC_BT_LOG_MCA_TRACE_LEVEL
#define HIDH_INITIAL_TRACE_LEVEL            UC_BT_LOG_HIDH_TRACE_LEVEL
#define HIDD_INITIAL_TRACE_LEVEL            UC_BT_LOG_HIDD_TRACE_LEVEL
#define APPL_INITIAL_TRACE_LEVEL            UC_BT_LOG_APPL_TRACE_LEVEL
#define GATT_INITIAL_TRACE_LEVEL            UC_BT_LOG_GATT_TRACE_LEVEL
#define SMP_INITIAL_TRACE_LEVEL             UC_BT_LOG_SMP_TRACE_LEVEL
#define BTIF_INITIAL_TRACE_LEVEL            UC_BT_LOG_BTIF_TRACE_LEVEL

// btla-specific --

#if !UC_BT_STACK_NO_LOG
#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL  UC_LOG_DEFAULT_LEVEL
#endif

/* Define tracing for BTM
*/
#define BTM_TRACE_ERROR(fmt, args...)       {if (btm_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_BTM", fmt, ## args);}
#define BTM_TRACE_WARNING(fmt, args...)     {if (btm_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_BTM", fmt, ## args);}
#define BTM_TRACE_API(fmt, args...)         {if (btm_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_BTM", fmt, ## args);}
#define BTM_TRACE_EVENT(fmt, args...)       {if (btm_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_BTM", fmt, ## args);}
#define BTM_TRACE_DEBUG(fmt, args...)       {if (btm_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_BTM", fmt, ## args);}

/* Define tracing for the L2CAP unit
*/
#define L2CAP_TRACE_ERROR(fmt, args...)     {if (l2cb.l2cap_trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_L2CAP", fmt, ## args);}
#define L2CAP_TRACE_WARNING(fmt, args...)   {if (l2cb.l2cap_trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_L2CAP", fmt, ## args);}
#define L2CAP_TRACE_API(fmt, args...)       {if (l2cb.l2cap_trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_L2CAP", fmt, ## args);}
#define L2CAP_TRACE_EVENT(fmt, args...)     {if (l2cb.l2cap_trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_L2CAP", fmt, ## args);}
#define L2CAP_TRACE_DEBUG(fmt, args...)     {if (l2cb.l2cap_trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_L2CAP", fmt, ## args);}

/* Define tracing for the SDP unit
*/
#define SDP_TRACE_ERROR(fmt, args...)       {if (sdp_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_SDP", fmt, ## args);}
#define SDP_TRACE_WARNING(fmt, args...)     {if (sdp_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_SDP", fmt, ## args);}
#define SDP_TRACE_API(fmt, args...)         {if (sdp_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_SDP", fmt, ## args);}
#define SDP_TRACE_EVENT(fmt, args...)       {if (sdp_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_SDP", fmt, ## args);}
#define SDP_TRACE_DEBUG(fmt, args...)       {if (sdp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_SDP", fmt, ## args);}

/* Define tracing for the RFCOMM unit
*/
#define RFCOMM_TRACE_ERROR(fmt, args...)    {if (rfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_RFCOMM", fmt, ## args);}
#define RFCOMM_TRACE_WARNING(fmt, args...)  {if (rfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_RFCOMM", fmt, ## args);}
#define RFCOMM_TRACE_API(fmt, args...)      {if (rfc_cb.trace_level >=BT_TRACE_LEVEL_API) BT_PRINT_I("BT_RFCOMM", fmt, ## args);}
#define RFCOMM_TRACE_EVENT(fmt, args...)    {if (rfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_RFCOMM", fmt, ## args);}
#define RFCOMM_TRACE_DEBUG(fmt, args...)    {if (rfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_RFCOMM", fmt, ## args);}

/* Generic Access Profile traces */
#define GAP_TRACE_ERROR(fmt, args...)       {if (gap_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_GAP", fmt, ## args);}
#define GAP_TRACE_API(fmt, args...)         {if (gap_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_GAP", fmt, ## args);}
#define GAP_TRACE_EVENT(fmt, args...)       {if (gap_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_GAP", fmt, ## args);}
#define GAP_TRACE_WARNING(fmt, args...)     {if (gap_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_GAP", fmt, ## args);}

/* define traces for HID Host */
#define HIDH_TRACE_ERROR(fmt, args...)      {if (hh_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_HIDH", fmt, ## args);}
#define HIDH_TRACE_WARNING(fmt, args...)    {if (hh_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_HIDH", fmt, ## args);}
#define HIDH_TRACE_API(fmt, args...)        {if (hh_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_HIDH", fmt, ## args);}
#define HIDH_TRACE_EVENT(fmt, args...)      {if (hh_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_HIDH", fmt, ## args);}
#define HIDH_TRACE_DEBUG(fmt, args...)      {if (hh_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_HIDH", fmt, ## args);}

/* define traces for HID Device */
#define HIDD_TRACE_ERROR(fmt, args...)      {if (hd_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_HIDD", fmt, ## args);}
#define HIDD_TRACE_WARNING(fmt, args...)    {if (hd_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_HIDD", fmt, ## args);}
#define HIDD_TRACE_API(fmt, args...)        {if (hd_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_HIDD", fmt, ## args);}
#define HIDD_TRACE_EVENT(fmt, args...)      {if (hd_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_HIDD", fmt, ## args);}
#define HIDD_TRACE_DEBUG(fmt, args...)      {if (hd_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_HIDD", fmt, ## args);}
#define HIDD_TRACE_VERBOSE(fmt, args...)    {if (hd_cb.trace_level >= BT_TRACE_LEVEL_VERBOSE) BT_PRINT_D("BT_HIDD", fmt, ## args);}

/* define traces for BNEP */

#define BNEP_TRACE_ERROR(fmt, args...)      {if (bnep_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_BNEP", fmt, ## args);}
#define BNEP_TRACE_WARNING(fmt, args...)    {if (bnep_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_BNEP", fmt, ## args);}
#define BNEP_TRACE_API(fmt, args...)        {if (bnep_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_BNEP", fmt, ## args);}
#define BNEP_TRACE_EVENT(fmt, args...)      {if (bnep_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_BNEP", fmt, ## args);}
#define BNEP_TRACE_DEBUG(fmt, args...)      {if (bnep_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_BNEP", fmt, ## args);}

/* define traces for PAN */

#define PAN_TRACE_ERROR(fmt, args...)       {if (pan_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_PAN", fmt, ## args);}
#define PAN_TRACE_WARNING(fmt, args...)     {if (pan_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_PAN", fmt, ## args);}
#define PAN_TRACE_API(fmt, args...)         {if (pan_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_PAN", fmt, ## args);}
#define PAN_TRACE_EVENT(fmt, args...)       {if (pan_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_PAN", fmt, ## args);}
#define PAN_TRACE_DEBUG(fmt, args...)       {if (pan_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_PAN", fmt, ## args);}

/* Define tracing for the A2DP profile
*/
#define A2D_TRACE_ERROR(fmt, args...)       {if (a2d_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_A2D", fmt, ## args);}
#define A2D_TRACE_WARNING(fmt, args...)     {if (a2d_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_A2D", fmt, ## args);}
#define A2D_TRACE_API(fmt, args...)         {if (a2d_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_A2D", fmt, ## args);}
#define A2D_TRACE_EVENT(fmt, args...)       {if (a2d_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_A2D", fmt, ## args);}
#define A2D_TRACE_DEBUG(fmt, args...)       {if (a2d_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_A2D", fmt, ## args);}

/* AVDTP
*/
#define AVDT_TRACE_ERROR(fmt, args...)      {if (avdt_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_AVDT", fmt, ## args);}
#define AVDT_TRACE_WARNING(fmt, args...)    {if (avdt_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_AVDT", fmt, ## args);}
#define AVDT_TRACE_API(fmt, args...)        {if (avdt_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_AVDT", fmt, ## args);}
#define AVDT_TRACE_EVENT(fmt, args...)      {if (avdt_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_AVDT", fmt, ## args);}
#define AVDT_TRACE_DEBUG(fmt, args...)      {if (avdt_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_AVDT", fmt, ## args);}

/* Define tracing for the AVCTP protocol
*/
#define AVCT_TRACE_ERROR(fmt, args...)      {if (avct_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_AVCT", fmt, ## args);}
#define AVCT_TRACE_WARNING(fmt, args...)    {if (avct_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_AVCT", fmt, ## args);}
#define AVCT_TRACE_API(fmt, args...)        {if (avct_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_AVCT", fmt, ## args);}
#define AVCT_TRACE_EVENT(fmt, args...)      {if (avct_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_AVCT", fmt, ## args);}
#define AVCT_TRACE_DEBUG(fmt, args...)      {if (avct_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_AVCT", fmt, ## args);}

/* Define tracing for the AVRCP profile
*/
#define AVRC_TRACE_ERROR(fmt, args...)      {if (avrc_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_AVRC", fmt, ## args);}
#define AVRC_TRACE_WARNING(fmt, args...)    {if (avrc_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_AVRC", fmt, ## args);}
#define AVRC_TRACE_API(fmt, args...)        {if (avrc_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_AVRC", fmt, ## args);}
#define AVRC_TRACE_EVENT(fmt, args...)      {if (avrc_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_AVRC", fmt, ## args);}
#define AVRC_TRACE_DEBUG(fmt, args...)      {if (avrc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_AVRC", fmt, ## args);}

/* MCAP
*/
#define MCA_TRACE_ERROR(fmt, args...)       {if (mca_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_MCA", fmt, ## args);}
#define MCA_TRACE_WARNING(fmt, args...)     {if (mca_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_MCA", fmt, ## args);}
#define MCA_TRACE_API(fmt, args...)         {if (mca_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_MCA", fmt, ## args);}
#define MCA_TRACE_EVENT(fmt, args...)       {if (mca_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_MCA", fmt, ## args);}
#define MCA_TRACE_DEBUG(fmt, args...)       {if (mca_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_MCA", fmt, ## args);}

/* Define tracing for the ATT/GATT unit
*/
#define GATT_TRACE_ERROR(fmt, args...)      {if (gatt_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_GATT", fmt, ## args);}
#define GATT_TRACE_WARNING(fmt, args...)    {if (gatt_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_GATT", fmt, ## args);}
#define GATT_TRACE_API(fmt, args...)        {if (gatt_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_GATT", fmt, ## args);}
#define GATT_TRACE_EVENT(fmt, args...)      {if (gatt_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_GATT", fmt, ## args);}
#define GATT_TRACE_DEBUG(fmt, args...)      {if (gatt_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_GATT", fmt, ## args);}

/* Define tracing for the SMP unit
*/
#define SMP_TRACE_ERROR(fmt, args...)       {if (smp_cb.trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_SMP", fmt, ## args);}
#define SMP_TRACE_WARNING(fmt, args...)     {if (smp_cb.trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_SMP", fmt, ## args);}
#define SMP_TRACE_API(fmt, args...)         {if (smp_cb.trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_SMP", fmt, ## args);}
#define SMP_TRACE_EVENT(fmt, args...)       {if (smp_cb.trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_SMP", fmt, ## args);}
#define SMP_TRACE_DEBUG(fmt, args...)       {if (smp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_SMP", fmt, ## args);}


extern UINT8 btif_trace_level;

// define traces for application
#define BTIF_TRACE_ERROR(fmt, args...)      {if (btif_trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_BTIF", fmt, ## args);}
#define BTIF_TRACE_WARNING(fmt, args...)    {if (btif_trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_BTIF", fmt, ## args);}
#define BTIF_TRACE_API(fmt, args...)        {if (btif_trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_BTIF", fmt, ## args);}
#define BTIF_TRACE_EVENT(fmt, args...)      {if (btif_trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_BTIF", fmt, ## args);}
#define BTIF_TRACE_DEBUG(fmt, args...)      {if (btif_trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_BTIF", fmt, ## args);}
#define BTIF_TRACE_VERBOSE(fmt, args...)    {if (btif_trace_level >= BT_TRACE_LEVEL_VERBOSE) BT_PRINT_V("BT_BTIF", fmt, ## args);}

/* define traces for application */

#define APPL_TRACE_ERROR(fmt, args...)      {if (appl_trace_level >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_APPL", fmt, ## args);}
#define APPL_TRACE_WARNING(fmt, args...)    {if (appl_trace_level >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_APPL", fmt, ## args);}
#define APPL_TRACE_API(fmt, args...)        {if (appl_trace_level >= BT_TRACE_LEVEL_API) BT_PRINT_I("BT_APPL", fmt, ## args);}
#define APPL_TRACE_EVENT(fmt, args...)      {if (appl_trace_level >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_APPL", fmt, ## args);}
#define APPL_TRACE_DEBUG(fmt, args...)      {if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_APPL", fmt, ## args);}
#define APPL_TRACE_VERBOSE(fmt, args...)    {if (appl_trace_level >= BT_TRACE_LEVEL_VERBOSE) BT_PRINT_V("BT_APPL", fmt, ## args);}

/* Define tracing for the HCI unit
 * Modified from `btu_cb.trace_level` to `HCI_INITIAL_TRACE_LEVEL`,
 * to use HCI_TRACE_XXXX in hci_layer.c without including `btu.h`
*/
#define HCI_TRACE_ERROR(fmt, args...)       {if (HCI_INITIAL_TRACE_LEVEL >= BT_TRACE_LEVEL_ERROR) BT_PRINT_E("BT_HCI", fmt,## args);}
#define HCI_TRACE_WARNING(fmt, args...)     {if (HCI_INITIAL_TRACE_LEVEL >= BT_TRACE_LEVEL_WARNING) BT_PRINT_W("BT_HCI", fmt,## args);}
#define HCI_TRACE_EVENT(fmt, args...)       {if (HCI_INITIAL_TRACE_LEVEL >= BT_TRACE_LEVEL_EVENT) BT_PRINT_D("BT_HCI", fmt,## args);}
#define HCI_TRACE_DEBUG(fmt, args...)       {if (HCI_INITIAL_TRACE_LEVEL >= BT_TRACE_LEVEL_DEBUG) BT_PRINT_D("BT_HCI", fmt,## args);}

#else
#define BT_PRINT_E(fmt, args...)
#define BT_PRINT_W(fmt, args...)
#define BT_PRINT_I(fmt, args...)
#define BT_PRINT_D(fmt, args...)
#define BT_PRINT_V(fmt, args...)

/* Define tracing for the HCI unit
*/
#define HCI_TRACE_ERROR(fmt, args...)
#define HCI_TRACE_WARNING(fmt, args...)
#define HCI_TRACE_EVENT(fmt, args...)
#define HCI_TRACE_DEBUG(fmt, args...)

/* Define tracing for BTM
*/
#define BTM_TRACE_ERROR(fmt, args...)
#define BTM_TRACE_WARNING(fmt, args...)
#define BTM_TRACE_API(fmt, args...)
#define BTM_TRACE_EVENT(fmt, args...)
#define BTM_TRACE_DEBUG(fmt, args...)

/* Define tracing for the L2CAP unit
*/
#define L2CAP_TRACE_ERROR(fmt, args...)
#define L2CAP_TRACE_WARNING(fmt, args...)
#define L2CAP_TRACE_API(fmt, args...)
#define L2CAP_TRACE_EVENT(fmt, args...)
#define L2CAP_TRACE_DEBUG(fmt, args...)

/* Define tracing for the SDP unit
*/
#define SDP_TRACE_ERROR(fmt, args...)
#define SDP_TRACE_WARNING(fmt, args...)
#define SDP_TRACE_API(fmt, args...)
#define SDP_TRACE_EVENT(fmt, args...)
#define SDP_TRACE_DEBUG(fmt, args...)

/* Define tracing for the RFCOMM unit
*/
#define RFCOMM_TRACE_ERROR(fmt, args...)
#define RFCOMM_TRACE_WARNING(fmt, args...)
#define RFCOMM_TRACE_API(fmt, args...)
#define RFCOMM_TRACE_EVENT(fmt, args...)
#define RFCOMM_TRACE_DEBUG(fmt, args...)

/* Generic Access Profile traces */
#define GAP_TRACE_ERROR(fmt, args...)
#define GAP_TRACE_EVENT(fmt, args...)
#define GAP_TRACE_API(fmt, args...)
#define GAP_TRACE_WARNING(fmt, args...)

/* define traces for HID Host */
#define HIDH_TRACE_ERROR(fmt, args...)
#define HIDH_TRACE_WARNING(fmt, args...)
#define HIDH_TRACE_API(fmt, args...)
#define HIDH_TRACE_EVENT(fmt, args...)
#define HIDH_TRACE_DEBUG(fmt, args...)

/* define traces for HID Device */
#define HIDD_TRACE_ERROR(fmt, args...)
#define HIDD_TRACE_WARNING(fmt, args...)
#define HIDD_TRACE_API(fmt, args...)
#define HIDD_TRACE_EVENT(fmt, args...)
#define HIDD_TRACE_DEBUG(fmt, args...)
#define HIDD_TRACE_VERBOSE(fmt, args...)

/* define traces for BNEP */

#define BNEP_TRACE_ERROR(fmt, args...)
#define BNEP_TRACE_WARNING(fmt, args...)
#define BNEP_TRACE_API(fmt, args...)
#define BNEP_TRACE_EVENT(fmt, args...)
#define BNEP_TRACE_DEBUG(fmt, args...)

/* define traces for PAN */

#define PAN_TRACE_ERROR(fmt, args...)
#define PAN_TRACE_WARNING(fmt, args...)
#define PAN_TRACE_API(fmt, args...)
#define PAN_TRACE_EVENT(fmt, args...)
#define PAN_TRACE_DEBUG(fmt, args...)

/* Define tracing for the A2DP profile
*/
#define A2D_TRACE_ERROR(fmt, args...)
#define A2D_TRACE_WARNING(fmt, args...)
#define A2D_TRACE_EVENT(fmt, args...)
#define A2D_TRACE_DEBUG(fmt, args...)
#define A2D_TRACE_API(fmt, args...)

/* AVDTP
*/
#define AVDT_TRACE_ERROR(fmt, args...)
#define AVDT_TRACE_WARNING(fmt, args...)
#define AVDT_TRACE_EVENT(fmt, args...)
#define AVDT_TRACE_DEBUG(fmt, args...)
#define AVDT_TRACE_API(fmt, args...)

/* Define tracing for the AVCTP protocol
*/
#define AVCT_TRACE_ERROR(fmt, args...)
#define AVCT_TRACE_WARNING(fmt, args...)
#define AVCT_TRACE_EVENT(fmt, args...)
#define AVCT_TRACE_DEBUG(fmt, args...)
#define AVCT_TRACE_API(fmt, args...)

/* Define tracing for the AVRCP profile
*/
#define AVRC_TRACE_ERROR(fmt, args...)
#define AVRC_TRACE_WARNING(fmt, args...)
#define AVRC_TRACE_EVENT(fmt, args...)
#define AVRC_TRACE_DEBUG(fmt, args...)
#define AVRC_TRACE_API(fmt, args...)

/* MCAP
*/
#define MCA_TRACE_ERROR(fmt, args...)
#define MCA_TRACE_WARNING(fmt, args...)
#define MCA_TRACE_EVENT(fmt, args...)
#define MCA_TRACE_DEBUG(fmt, args...)
#define MCA_TRACE_API(fmt, args...)

/* Define tracing for the ATT/GATT unit
*/
#define GATT_TRACE_ERROR(fmt, args...)
#define GATT_TRACE_WARNING(fmt, args...)
#define GATT_TRACE_API(fmt, args...)
#define GATT_TRACE_EVENT(fmt, args...)
#define GATT_TRACE_DEBUG(fmt, args...)

/* Define tracing for the SMP unit
*/
#define SMP_TRACE_ERROR(fmt, args...)
#define SMP_TRACE_WARNING(fmt, args...)
#define SMP_TRACE_API(fmt, args...)
#define SMP_TRACE_EVENT(fmt, args...)
#define SMP_TRACE_DEBUG(fmt, args...)

extern UINT8 btif_trace_level;

// define traces for application
#define BTIF_TRACE_ERROR(fmt, args...)
#define BTIF_TRACE_WARNING(fmt, args...)
#define BTIF_TRACE_API(fmt, args...)
#define BTIF_TRACE_EVENT(fmt, args...)
#define BTIF_TRACE_DEBUG(fmt, args...)
#define BTIF_TRACE_VERBOSE(fmt, args...)

/* define traces for application */

#define APPL_TRACE_ERROR(fmt, args...)
#define APPL_TRACE_WARNING(fmt, args...)
#define APPL_TRACE_API(fmt, args...)
#define APPL_TRACE_EVENT(fmt, args...)
#define APPL_TRACE_DEBUG(fmt, args...)
#define APPL_TRACE_VERBOSE(fmt, args...)

#endif  ///!UC_BT_STACK_NO_LOG


/* Simplified Trace Helper Macro
*/
#define bdld(fmt, args...) \
    do{\
        if((MY_LOG_LEVEL) >= BT_TRACE_LEVEL_DEBUG) \
        BT_PRINT_D(fmt, ## args);        \
    }while(0)

#define bdlw(fmt, args...) \
    do{\
        if((MY_LOG_LEVEL) >= BT_TRACE_LEVEL_WARNING) \
        BT_PRINT_W(fmt, ## args);        \
    }while(0)

#define bdle(fmt, args...) \
    do{\
        if((MY_LOG_LEVEL) >= BT_TRACE_LEVEL_ERROR) \
        BT_PRINT_E(fmt, ## args);        \
    }while(0)

#define bdla(assert_if) \
    do{\
        if(((MY_LOG_LEVEL) >= BT_TRACE_LEVEL_ERROR) && !(assert_if)) \
        BT_PRINT_E("%s: assert failed\n", #assert_if);       \
    }while(0)

typedef UINT8 tBTTRC_PARAM_TYPE;
typedef UINT8 tBTTRC_LAYER_ID;
typedef UINT8 tBTTRC_TYPE;

typedef struct {
    tBTTRC_LAYER_ID layer_id;
    tBTTRC_TYPE     type;      /* TODO: use tBTTRC_TYPE instead of "classical level 0-5" */
} tBTTRC_LEVEL;

typedef UINT8 (tBTTRC_SET_TRACE_LEVEL)( UINT8 );

typedef struct {
    const tBTTRC_LAYER_ID         layer_id_start;
    const tBTTRC_LAYER_ID         layer_id_end;
    tBTTRC_SET_TRACE_LEVEL        *p_f;
    const char                    *trc_name;
    UINT8                         trace_level;
} tBTTRC_FUNC_MAP;

/* External declaration for appl_trace_level here to avoid to add the declaration in all the files using APPL_TRACExxx macros */
extern UINT8 appl_trace_level;

#endif /*_BT_TRACE_H_*/
