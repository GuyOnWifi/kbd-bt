#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/idf_additions.h"
#include "gap.h"
#include "gatt_svr.h"
#include "heart_rate.h"
#include "host/ble_hs.h"
#include "host/ble_store.h"
#include "nimble/nimble_port.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include <stdint.h>
#include <stdio.h>

// Forward declaration touse internal API
void ble_store_config_init(void);

static void on_stack_reset(int reason) {
  // On reset, print reset reason to console
  ESP_LOGI(TAG, "nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync() {
  // It is now ready to advertise
  adv_init();
}

static void nimble_host_config_init() {
  ble_hs_cfg.reset_cb = on_stack_reset;
  ble_hs_cfg.sync_cb = on_stack_sync;
  ble_hs_cfg.gatts_register_cb = kbd_gatt_svr_register_cb;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

  // Store host config
  ble_store_config_init();
}

// The RTOS task for nimble
static void nimble_host_task(void *param) {
  ESP_LOGI(TAG, "Nimble host task has been started");

  // Run the NimBLE stack
  nimble_port_run();

  vTaskDelete(NULL);
}

// Dummy task for heart rate
static void heart_rate_task(void *param) {
  ESP_LOGI(TAG, "heart rate task started!");
  while (1) {
    update_heart_rate();
    ESP_LOGI(TAG, "heart rate updated to %d", get_heart_rate());
    send_heart_rate_indication();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void app_main(void) {
  printf("Hello World!");

  // Return code
  int rc;

  // Initialize the NVS, which is used to store PHY data and possible Bluetooth
  // stuff
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialize the BLE stack (NimBLE)
  ret = nimble_port_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize nimble port");
    return;
  }

  // Initialize the GAP (Generic Access Profile)
  rc = gap_init();
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to initialize GAP, error code: %d", rc);
  }

  rc = kbd_gatt_svr_init();
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to initialize GATT, error code %d", rc);
  }

  // Configure NimBLE
  nimble_host_config_init();

  // Run it as a task
  xTaskCreate(nimble_host_task, "NimBLE Host", 4 * 1024, NULL, 5, NULL);
  xTaskCreate(heart_rate_task, "Heart Rate", 4 * 1024, NULL, 5, NULL);
  return;
}
