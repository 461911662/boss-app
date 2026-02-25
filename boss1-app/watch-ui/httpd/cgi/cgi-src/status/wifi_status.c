#include <stdio.h>
#include <string.h>
#include "wifi_status.h"

int wifi_get_status(wifi_status_t *status)
{
    if (!status)
        return -1;

    memset(status, 0, sizeof(wifi_status_t));

    status->connected = true;
    strncpy(status->ssid, "BOSS1", sizeof(status->ssid) - 1);
    strncpy(status->ip, "192.168.4.2", sizeof(status->ip) - 1);
    status->rssi = -55;
    status->channel = 6;
    strncpy(status->band, "2.4G", sizeof(status->band) - 1);

    return 0;
}

int wifi_get_connected_devices(wifi_device_t *devices, int max_devices)
{
    if (!devices || max_devices <= 0)
        return 0;

    memset(devices, 0, sizeof(wifi_device_t) * max_devices);

    return 0;
}
