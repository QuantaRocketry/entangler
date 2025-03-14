#include "indicator_manager.h"

#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/usb_device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(indicator, CONFIG_LOG_MAX_LEVEL);

#define STRIP_NODE DT_ALIAS(led_strip)

#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define DELAY_TIME K_MSEC(500)

#define RGB(_r, _g, _b) {.r = (_r), .g = (_g), .b = (_b)}

static const struct led_rgb colors[] = {
	RGB(0x0f, 0x00, 0x00), /* red */
	RGB(0x00, 0x0f, 0x00), /* green */
	RGB(0x00, 0x00, 0x0f), /* blue */
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

void indicator_thread(void *p1, void *p2, void *p3)
{
	LOG_DBG("Starting neopixel thread");

	size_t color = 0;
	int rc;

	if (device_is_ready(strip)) {
		LOG_DBG("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		k_thread_abort(k_current_get());
	}

	LOG_DBG("Displaying pattern on strip");
	while (1) {
		for (size_t cursor = 0; cursor < ARRAY_SIZE(pixels); cursor++) {
			memset(&pixels, 0x00, sizeof(pixels));
			memcpy(&pixels[cursor], &colors[color], sizeof(struct led_rgb));

			rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
			if (rc) {
				LOG_ERR("couldn't update strip: %d", rc);
				k_thread_abort(k_current_get());
			}

			k_sleep(DELAY_TIME);
		}

		color = (color + 1) % ARRAY_SIZE(colors);
	}
}

K_THREAD_DEFINE(indicator_thread_id, 1024, indicator_thread, NULL, NULL, NULL, 5, 0, 0);
