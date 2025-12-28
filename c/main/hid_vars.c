#include <hid_vars.h>

const uint8_t HID_BOOT_REPORT_MAP[] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0, //   Usage Minimum (0xE0)
    0x29, 0xE7, //   Usage Maximum (0xE7)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x95, 0x08, //   Report Count (8)
    0x75, 0x01, //   Report Size (1)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
                //   Position)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x08, //   Report Size (8)
    0x81, 0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null
                //   Position)
    0x95, 0x06, //   Report Count (6)
    0x75, 0x08, //   Report Size (8)
    0x15, 0x00, //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x05, 0x07,       //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,       //   Usage Minimum (0x00)
    0x2A, 0xFF, 0x00, //   Usage Maximum (0xFF)
    0x81, 0x00, //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No
                //   Null Position)
    0x25, 0x01, //   Logical Maximum (1)
    0x95, 0x05, //   Report Count (5)
    0x75, 0x01, //   Report Size (1)
    0x05, 0x08, //   Usage Page (LEDs)
    0x19, 0x01, //   Usage Minimum (Num Lock)
    0x29, 0x05, //   Usage Maximum (Kana)
    0x91, 0x02, //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
                //   Position,Non-volatile)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x03, //   Report Size (3)
    0x91, 0x03, //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No
                //   Null Position,Non-volatile)
    0xC0,       // End Collection

    // 67 bytes
};

const size_t HID_BOOT_REPORT_MAP_LEN = sizeof(HID_BOOT_REPORT_MAP);

const uint8_t HID_COMPLEX_REPORT_MAP[] = {
    0x06, 0x01, 0x00, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x80,       // Usage (Sys Control)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report ID (1)
    0x19, 0x81,       //   Usage Minimum (Sys Power Down)
    0x29, 0x83,       //   Usage Maximum (Sys Wake Up)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x95, 0x03,       //   Report Count (3)
    0x75, 0x01,       //   Report Size (1)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
                //   Position)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x05, //   Report Size (5)
    0x81, 0x01, //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No
                //   Null Position)
    0xC0,       // End Collection
    0x05, 0x0C, // Usage Page (Consumer)
    0x09, 0x01, // Usage (Consumer Control)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x02, //   Report ID (2)
    0x19, 0x00, //   Usage Minimum (Unassigned)
    0x2A, 0xFF, 0x02, //   Usage Maximum (0x02FF)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x7F, //   Logical Maximum (32767)
    0x95, 0x01,       //   Report Count (1)
    0x75, 0x10,       //   Report Size (16)
    0x81, 0x00, //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No
                //   Null Position)
    0xC0,       // End Collection
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,       // Usage (0x01)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x03,       //   Report ID (3)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x09, 0x2F,       //   Usage (0x2F)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x03,       //   Report Count (3)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
                //   Position)
    0xC0,       // End Collection
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x04, //   Report ID (4)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0x04, //   Usage Minimum (0x04)
    0x29, 0x70, //   Usage Maximum (0x70)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x78, //   Report Count (120)
    0x81, 0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null
                //   Position)
    0xC0,       // End Collection
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,       // Usage (0x01)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x05,       //   Report ID (5)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x19, 0x01,       //   Usage Minimum (0x01)
    0x29, 0x02,       //   Usage Maximum (0x02)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x05,       //   Report Count (5)
    0xB1, 0x02, //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No
                //   Null Position,Non-volatile)
    0xC0,       // End Collection
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,       // Usage (0x01)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x06,       //   Report ID (6)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x19, 0x01,       //   Usage Minimum (0x01)
    0x29, 0x02,       //   Usage Maximum (0x02)
    0x75, 0x08,       //   Report Size (8)
    0x96, 0x07, 0x04, //   Report Count (1031)
    0xB1, 0x02, //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No
                //   Null Position,Non-volatile)
    0xC0,       // End Collection
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02, // Usage (Mouse)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x07, //   Report ID (7)
    0x09, 0x01, //   Usage (Pointer)
    0xA1, 0x00, //   Collection (Physical)
    0x05, 0x09, //     Usage Page (Button)
    0x15, 0x00, //     Logical Minimum (0)
    0x25, 0x01, //     Logical Maximum (1)
    0x19, 0x01, //     Usage Minimum (0x01)
    0x29, 0x05, //     Usage Maximum (0x05)
    0x75, 0x01, //     Report Size (1)
    0x95, 0x05, //     Report Count (5)
    0x81, 0x02, //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No
                //     Null Position)
    0x95, 0x03, //     Report Count (3)
    0x81, 0x01, //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No
                //     Null Position)
    0x05, 0x01, //     Usage Page (Generic Desktop Ctrls)
    0x16, 0x00, 0x80, //     Logical Minimum (-32768)
    0x26, 0xFF, 0x7F, //     Logical Maximum (32767)
    0x09, 0x30,       //     Usage (X)
    0x09, 0x31,       //     Usage (Y)
    0x75, 0x10,       //     Report Size (16)
    0x95, 0x02,       //     Report Count (2)
    0x81, 0x06, //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No
                //     Null Position)
    0x15, 0x81, //     Logical Minimum (-127)
    0x25, 0x7F, //     Logical Maximum (127)
    0x09, 0x38, //     Usage (Wheel)
    0x75, 0x08, //     Report Size (8)
    0x95, 0x01, //     Report Count (1)
    0x81, 0x06, //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No
                //     Null Position)
    0x05, 0x0C, //     Usage Page (Consumer)
    0x0A, 0x38, 0x02, //     Usage (AC Pan)
    0x95, 0x01,       //     Report Count (1)
    0x81, 0x06, //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No
                //     Null Position)
    0xC0,       //   End Collection
    0xC0,       // End Collection
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,       // Usage (0x01)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x08,       //   Report ID (8)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x09, 0x00,       //   Usage (0x00)
    0x75, 0x08,       //   Report Size (8)
    0x96, 0x7D, 0x01, //   Report Count (381)
    0xB1, 0x02, //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No
                //   Null Position,Non-volatile)
    0xC0,       // End Collection

    // 251 bytes
};

const size_t HID_COMPLEX_REPORT_MAP_LEN = sizeof(HID_COMPLEX_REPORT_MAP);
