#pragma once

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <zephyr/settings/settings.h>

typedef struct {
	int freq;             // The frequency in Hz
	int bandwidth;        // The bandwidth in Hz
	int spreading_factor; // The Spreading factor ( 7-12 )
	uint32_t sync_word;   // The 4 byte sync word ( 0x00 - 0xFF )
	int coding_rate;      // The coding rate of 4 ( 4/(5-8) )
} lora_settings_t;

int settings_init_lora(void);
lora_settings_t settings_get_lora(void);
int settings_set_lora(lora_settings_t *settings);
int settings_reset_lora(void);
