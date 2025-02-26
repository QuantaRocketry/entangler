#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lora, CONFIG_LOG_DEFAULT_LEVEL);

#include "lora.h"

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(DEFAULT_RADIO_NODE),
             "No default LoRa radio specified in DT");

#define MAX_DATA_LEN 10
char data[MAX_DATA_LEN] = {'V', 'K', '2', 'G', 'T', 'X'};
void lora_thread(void *p1, void *p2, void *p3) {

  // Setup LoRa
  const struct device *const lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
  struct lora_modem_config config;
  int ret;

  if (!device_is_ready(lora_dev)) {
    LOG_ERR("%s Device not ready", lora_dev->name);
    k_thread_abort(k_current_get());
  }

  config.frequency = 915000000;
  config.bandwidth = BW_125_KHZ;
  config.datarate = SF_7;
  config.preamble_len = 8;
  config.coding_rate = CR_4_5;
  config.iq_inverted = false;
  config.public_network = false;
  config.tx_power = 4;
  config.tx = true;

  ret = lora_config(lora_dev, &config);
  if (ret < 0) {
    LOG_ERR("LoRa config failed");
    k_thread_abort(k_current_get());
  }

  while (1) {
    ret = lora_send(lora_dev, data, MAX_DATA_LEN);
    if (ret < 0) {
      LOG_ERR("LoRa send failed");
      k_thread_abort(k_current_get());
    }

    LOG_DBG("Data sent: %s", data);

    /* Send data at 10s interval */
    k_sleep(K_SECONDS(10));
  }
};
K_THREAD_DEFINE(lora_thread_id, 1024, lora_thread, NULL, NULL, NULL, 5, 0, 0);
