#include "hogp_gatt_svr.h"
#include "config.h"
#include "esp_log.h"
#include "hid_vars.h"
#include "host/ble_att.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "os/os_mbuf.h"
#include "services/gatt/ble_svc_gatt.h"
#include <stdint.h>

#define HID_SVC_UUID 0x1812
#define HID_INFO_CHR_UUID 0x2A4A
#define REPORT_MAP_CHR_UUID 0x2A4B
#define HID_CTRL_POINT_CHR_UUID 0x2A4C
#define REPORT_CHR_UUID 0x2A4D
#define PRTCL_MODE_CHR_UUID 0x2A4E
#define BOOT_KBD_INP_REPORT_CHR_UUID 0x2A22
#define BOOT_KBD_OUTP_REPORT_CHR_UUID 0x2A33
#define REPORT_REFERENCE_DSC_UUID 0x2908

enum {
  HID_INFO_ATTR,
  REPORT_MAP_ATTR,
  HID_CTRL_POINT_ATTR,
  REPORT_ATTR,
  PRTCL_MODE_ATTR,
  BOOT_KBD_INP_REPORT_ATTR,
  BOOT_KBD_OUTP_REPORT_ATTR,
  HID_IDX_COUNT
};

enum {
  REPORT_CONN_STATUS,
  BOOT_KBD_CONN_STATUS,
  CONN_STATUS_COUNT,
};

typedef struct {
  uint8_t notify : 1;
  uint8_t indicate : 1;
  uint8_t reserved : 6;
} cccd_subscription_state_t;

// Callback functions for access
static int hid_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);

static uint16_t hogp_svr_handles[HID_IDX_COUNT];

static uint16_t hogp_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static cccd_subscription_state_t hogp_subscription_states[CONN_STATUS_COUNT];

uint8_t last_report[8] = {
    0x00,       // Byte 0: Modifiers (e.g., 0x02 for Left Shift)
    0x00,       // Byte 1: Reserved (Always 0)
    0x00, 0x00, // Byte 2-3: First two keys
    0x00, 0x00, // Byte 4-5: Next two keys
    0x00, 0x00  // Byte 6-7: Last two keys
};

static const struct ble_gatt_svc_def hogp_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(HID_SVC_UUID),
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {.uuid = BLE_UUID16_DECLARE(HID_INFO_CHR_UUID),
                 .access_cb = hid_svr_chr_access,
                 .flags = BLE_GATT_CHR_F_READ,
                 .val_handle = &hogp_svr_handles[HID_INFO_ATTR]},

                {.uuid = BLE_UUID16_DECLARE(REPORT_MAP_CHR_UUID),
                 .access_cb = hid_svr_chr_access,
                 .flags = BLE_GATT_CHR_F_READ,
                 .val_handle = &hogp_svr_handles[REPORT_MAP_ATTR]},

                {.uuid = BLE_UUID16_DECLARE(HID_CTRL_POINT_CHR_UUID),
                 .access_cb = hid_svr_chr_access,
                 .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
                 .val_handle = &hogp_svr_handles[HID_CTRL_POINT_ATTR]},

                {.uuid = BLE_UUID16_DECLARE(REPORT_CHR_UUID),
                 .access_cb = hid_svr_chr_access,
                 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                          BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_NOTIFY |
                          BLE_GATT_CHR_F_INDICATE,
                 .val_handle = &hogp_svr_handles[REPORT_ATTR],
                 .descriptors =
                     (struct ble_gatt_dsc_def[]){
                         {
                             .uuid =
                                 BLE_UUID16_DECLARE(REPORT_REFERENCE_DSC_UUID),
                             .att_flags = BLE_ATT_F_READ,
                             .access_cb = hid_svr_chr_access,
                         },
                         {0}}},

                {.uuid = BLE_UUID16_DECLARE(PRTCL_MODE_CHR_UUID),
                 .access_cb = hid_svr_chr_access,
                 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
                 .val_handle = &hogp_svr_handles[PRTCL_MODE_ATTR]},

                {.uuid = BLE_UUID16_DECLARE(BOOT_KBD_INP_REPORT_CHR_UUID),
                 .access_cb = hid_svr_chr_access,
                 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                 .val_handle = &hogp_svr_handles[BOOT_KBD_INP_REPORT_ATTR]},

                {.uuid = BLE_UUID16_DECLARE(BOOT_KBD_OUTP_REPORT_CHR_UUID),
                 .access_cb = hid_svr_chr_access,
                 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                          BLE_GATT_CHR_F_WRITE_NO_RSP,
                 .val_handle = &hogp_svr_handles[BOOT_KBD_OUTP_REPORT_ATTR]},

                {0} /* No more characteristics */},
    },
    {0} /* No more services */
};

static int hid_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg) {
  // Local variables
  int rc;
  uint16_t uuid16 = ble_uuid_u16(ctxt->chr->uuid);

  ESP_LOGI(TAG, "GATT ACESSS, ACCESS TYPE: %d, ATTR: %d, UUID: %d", ctxt->op,
           attr_handle, uuid16);

  if (uuid16 == REPORT_REFERENCE_DSC_UUID) {
    const uint8_t rpt_ref_data[] = {0x00, 0x01};
    rc = os_mbuf_append(ctxt->om, rpt_ref_data, sizeof(rpt_ref_data));
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  if (attr_handle == hogp_svr_handles[HID_INFO_ATTR]) {
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) {
      goto error;
    }

    const uint8_t hid_info_data[] = {
        0x11, 0x01, // bcdHID (1.11)
        0x00,       // bCountryCode (universal)
        0x02        // Flags (Normally connectable)
    };
    rc = os_mbuf_append(ctxt->om, hid_info_data, sizeof(hid_info_data));
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  } else if (attr_handle == hogp_svr_handles[REPORT_MAP_ATTR]) {
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) {
      goto error;
    }

    rc = os_mbuf_append(ctxt->om, HID_BOOT_REPORT_MAP, HID_BOOT_REPORT_MAP_LEN);
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  } else if (attr_handle == hogp_svr_handles[HID_CTRL_POINT_ATTR]) {
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
      goto error;
    }

    ESP_LOGW(TAG, "HID Control point not yet implemented");
    return 0;
  } else if (attr_handle == hogp_svr_handles[REPORT_ATTR]) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
      rc = os_mbuf_append(ctxt->om, last_report, sizeof(last_report));
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      ESP_LOGW(TAG, "HID Output features not supported! ");
      return 0;
    } else {
      goto error;
    }
  } else if (attr_handle == hogp_svr_handles[PRTCL_MODE_ATTR]) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
      const uint8_t REPORT_PRTCL_MODE = 0x01;
      rc = os_mbuf_append(ctxt->om, &REPORT_PRTCL_MODE,
                          sizeof(REPORT_PRTCL_MODE));
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      ESP_LOGW(TAG, "SWITCHING TO BOOT MODE NOT YET SUPPORTED");
    }
  } else if (attr_handle == hogp_svr_handles[BOOT_KBD_INP_REPORT_ATTR]) {
    ESP_LOGW(TAG, "BOOT KBD NOT YET IMPLEMENTED");
  } else if (attr_handle == hogp_svr_handles[BOOT_KBD_OUTP_REPORT_ATTR]) {
    ESP_LOGW(TAG, "BOOT KBD NOT YET IMPLEMENTED");
  }

error:
  ESP_LOGE(TAG,
           "unexpected access operation to hogp characteristic, (could be not "
           "supported)"
           "opcode: %d",
           ctxt->op);
  return BLE_ATT_ERR_UNLIKELY;
}

void send_keyboard_input_notify(uint8_t key) {
  // TODO: Make the boot keyboard mode also work

  ESP_LOGI(TAG, "Notifications on: %d, Conn_hadnle = %d",
           hogp_subscription_states[REPORT_CONN_STATUS].notify,
           hogp_conn_handle);

  if (hogp_subscription_states[REPORT_CONN_STATUS].notify &&
      hogp_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
    ESP_LOGI(TAG, "SENDING A KEY");
    last_report[2] = key; // thats probably the a key
    ble_gatts_notify(hogp_conn_handle, hogp_svr_handles[REPORT_ATTR]);
  }
}

// Handles GATT attribute register events: Service register event,
// characterstic regiseter, descriptor register. These occur when the BLE
// stack has initialized and loaded the service definitions
void hogp_gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
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
void hogp_gatt_svr_subscribe_cb(struct ble_gap_event *event) {
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
  if (event->subscribe.attr_handle == hogp_svr_handles[REPORT_ATTR]) {
    hogp_conn_handle = event->subscribe.conn_handle;
    hogp_subscription_states[REPORT_CONN_STATUS].notify =
        event->subscribe.cur_notify;
    hogp_subscription_states[REPORT_CONN_STATUS].indicate =
        event->subscribe.cur_indicate;
  } else if (event->subscribe.attr_handle ==
             hogp_svr_handles[BOOT_KBD_INP_REPORT_ATTR]) {
    hogp_conn_handle = event->subscribe.conn_handle;
    hogp_subscription_states[BOOT_KBD_CONN_STATUS].notify =
        event->subscribe.cur_notify;
    hogp_subscription_states[BOOT_KBD_CONN_STATUS].indicate =
        event->subscribe.cur_indicate;
  }
}

int hogp_gatt_svr_init() {
  int rc;

  // Initialize GATT services
  ble_svc_gatt_init();

  // Setup the memory, etc needed to accomodate
  rc = ble_gatts_count_cfg(hogp_svcs);
  if (rc != 0) {
    return rc;
  }

  // Add the Services
  rc = ble_gatts_add_svcs(hogp_svcs);
  if (rc != 0) {
    return rc;
  }

  return 0;
}
