#ifndef GATT_SVR_H
#define GATT_SVR_H

#include "host/ble_gatt.h"

// GAP APIs for suscribe / indicate events
#include "host/ble_gap.h"

void send_heart_rate_indication(void);
void kbd_gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
void kbd_gatt_svr_subscribe_cb(struct ble_gap_event *event);
int kbd_gatt_svr_init(void);

#endif // pragma once
