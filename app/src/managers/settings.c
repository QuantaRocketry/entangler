#include "settings.h"
#include "settings/device.h"
#include "settings/lora.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <zephyr/settings/settings.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(settings_manager, CONFIG_SETTINGS_LOG_LEVEL);

K_SEM_DEFINE(settings_sem, 0, 1);

int init_settings(void)
{
	int i;

	/* settings initialization */
	int rc = settings_subsys_init();
	if (rc) {
		LOG_ERR("settings subsys FAIL: (err '%s')", strerror(-rc));
		return rc;
	}
	LOG_INF("settings subsys: OK.");

	rc = settings_init_device();
	if (rc) {
		LOG_ERR("subtree <device> FAIL: (err '%s')", strerror(-rc));
		return rc;
	}
	LOG_INF("subtree <device>: OK");
	
	rc = settings_init_lora();
	if (rc) {
		LOG_ERR("subtree <lora> FAIL: (err '%s')", strerror(-rc));
		return rc;
	}
	LOG_INF("subtree <lora>: OK");

	settings_load();
	settings_save();
	settings_commit();

	k_sem_give(&settings_sem);

	return 0;
}

int await_settings_init(k_timeout_t timeout)
{
	int rc = k_sem_take(&settings_sem, timeout);

	if (rc == 0) {
		k_sem_give(&settings_sem);
	}

	return 0;
}

SYS_INIT(init_settings, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);