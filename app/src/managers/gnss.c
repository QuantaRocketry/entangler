#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gnss.h>

#include "../managers/lora.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(gnss, CONFIG_GNSS_LOG_LEVEL);

#define GNSS_NODE DT_ALIAS(gnss)
#if DT_NODE_HAS_STATUS_OKAY(GNSS_NODE)

#define GNSS_MODEM DEVICE_DT_GET(GNSS_NODE)

struct gnss_data_store {
};

static void gnss_data_cb(const struct device *dev, const struct gnss_data *data)
{
	uint64_t timepulse_ns;
	k_ticks_t timepulse;

	if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
		if (gnss_get_latest_timepulse(dev, &timepulse) == 0) {
			timepulse_ns = k_ticks_to_ns_near64(timepulse);
			LOG_INF("Got a fix @ %lld ns\n", timepulse_ns);
		} else {
			LOG_INF("Got a fix!\n");
		}
	}
}
GNSS_DATA_CALLBACK_DEFINE(GNSS_MODEM, gnss_data_cb);

#define GNSS_SYSTEMS_LOG_INF(define, supported, enabled)                                           \
	printf("\t%20s: Supported: %3s Enabled: %3s\n",                                            \
	       STRINGIFY(define), (supported & define) ? "Yes" : "No",                             \
			 (enabled & define) ? "Yes" : "No");

void gnss_init(void *p1, void *p2, void *p3)
{
	gnss_systems_t supported, enabled;
	uint32_t fix_interval;
	int rc;

	rc = gnss_get_supported_systems(GNSS_MODEM, &supported);
	if (rc < 0) {
		LOG_ERR("Failed to query supported systems (%d)", rc);
		k_thread_abort(k_current_get());
		return;
	}
	rc = gnss_get_enabled_systems(GNSS_MODEM, &enabled);
	if (rc < 0) {
		LOG_ERR("Failed to query enabled systems (%d)", rc);
		k_thread_abort(k_current_get());
		return;
	}
	LOG_INF("GNSS Systems:");
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_GPS, supported, enabled);
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_GLONASS, supported, enabled);
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_GALILEO, supported, enabled);
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_BEIDOU, supported, enabled);
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_QZSS, supported, enabled);
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_IRNSS, supported, enabled);
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_SBAS, supported, enabled);
	GNSS_SYSTEMS_LOG_INF(GNSS_SYSTEM_IMES, supported, enabled);

	rc = gnss_get_fix_rate(GNSS_MODEM, &fix_interval);
	if (rc < 0) {
		LOG_ERR("Failed to query fix rate (%d)", rc);
		k_thread_abort(k_current_get());
		return;
	}
	k_thread_abort(k_current_get());
}

K_THREAD_DEFINE(gnss_init_id, 1024, gnss_init, NULL, NULL, NULL, 5, 0, 0);

#else
#pragma message("No GNSS node defined, skipping GNSS driver.")
#endif // DT_NODE_HAS_STATUS_OKAY(GNSS_MODEM)
