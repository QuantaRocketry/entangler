#include "lora.h"

#include <zephyr/kernel.h>

#include "../managers/settings.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lora, CONFIG_LOG_DEFAULT_LEVEL);

#define RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(RADIO_NODE), "No default LoRa radio specified in DT");
#define LORA_MODEM DEVICE_DT_GET(RADIO_NODE);

#define MAX_CALLSIGN_LEN 10
char callsign[MAX_CALLSIGN_LEN] = {'V', 'K', '2', 'G', 'T', 'X'};

enum lora_signal_bandwidth parse_bandwidth(int bw)
{
	if (bw == 125000) {
		return BW_125_KHZ;
	}
	if (bw == 250000) {
		return BW_250_KHZ;
	}
	if (bw == 500000) {
		return BW_500_KHZ;
	}
	return BW_125_KHZ;
};

enum lora_coding_rate parse_cr(int cr)
{
	int parsed = cr - 4;
	if (parsed > 0 && parsed <= 4) {
		return parsed;
	}
	return CR_4_5;
}

void lora_thread(void *p1, void *p2, void *p3)
{
	const struct device *const lora_dev = LORA_MODEM;
	struct lora_modem_config config;
	int rc;

	if (!device_is_ready(lora_dev)) {
		LOG_ERR("%s device not ready", lora_dev->name);
		k_thread_abort(k_current_get());
		return;
	}

	// Wait for settings for 30 sec before failing
	rc = await_settings_init(K_SECONDS(30));
	if (rc) {
		LOG_ERR("Failed to load lora settings: %d", rc);
		k_thread_abort(k_current_get());
		return;
	}

	lora_settings_t settings = settings_get_lora();

	config.frequency = settings.freq;
	config.bandwidth = parse_bandwidth(settings.bandwidth);
	config.datarate = settings.spreading_factor;
	config.preamble_len = 8;
	config.coding_rate = parse_cr(settings.coding_rate);
	config.iq_inverted = false;
	config.public_network = false;
	config.tx_power = 4;
	config.tx = true;

	rc = lora_config(lora_dev, &config);
	if (rc < 0) {
		LOG_ERR("LoRa config failed");
		k_thread_abort(k_current_get());
		return;
	}

	while (1) {
		rc = lora_send(lora_dev, callsign, MAX_CALLSIGN_LEN);
		if (rc < 0) {
			LOG_ERR("LoRa send failed");
			k_thread_abort(k_current_get());
			return;
		}

		LOG_DBG("Heartbeat sent: %s", callsign);

		/* Send data at 10s interval */
		k_sleep(K_SECONDS(10));
	}
};
K_THREAD_DEFINE(lora_thread_id, 1024, lora_thread, NULL, NULL, NULL, 5, 0, 0);
