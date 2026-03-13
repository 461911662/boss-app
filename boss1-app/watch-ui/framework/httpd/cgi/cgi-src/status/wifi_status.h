#ifndef CGI_STATUS_WIFI_STATUS_H
#define CGI_STATUS_WIFI_STATUS_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_DEVICES 8

typedef struct
{
    bool connected;
    char ssid[32];
    int rssi;
    int channel;
    char band[16];
    char ip[16];
} wifi_status_t;

typedef struct
{
    char name[32];
    char ip[16];
} wifi_device_t;

int wifi_get_status(wifi_status_t *status);
int wifi_get_connected_devices(wifi_device_t *devices, int max_devices);

#endif
