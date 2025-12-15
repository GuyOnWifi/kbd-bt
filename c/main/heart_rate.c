
/* Includes */
#include "heart_rate.h"
#include "common.h"

/* Private variables */
static uint8_t heart_rate;

/* Public functions */
uint8_t get_heart_rate(void) { return heart_rate; }

void update_heart_rate(void) { heart_rate = 60 + (uint8_t)(esp_random() % 21); }
