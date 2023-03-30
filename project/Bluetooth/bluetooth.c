/*
 * bluetooth.c
 *
 *  Created on: 2022年11月22日
 *      Author: literem
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "bluetooth.h"
#include "response.h"
#include "receive.h"

//#define PRINT_DEBUG
#define SPP_SERVER_NAME 	"LED_Matrix_Server"
#define BT_DEVICE_NAME 		"LED_Matrix"
#define sec_mask 			ESP_SPP_SEC_AUTHENTICATE
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;
static uint32_t bt_handle;

u8 array_cmd_error[] = "Command ERROR!";
u8 array_value_error[] = "Value ERROR!";
u8 array_ok[] = {'#','O','K',':','0','0',';'};

void onCommandError()
{
#ifdef PRINT_DEBUG
	printf("Command ERROR!\n");
#endif
	if(bt_handle != 0)
	{
		esp_spp_write(bt_handle,14,array_cmd_error);
	}
}

void onValueError()
{
#ifdef PRINT_DEBUG
	printf("Value ERROR!\n");
#endif
	if(bt_handle != 0)
	{
		esp_spp_write(bt_handle,12,array_value_error);
	}
}

void onSuccess()
{
#ifdef PRINT_DEBUG
	printf("OK!\n");
#endif
	if(bt_handle != 0)
	{
		esp_spp_write(bt_handle,7,array_ok);
	}
}

void btSendResponse(u8 *dat)
{
	if(bt_handle != 0)
		esp_spp_write(bt_handle,RSP_DATA_LEN,dat);
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch(event)
    {
		case ESP_SPP_INIT_EVT://SPP初始化时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
			esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME);
			break;
		case ESP_SPP_OPEN_EVT://SPP客户端连接打开时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");
			break;
		case ESP_SPP_CLOSE_EVT://SPP客户端连接关闭时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT");
			bt_handle = 0;
			break;
		case ESP_SPP_START_EVT://SPP服务器启动时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT");
			esp_bt_dev_set_device_name(BT_DEVICE_NAME);
			esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
			break;

		case ESP_SPP_DATA_IND_EVT://当SPP连接接收到数据时，事件发生，仅适用于ESP_SPP_MODE_CB
			//ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d",param->data_ind.len, param->data_ind.handle);
			receiveHandle(param->data_ind.data,param->data_ind.len);
			break;

		case ESP_SPP_SRV_OPEN_EVT://SPP服务器连接打开时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT");
			bt_handle = param->cong.handle;
			break;

		case ESP_SPP_CL_INIT_EVT://SPP客户端启动连接时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
			break;
		case ESP_SPP_CONG_EVT://当SPP连接拥塞状态更改时，事件发生，仅适用于ESP_SPP_MODE_CB
			//ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
			break;
		case ESP_SPP_WRITE_EVT://SPP写入操作完成后，事件发生，仅适用于ESP_SPP_MODE_CB
			//ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT");
			break;
		case ESP_SPP_SRV_STOP_EVT://SPP服务器连接关闭时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_STOP_EVT");
			break;
		case ESP_SPP_UNINIT_EVT://SPP服务未初始化时
			//ESP_LOGI(SPP_TAG, "ESP_SPP_UNINIT_EVT");
			break;
		case ESP_SPP_DISCOVERY_COMP_EVT:
			break;
		default:break;
    }
}

/**
 * 配对处理
 */
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
	if(event == ESP_BT_GAP_PIN_REQ_EVT)//传统配对Pin码请求
	{
		if (param->pin_req.min_16_digit)
		{
			//ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
			esp_bt_pin_code_t pin_code = {0};
			esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
		}
		else
		{
			//ESP_LOGI(SPP_TAG, "Input pin code: 1234");
			esp_bt_pin_code_t pin_code;
			pin_code[0] = '1';
			pin_code[1] = '2';
			pin_code[2] = '3';
			pin_code[3] = '4';
			esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
		}
	}
}


void btSendData(u8 *dat,u8 len)
{
	if(bt_handle != 0)
	{
		esp_spp_write(bt_handle,len,dat);
	}
}

void btInit(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        //ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        //ESP_LOGE(SPP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        //ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        //ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        //ESP_LOGE(SPP_TAG, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        //ESP_LOGE(SPP_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        //ESP_LOGE(SPP_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    //不使用pin配对
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);
}
