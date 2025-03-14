#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/usb/usb_device.h>

#include "shell/shell.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_LOG_MAX_LEVEL);

int main(void)
{
	int rc;

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

	rc = settings_subsys_init();
	if (rc) {
		LOG_ERR("settings subsys initialization: fail (err %d)\n", rc);
	}

	k_sleep(K_SECONDS(10));
	return 0;
}
