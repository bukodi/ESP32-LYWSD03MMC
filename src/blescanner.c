#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"

#include "err_utils.h"
#include "blescanner.h"

#define TAG "BLESCAN"

static void (*_scan_result_cb)(ATCInfo *) = NULL;

/* Declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_PASSIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_window            = 0x30,
    .scan_interval          = 0x50,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

typedef struct {
    int rssi;
    uint8_t macaddr[3];
    uint16_t temp;    
    uint8_t humPercent;
    uint8_t batPercent;
    uint16_t batMv;
} received_data;

void handleRecivedData( received_data *pData ) 
{
    ESP_LOGI(TAG, "MAC: %02x%02x%02x, RSSI: %d, temp: %d, hum: %d%%, bat: %d%% (%d mV)", 
        pData->macaddr[0], pData->macaddr[1], pData->macaddr[2], 
        pData->rssi,
        pData->temp,
        pData->humPercent,
        pData->batPercent, pData->batMv);
    free( pData);
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        //the unit of the duration is second
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGD(TAG, "scan start success");

        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            if( scan_result->scan_rst.bda[0] != 0xa4 || scan_result->scan_rst.bda[1] != 0xc1 || scan_result->scan_rst.bda[2] != 0x38 ) {
                // Skip if the MAC address doesn't start with a4 c1 38
                break;
            }
            if (scan_result->scan_rst.adv_data_len <= 0) {
                break;
            }
            if( scan_result->scan_rst.ble_adv[4] != 0xa4 || scan_result->scan_rst.ble_adv[5] != 0xc1 || scan_result->scan_rst.ble_adv[6] != 0x38 ) {
                // Skip if the MAC address doesn't start with a4 c1 38
                break;
            }
            
            received_data *pData = (received_data*)malloc( sizeof( received_data) );
            pData->rssi = scan_result->scan_rst.rssi;
            pData->macaddr[0] = scan_result->scan_rst.ble_adv[7];
            pData->macaddr[1] = scan_result->scan_rst.ble_adv[8];
            pData->macaddr[2] = scan_result->scan_rst.ble_adv[9];
            pData->temp = scan_result->scan_rst.ble_adv[10] * 256 + scan_result->scan_rst.ble_adv[11];
            pData->humPercent = scan_result->scan_rst.ble_adv[12];
            pData->batPercent = scan_result->scan_rst.ble_adv[13];
            pData->batMv = scan_result->scan_rst.ble_adv[14] * 256 + scan_result->scan_rst.ble_adv[15];
            //esp_log_buffer_hex(TAG, scan_result->scan_rst.bda, 6);
            //ESP_LOGI(TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            //adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
            //                                    ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            //ESP_LOGI(TAG, "searched Device Name Len %d", adv_name_len);
            //esp_log_buffer_char(TAG, adv_name, adv_name_len);

            //ESP_LOGI(TAG, "device RSSI = %d", scan_result->scan_rst.rssi);

            if( pData->batMv > 3000 || pData->temp > 300 ) {
                ESP_LOGW(TAG, "adv data:");
                ESP_LOG_BUFFER_HEX_LEVEL( TAG, &scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len, ESP_LOG_WARN);
            }
            handleRecivedData( pData );

            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
        break;
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        //scan stop complete event to indicate scan stop successfully or failed
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "scan stop failed, error status = %x", param->scan_stop_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "scan stop success");

        break;

    default:
        break;
    }
}


esp_err_t execute_blescan( uint32_t durationInSec, void (*scan_result_cb)(ATCInfo *) ) {
    _scan_result_cb = scan_result_cb;
    esp_ble_gap_stop_scanning(); // Intentionally ignore the error
    ERROR_RETURN( esp_ble_gap_start_scanning(durationInSec) );
    return ESP_OK;
}

esp_err_t init_blescanner() {

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ERROR_RETURN( esp_bt_controller_init(&bt_cfg) );
    ERROR_RETURN( esp_bt_controller_enable(ESP_BT_MODE_BLE) );
    ERROR_RETURN( esp_bluedroid_init() );
    ERROR_RETURN( esp_bluedroid_enable() );

    //register the  callback function to the gap module
    esp_err_t ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret){
        ESP_LOGE(TAG, "%s gap register failed, error code = %x\n", __func__, ret);
        return ESP_OK;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }

    ret = esp_ble_gap_set_scan_params(&ble_scan_params);
    if (ret){
        ESP_LOGE(TAG, "set scan params error, error code = %x", ret);
    } else {
        ESP_LOGI(TAG, "scan params set");
    }
    
    return ESP_OK;
}
