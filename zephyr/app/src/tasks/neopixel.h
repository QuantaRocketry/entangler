#ifndef INCLUDE_NEOPIXEL_H_
#define INCLUDE_NEOPIXEL_H_

#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/usb/usb_device.h>

void neopixel_thread(void *p1, void *p2, void *p3);

#endif /* INCLUDE_NEOPIXEL_H_ */
