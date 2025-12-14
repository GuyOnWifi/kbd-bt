#include "gap.h"
#include "config.h"
#include "esp_log.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "host/ble_hs_adv.h"
#include "host/ble_hs_id.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

static uint8_t own_addr_type;
static uint8_t addr_val[6] = {0};
static uint8_t uri[] = {BLE_GAP_URI_PREFIX_HTTPS,
                        '/',
                        '/',
                        'e',
                        'a',
                        's',
                        'o',
                        'n',
                        'h',
                        'u',
                        'a',
                        'n',
                        'g',
                        '.',
                        'd',
                        'e',
                        'v'};

// Format the bluetooth address in a redable format
inline static void format_addr(char *addr_str, uint8_t addr[]) {
  sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2],
          addr[3], addr[4], addr[5]);
}

static int gap_event_handler(struct ble_gap_event *event, void *arg) {
  int rc = 0;
  struct ble_gap_conn_desc desc;

  switch (event->type) {

  case BLE_GAP_EVENT_CONNECT:
    // Two possibilities: a new connection or a failed connection
    ESP_LOGI(TAG, "Connection %s: stauts=%d",
             event->connect.status == 0 ? "established" : "failed",
             event->connect.status);

    if (event->connect.status == 0) {
      // Connection succeeded, check connection handle
      rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
      if (rc != 0) {
        ESP_LOGE(TAG, "failed to find connection by handle, error code: %d",
                 rc);
        return rc;
      }

      // print_conn_desc(&desc);
      // led_on();

      // Try to update connection parameters
      struct ble_gap_upd_params params = {.itvl_min = desc.conn_itvl,
                                          .itvl_max = desc.conn_itvl,
                                          .latency = 3,
                                          .supervision_timeout =
                                              desc.supervision_timeout};
      rc = ble_gap_update_params(event->connect.conn_handle, &params);
      if (rc != 0) {
        ESP_LOGE(TAG, "failed to update connection parameters, error code: %d",
                 rc);
        return rc;
      }
    } else {
      // Connection failed, restart advertising
      adv_start();
    }
    return rc;
    break;

  case BLE_GAP_EVENT_DISCONNECT:
    ESP_LOGI(TAG, "Disconnected, reason=%d", event->disconnect.reason);
    adv_start();
    break;

  case BLE_GAP_EVENT_CONN_UPDATE:
    ESP_LOGI(TAG, "Connection updated: status=%d", event->conn_update.status);

    rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
    if (rc != 0) {
      ESP_LOGE(TAG, "Failed to find conection by handle, error: %d", rc);
      return rc;
    }
  }
  return rc;
}

int adv_start() {
  int rc = 0;
  const char *name;
  struct ble_hs_adv_fields adv_fields = {0};
  struct ble_hs_adv_fields rsp_fields = {0};
  struct ble_gap_adv_params adv_params = {0};

  // Advertising flags (general discoverable & BR/EDR unsupported)
  adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

  name = ble_svc_gap_device_name();
  adv_fields.name = (uint8_t *)name;
  adv_fields.name_len = strlen(name);
  adv_fields.name_is_complete = 1;

  ESP_LOGI(TAG, "%s", name);

  // Set device TX
  adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
  adv_fields.tx_pwr_lvl_is_present = 1;

  // Set device apperance
  adv_fields.appearance = BLE_GAP_APPEARANCE_GENERIC_TAG;
  adv_fields.appearance_is_present = 1;

  // Set device LE role
  adv_fields.le_role = BLE_GAP_LE_ROLE_PERIPHERAL;
  adv_fields.le_role_is_present = 1;

  // Set advertisment fields
  rc = ble_gap_adv_set_fields(&adv_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set advertisment fields, error code: %d", rc);
    return rc;
  }

  // URI
  rsp_fields.uri = uri;
  rsp_fields.uri_len = sizeof(uri);

  // Intervals
  rsp_fields.adv_itvl = BLE_GAP_ADV_ITVL_MS(500);
  rsp_fields.adv_itvl_is_present = 1;

  rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set scan fields, error code: %d", rc);
    return rc;
  }

  // Set non-connetable and general discoverable mode to be a beacon
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                         gap_event_handler, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to start advertising, error code: %d", rc);
    return rc;
  }

  ESP_LOGI(TAG, "Advertising started!");
  return 0;
}

int adv_init() {
  int rc = 0;
  char addr_str[18] = {0};

  rc = ble_hs_util_ensure_addr(0);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to ensure address, error code: %d", rc);
    return rc;
  }

  rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to infer address type, error code: %d", rc);
    return rc;
  }

  rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to copy address, error code: %d", rc);
    return rc;
  }

  format_addr(addr_str, addr_val);
  ESP_LOGI(TAG, "address: %s", addr_str);

  adv_start();

  return 0;
}

int gap_init() {
  int rc = 0;

  // Call NimBLE GAP initialization API
  ble_svc_gap_init();

  // Set GAP device name
  rc = ble_svc_gap_device_name_set(DEVICE_NAME);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set device name to %s, error code: %d",
             DEVICE_NAME, rc);
    return rc;
  }

  ESP_LOGI(TAG, "Setting name to %s", DEVICE_NAME);

  // Sets the GAP apperaence
  // https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf
  rc = ble_svc_gap_device_appearance_set(BLE_GAP_APPEARANCE_GENERIC_TAG);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set device appearance, error code: %d", rc);
    return rc;
  }

  return rc;
}
