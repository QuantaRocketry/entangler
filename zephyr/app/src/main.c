#include <errno.h>
#include <zephyr/device.h>

#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/usb_device.h>

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS_OKAY(DEFAULT_RADIO_NODE),
             "No default LoRa radio specified in DT");

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_LOG_MAX_LEVEL);

#define MAX_DATA_LEN 10
char data[MAX_DATA_LEN] = {'V', 'K', '2', 'G', 'T', 'X'};

int main(void) {
  /* Enable Logging */
  const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
  uint32_t dtr = 0;

  if (usb_enable(NULL)) {
    return 0;
  }

  /* Poll if the DTR flag was set */
  while (!dtr) {
    uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
    // Give CPU resources to low priority threads.
    k_sleep(K_MSEC(100));
  }

  const struct device *const lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
  struct lora_modem_config config;
  int ret;

  if (!device_is_ready(lora_dev)) {
    LOG_ERR("%s Device not ready", lora_dev->name);
    return 0;
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
    return 0;
  }

  while (1) {
    ret = lora_send(lora_dev, data, MAX_DATA_LEN);
    if (ret < 0) {
      LOG_ERR("LoRa send failed");
      return 0;
    }

    LOG_INF("Data sent!");

    /* Send data at 10s interval */
    k_sleep(K_SECONDS(10));
  }
  return 0;
}
