#include "gatt_svr.h"
#include "config.h"
#include "esp_log.h"
#include "heart_rate.h"
#include "host/ble_att.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "os/os_mbuf.h"
#include "services/gatt/ble_svc_gatt.h"
#include <stdint.h>

// Functions to access characteristic data
static int heart_rate_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg);

// Heart Rate service
static const ble_uuid16_t heart_rate_svc_uuid = BLE_UUID16_INIT(0x180D);

static uint8_t heart_rate_chr_val[2] = {0};
static uint16_t heart_rate_chr_val_handle; // Handle to the characteristic value
static const ble_uuid16_t heart_rate_chr_uuid = BLE_UUID16_INIT(0x2A37);

static uint16_t heart_rate_chr_conn_handle =
    0; // Stores the central device that's accessing it
static bool heart_rate_chr_conn_handle_inited = false;
static bool heart_rate_ind_status = false;

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    // Heart Rate Service
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &heart_rate_svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {.uuid = &heart_rate_chr_uuid.u,
                 .access_cb = heart_rate_chr_access,
                 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
                 .val_handle = &heart_rate_chr_val_handle},
                {0}},
    },
    {0}};

static int heart_rate_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg) {
  // Local variables
  int rc;

  // Handle access events (heart rate is read only)
  switch (ctxt->op) {

  // Read event
  case BLE_GATT_ACCESS_OP_READ_CHR:
    if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
      ESP_LOGI(TAG, "characteristic read; conn_handle=%d attr_handle=%d",
               conn_handle, attr_handle);
    } else {
      ESP_LOGI(TAG, "characteristic read by nimble stack; attr_handle=%d",
               attr_handle);
    }

    // Heart rate attribute data
    if (attr_handle == heart_rate_chr_val_handle) {
      // Update buffer value
      heart_rate_chr_val[1] = get_heart_rate();
      rc = os_mbuf_append(ctxt->om, &heart_rate_chr_val,
                          sizeof(heart_rate_chr_val));
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    goto error;
  default:
    goto error;
  }

error:
  ESP_LOGE(
      TAG,
      "unexpected access operation to heart rate characteristic, opcode: %d",
      ctxt->op);
  return BLE_ATT_ERR_UNLIKELY;
};

// The indication callback
void send_heart_rate_indication(void) {
  // Check if it is in indicate mode and if there is a connection too the CCCD
  if (heart_rate_ind_status && heart_rate_chr_conn_handle_inited) {
    ble_gatts_indicate(heart_rate_chr_conn_handle, heart_rate_chr_val_handle);
    ESP_LOGI(TAG, "heart rate indication sent!");
  }
}

// Handles GATT attribute register events: Service register event, characterstic
// regiseter, descriptor register. These occur when the BLE stack has
// initialized and loaded the service definitions
void kbd_gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
  char buf[BLE_UUID_STR_LEN];

  switch (ctxt->op) {
  // Service register event
  case BLE_GATT_REGISTER_OP_SVC:
    ESP_LOGD(TAG, "registered service %s with handle=%d",
             ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf), ctxt->svc.handle);
    break;

  // Characteristic register event
  case BLE_GATT_REGISTER_OP_CHR:
    ESP_LOGD(TAG,
             "registering characteristic %s with "
             "def_handle=%d val_handle=%d",
             ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
             ctxt->chr.def_handle, ctxt->chr.val_handle);
    break;

  // Descriptor register event
  case BLE_GATT_REGISTER_OP_DSC:
    ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
             ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf), ctxt->dsc.handle);
    break;

  // Unknown event. Crash, should never happen
  default:
    assert(0);
    break;
  }
}

// GATT server subscribe event callback
void kbd_gatt_svr_subscribe_cb(struct ble_gap_event *event) {
  // Check for connection
  if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
    ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
             event->subscribe.conn_handle, event->subscribe.attr_handle);
  } else {
    // Usually to restore connection from previously bonded
    ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
             event->subscribe.attr_handle);
  }

  // Check the ATT handle
  if (event->subscribe.attr_handle == heart_rate_chr_val_handle) {
    heart_rate_chr_conn_handle = event->subscribe.conn_handle;
    heart_rate_chr_conn_handle_inited = true;
    heart_rate_ind_status = event->subscribe.cur_indicate;
  }
}

int kbd_gatt_svr_init() {
  int rc;

  // Initialize GATT services
  ble_svc_gatt_init();

  // Setup the memory, etc needed to accomodate
  rc = ble_gatts_count_cfg(gatt_svr_svcs);
  if (rc != 0) {
    return rc;
  }

  // Add the Services
  rc = ble_gatts_add_svcs(gatt_svr_svcs);
  if (rc != 0) {
    return rc;
  }

  return 0;
}
