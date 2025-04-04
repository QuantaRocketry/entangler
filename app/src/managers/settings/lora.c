#include "lora.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <zephyr/settings/settings.h>
#include <zephyr/sys/printk.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(settings_manager_lora, CONFIG_SETTINGS_LOG_LEVEL);

K_SEM_DEFINE(test_sem, 0, 1);
K_SEM_DEFINE(lora_settings_sem, 0, 1);

#define DEFAULT_LORA                                                                               \
	(lora_settings_t){.freq = 915000000,                                                       \
			  .bandwidth = 62500,                                                      \
			  .spreading_factor = 7,                                                   \
			  .sync_word = 0x00,                                                       \
			  .coding_rate = 5}

lora_settings_t lora_settings = DEFAULT_LORA;

#define SETTINGS_LORA_PATH          "lora"
#define SETTINGS_LORA_FREQ_KEY      SETTINGS_LORA_PATH "/freq"
#define SETTINGS_LORA_BANDWIDTH_KEY SETTINGS_LORA_PATH "/bandwidth"
#define SETTINGS_LORA_SF_KEY        SETTINGS_LORA_PATH "/sf"
#define SETTINGS_LORA_SYNC_KEY      SETTINGS_LORA_PATH "/sync"
#define SETTINGS_LORA_CR_KEY        SETTINGS_LORA_PATH "/cr"

static int lora_handler_set(const char *key, size_t len_rd, settings_read_cb read_cb, void *cb_arg)
{
	const char *next;
	int rc;

	if (settings_name_steq(key, "freq", &next) && !next) {
		if (len_rd != sizeof(lora_settings.freq)) {
			return -EINVAL;
		}

		rc = read_cb(cb_arg, &lora_settings.freq, sizeof(lora_settings.freq));
		if (rc < 0) {
			return rc;
		}

		return 0;
	}

	if (settings_name_steq(key, "bandwidth", &next) && !next) {
		if (len_rd != sizeof(lora_settings.bandwidth)) {
			return -EINVAL;
		}

		rc = read_cb(cb_arg, &lora_settings.bandwidth, sizeof(lora_settings.bandwidth));
		if (rc < 0) {
			return rc;
		}

		return 0;
	}

	if (settings_name_steq(key, "sf", &next) && !next) {
		if (len_rd != sizeof(lora_settings.spreading_factor)) {
			return -EINVAL;
		}

		rc = read_cb(cb_arg, &lora_settings.spreading_factor,
			     sizeof(lora_settings.spreading_factor));
		if (rc < 0) {
			return rc;
		}

		/* Validate spreading factor */
		if (lora_settings.spreading_factor < 7 || lora_settings.spreading_factor > 12) {
			lora_settings.spreading_factor = 7; /* Reset to default if invalid */
		}

		return 0;
	}

	if (settings_name_steq(key, "sw", &next) && !next) {
		if (len_rd != sizeof(lora_settings.sync_word)) {
			return -EINVAL;
		}

		rc = read_cb(cb_arg, &lora_settings.sync_word, sizeof(lora_settings.sync_word));
		if (rc < 0) {
			return rc;
		}

		return 0;
	}

	if (settings_name_steq(key, "cr", &next) && !next) {
		if (len_rd != sizeof(lora_settings.coding_rate)) {
			return -EINVAL;
		}

		rc = read_cb(cb_arg, &lora_settings.coding_rate, sizeof(lora_settings.coding_rate));
		if (rc < 0) {
			return rc;
		}

		/* Validate coding rate */
		if (lora_settings.coding_rate < 5 || lora_settings.coding_rate > 8) {
			lora_settings.coding_rate = 5; /* Reset to default if invalid */
		}

		return 0;
	}

	return -ENOENT;
}

static int lora_handler_get(const char *name, char *val, int val_len_max)
{
	if (strcmp(name, SETTINGS_LORA_FREQ_KEY) == 0) {
		return snprintf(val, val_len_max, "%d", lora_settings.freq);
	} else if (strcmp(name, SETTINGS_LORA_BANDWIDTH_KEY) == 0) {
		return snprintf(val, val_len_max, "%d", lora_settings.bandwidth);
	} else if (strcmp(name, SETTINGS_LORA_SF_KEY) == 0) {
		return snprintf(val, val_len_max, "%d", lora_settings.spreading_factor);
	} else if (strcmp(name, SETTINGS_LORA_SYNC_KEY) == 0) {
		return snprintf(val, val_len_max, "0x%08X", lora_settings.sync_word);
	} else if (strcmp(name, SETTINGS_LORA_CR_KEY) == 0) {
		return snprintf(val, val_len_max, "%d", lora_settings.coding_rate);
	}
	return -ENOENT;
}

int lora_handler_commit(void)
{
	LOG_INF("loading all settings under <lora> handler is done");
	return 0;
}

static struct settings_handler lora_settings_conf = {
	.name = SETTINGS_LORA_PATH,
	.h_get = lora_handler_get,
	.h_set = lora_handler_set,
	.h_commit = lora_handler_commit,
};

int settings_init_lora(void)
{
	int rc;

	rc = settings_register(&lora_settings_conf);
	if (rc) {
		LOG_ERR("Failed to register LoRa settings handler (err %d)", rc);
		return rc;
	}

	rc = settings_load();
	if (rc) {
		LOG_ERR("Failed to load LoRa settings (err %d)", rc);
		return rc;
	}

	k_sem_give(&lora_settings_sem);
	return 0;
}

lora_settings_t settings_get_lora()
{
	k_sem_take(&lora_settings_sem, K_FOREVER);
	lora_settings_t ret = lora_settings;
	k_sem_give(&lora_settings_sem);

	return ret;
}

int settings_set_lora(lora_settings_t *settings)
{
	int rc;

	k_sem_take(&lora_settings_sem, K_FOREVER);

	memcpy(&lora_settings, settings, sizeof(lora_settings_t));

	rc = settings_save_one(SETTINGS_LORA_FREQ_KEY, &lora_settings.freq,
			       sizeof(lora_settings.freq));
	if (rc) {
		LOG_ERR("Failed to save frequency setting (err %d)", rc);
		return rc;
	}

	rc = settings_save_one(SETTINGS_LORA_BANDWIDTH_KEY, &lora_settings.bandwidth,
			       sizeof(lora_settings.bandwidth));
	if (rc) {
		LOG_ERR("Failed to save bandwidth setting (err %d)", rc);
		return rc;
	}

	rc = settings_save_one(SETTINGS_LORA_SF_KEY, &lora_settings.spreading_factor,
			       sizeof(lora_settings.spreading_factor));
	if (rc) {
		LOG_ERR("Failed to save spreading factor setting (err %d)", rc);
		return rc;
	}

	rc = settings_save_one(SETTINGS_LORA_SYNC_KEY, &lora_settings.sync_word,
			       sizeof(lora_settings.sync_word));
	if (rc) {
		LOG_ERR("Failed to save sync word setting (err %d)", rc);
		return rc;
	}

	rc = settings_save_one(SETTINGS_LORA_CR_KEY, &lora_settings.coding_rate,
			       sizeof(lora_settings.coding_rate));
	if (rc) {
		LOG_ERR("Failed to save coding_rate setting (err %d)", rc);
		return rc;
	}

	k_sem_give(&lora_settings_sem);

	return 0;
}

int settings_reset_lora()
{
	int rc;

	k_sem_take(&lora_settings_sem, K_FOREVER);
	lora_settings = DEFAULT_LORA;
	k_sem_give(&lora_settings_sem);

	rc = settings_set_lora(&lora_settings);
	if (rc) {
		LOG_ERR("Failed to reset lora settings (err %d)", rc);
		return rc;
	}

	return 0;
}
