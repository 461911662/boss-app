#include <string.h>
#include "utils.h"
#include "wifi_status.h"

int handle_status(cgi_request_t *req, cgi_response_t *resp)
{
    wifi_status_t status;
    wifi_device_t devices[MAX_DEVICES];
    int device_count;

    wifi_get_status(&status);
    device_count = wifi_get_connected_devices(devices, MAX_DEVICES);

    resp->data = cJSON_CreateObject();
    cJSON_AddBoolToObject(resp->data, "connected", status.connected);
    cJSON_AddStringToObject(resp->data, "ssid", status.ssid);
    cJSON_AddNumberToObject(resp->data, "rssi", status.rssi);
    cJSON_AddNumberToObject(resp->data, "channel", status.channel);
    cJSON_AddStringToObject(resp->data, "band", status.band);
    cJSON_AddStringToObject(resp->data, "ip", status.ip);

    cJSON *devices_array = cJSON_CreateArray();
    for (int i = 0; i < device_count; i++)
    {
        cJSON *dev = cJSON_CreateObject();
        cJSON_AddStringToObject(dev, "name", devices[i].name);
        cJSON_AddStringToObject(dev, "ip", devices[i].ip);
        cJSON_AddItemToArray(devices_array, dev);
    }
    cJSON_AddItemToObject(resp->data, "devices", devices_array);

    return cgi_response_send_json(resp);
}
