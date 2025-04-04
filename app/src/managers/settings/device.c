#include "device.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>

#include <zephyr/settings/settings.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(settings_manager_device, CONFIG_SETTINGS_LOG_LEVEL);

K_SEM_DEFINE(device_settings_sem, 0, 1);

device_settings_t device_settings = DEFAULT_DEVICE;

#define SETTINGS_DEVICE_PATH        "device"
#define SETTINGS_DEVICE_NAME_KEY    SETTINGS_DEVICE_PATH "/name"
#define SETTINGS_DEVICE_TYPE_KEY    SETTINGS_DEVICE_PATH "/type"
#define SETTINGS_DEVICE_VERSION_KEY SETTINGS_DEVICE_PATH "/version"

static int device_handler_set(const char *key, size_t len_rd, settings_read_cb read_cb,
			      void *cb_arg)
{
	const char *next;
	int rc;

	if (settings_name_steq(key, "name", &next) && !next) {
		/* Handle string setting differently - make sure it fits in the buffer */
		if (len_rd >= sizeof(device_settings.name)) {
			/* Truncate if necessary */
			char temp_name[sizeof(device_settings.name)];
			rc = read_cb(cb_arg, temp_name, sizeof(temp_name) - 1);
			if (rc < 0) {
				return rc;
			}

			/* Ensure null-termination */
			temp_name[sizeof(temp_name) - 1] = '\0';
			strncpy(device_settings.name, temp_name, sizeof(device_settings.name));
		} else {
			/* Read directly if it fits */
			char temp_name[sizeof(device_settings.name)];
			rc = read_cb(cb_arg, temp_name, len_rd);
			if (rc < 0) {
				return rc;
			}

			/* Ensure null-termination */
			temp_name[rc] = '\0';
			strncpy(device_settings.name, temp_name, sizeof(device_settings.name));
		}

		return 0;
	}

	if (settings_name_steq(key, "type", &next) && !next) {
		/* Handle string setting differently - make sure it fits in the buffer */
		if (len_rd >= sizeof(device_settings.type)) {
			/* Truncate if necessary */
			char temp_name[sizeof(device_settings.type)];
			rc = read_cb(cb_arg, temp_name, sizeof(temp_name) - 1);
			if (rc < 0) {
				return rc;
			}

			/* Ensure null-termination */
			temp_name[sizeof(temp_name) - 1] = '\0';
			strncpy(device_settings.type, temp_name, sizeof(device_settings.type));
		} else {
			/* Read directly if it fits */
			char temp_name[sizeof(device_settings.type)];
			rc = read_cb(cb_arg, temp_name, len_rd);
			if (rc < 0) {
				return rc;
			}

			/* Ensure null-termination */
			temp_name[rc] = '\0';
			strncpy(device_settings.type, temp_name, sizeof(device_settings.type));
		}

		return 0;
	}

	if (settings_name_steq(key, "version", &next) && !next) {
		if (len_rd != sizeof(device_settings.version)) {
			return -EINVAL;
		}

		rc = read_cb(cb_arg, &device_settings.version, sizeof(device_settings.version));
		if (rc < 0) {
			return rc;
		}

		return 0;
	}

	return -ENOENT;
}

int device_handler_commit(void)
{
	LOG_INF("loading all settings under <device> handler is done");
	return 0;
}

static struct settings_handler device_settings_conf = {
	.name = SETTINGS_DEVICE_PATH,
	.h_set = device_handler_set,
	.h_commit = device_handler_commit,
};

int settings_init_device(void)
{
	int rc;

	/* Register the settings handler */
	rc = settings_register(&device_settings_conf);
	if (rc) {
		LOG_ERR("Failed to register device settings handler (err '%s')", strerror(-rc));
		k_sem_give(&device_settings_sem);
		return rc;
	}

	/* Load stored settings */
	rc = settings_load();
	if (rc) {
		LOG_ERR("Failed to load device settings (err '%s')", strerror(-rc));
		k_sem_give(&device_settings_sem);
		return rc;
	}

	k_sem_give(&device_settings_sem);
	return 0;
}

device_settings_t settings_get_device()
{
	k_sem_take(&device_settings_sem, K_FOREVER);
	device_settings_t ret = device_settings;
	k_sem_give(&device_settings_sem);

	return ret;
}

int settings_set_device(device_settings_t *settings)
{
	int rc;

	k_sem_take(&device_settings_sem, K_FOREVER);

	memcpy(&device_settings, settings, sizeof(device_settings_t));

	/* <device/name> */
	// strncpy(device_settings.name, name, sizeof(device_settings.name) - 1);
	device_settings.name[sizeof(device_settings.name) - 1] = '\0';

	rc = settings_save_one(SETTINGS_DEVICE_NAME_KEY, device_settings.name,
			       strlen(device_settings.name) + 1);
	if (rc) {
		LOG_ERR("Failed to save device/name setting (err '%s')", strerror(-rc));
		k_sem_give(&device_settings_sem);
		return rc;
	}

	/* <device/type> */
	// strncpy(device_settings.type, type, sizeof(device_settings.type) - 1);
	device_settings.type[sizeof(device_settings.type) - 1] = '\0';

	rc = settings_save_one(SETTINGS_DEVICE_TYPE_KEY, device_settings.type,
			       strlen(device_settings.type) + 1);
	if (rc) {
		LOG_ERR("Failed to save device/type setting (err '%s')", strerror(-rc));
		k_sem_give(&device_settings_sem);
		return rc;
	}

	/* <device/version> */
	rc = settings_save_one(SETTINGS_DEVICE_VERSION_KEY, &device_settings.version,
			       sizeof(&device_settings.version));
	if (rc) {
		LOG_ERR("Failed to save device/version setting (err '%s')", strerror(-rc));
		k_sem_give(&device_settings_sem);
		return rc;
	}

	k_sem_give(&device_settings_sem);

	return 0;
}

int settings_reset_device()
{
	int rc;

	k_sem_take(&device_settings_sem, K_FOREVER);
	device_settings = DEFAULT_DEVICE;
	k_sem_give(&device_settings_sem);

	rc = settings_set_device(&device_settings);
	if (rc) {
		LOG_ERR("Failed to reset device settings (err '%s')", strerror(-rc));
		return rc;
	}

	return 0;
}
