#pragma once
#include <zephyr/kernel.h>

#include "settings/device.h"
#include "settings/lora.h"

int init_settings(void);
int await_settings_init(k_timeout_t timeout);
