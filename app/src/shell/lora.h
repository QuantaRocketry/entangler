#pragma once

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_lora,
	SHELL_CMD(config, NULL,
		  "Configure the LoRa radio\n"
		  " Usage: config [freq <Hz>] [tx-power <dBm>] [bw <kHz>] "
		  "[sf <int>] [cr <int>] [pre-len <int>]\n",
		  NULL),
	SHELL_CMD_ARG(send, NULL,
		      "Send LoRa packet\n"
		      " Usage: send <data>",
		      NULL, 2, 0),
	SHELL_CMD_ARG(recv, NULL,
		      "Receive LoRa packet\n"
		      " Usage: recv [timeout (ms)]",
		      NULL, 1, 1),
	SHELL_CMD_ARG(test_cw, NULL,
		      "Send a continuous wave\n"
		      " Usage: test_cw <freq (Hz)> <power (dBm)> <duration (s)>",
		      NULL, 4, 0),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(lora_ext, &sub_lora, "Extended LoRa commands", NULL);
