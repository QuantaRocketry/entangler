#pragma once
#include <inttypes.h>
#include <stdbool.h>

#define SETTINGS_PATH "settings"

typedef struct {
	uint32_t freq;
	bool tx_enable;
	char callsign[6];
	bool heartbeat_enable;
	uint8_t hearbeat_frequency;
} settings_lora_t;
#define SETTINGS_KEY_LORA "lora"
#define SETTINGS_LORA     (SETTINGS_PATH "/" SETTINGS_KEY_LORA)
