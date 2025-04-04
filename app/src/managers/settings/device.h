#pragma once

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <zephyr/settings/settings.h>

typedef struct {
	char name[16]; // The name of the device
	char type[16]; // The type of device
	int version;   // The device version
} device_settings_t;

#ifndef DEVICE_VERSION
#define DEVICE_VERSION 0
#endif // DEVICE_VERSION

#define DEFAULT_DEVICE (device_settings_t){.name = "Entangler", .type = "ENTANGLER", .version = DEVICE_VERSION}

int settings_init_device();
device_settings_t settings_get_device();
int settings_set_device(device_settings_t *settings);
int settings_reset_device();
