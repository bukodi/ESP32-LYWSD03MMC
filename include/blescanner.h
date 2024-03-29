#pragma once

#include <esp_err.h>
 
 typedef struct _ATCInfo {
   int rssi;
   uint8_t macaddr[6];
   uint8_t advData[50];
} ATCInfo;


esp_err_t init_blescanner();

esp_err_t execute_blescan( uint32_t durationInSec, void (*scan_result_cb)(ATCInfo *) );

