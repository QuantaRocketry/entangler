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

#define MAX_DATA_LEN 255

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(logs);

// LoRa Callback
void lora_receive_cb(const struct device *dev, uint8_t *data, uint16_t size,
                     int16_t rssi, int8_t snr, void *user_data) {
  static int cnt;

  ARG_UNUSED(dev);
  ARG_UNUSED(size);
  ARG_UNUSED(user_data);

  LOG_INF("LoRa RX RSSI: %d dBm, SNR: %d dB", rssi, snr);
  LOG_HEXDUMP_INF(data, size, "LoRa RX payload");

  /* Stop receiving after 10 packets */
  if (++cnt == 10) {
    LOG_INF("Stopping packet receptions");
    lora_recv_async(dev, NULL, NULL);
  }
}

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

  // k_sleep(K_SECONDS(1));

  /* Enable LoRa */
  const struct device *const lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);
  struct lora_modem_config config;
  int ret, len;
  uint8_t data[MAX_DATA_LEN] = {0};
  int16_t rssi;
  int8_t snr;

  if (!device_is_ready(lora_dev)) {
    LOG_ERR("%s Device not ready", lora_dev->name);
    return 0;
  }

  config.frequency = 865100000;
  config.bandwidth = BW_125_KHZ;
  config.datarate = SF_10;
  config.preamble_len = 8;
  config.coding_rate = CR_4_5;
  config.iq_inverted = false;
  config.public_network = false;
  config.tx_power = 14;
  config.tx = false;

  ret = lora_config(lora_dev, &config);
  if (ret < 0) {
    LOG_ERR("LoRa config failed, exiting...");
    return 0;
  }

  /* Receive 4 packets synchronously */
  LOG_INF("Synchronous reception");
  for (int i = 0; i < 4; i++) {
    /* Block until data arrives */
    len = lora_recv(lora_dev, data, MAX_DATA_LEN, K_SECONDS(5), &rssi, &snr);
    if (len < 0) {
      LOG_WRN("LoRa synchronous receive timed out / failed");
      break;
    }

    LOG_INF("LoRa RX RSSI: %d dBm, SNR: %d dB", rssi, snr);
    LOG_HEXDUMP_INF(data, len, "LoRa RX payload");
  }

  /* Enable asynchronous reception */
  LOG_INF("Starting Asynchronous reception");
  lora_recv_async(lora_dev, lora_receive_cb, NULL);
  k_sleep(K_FOREVER);
  return 0;
}
