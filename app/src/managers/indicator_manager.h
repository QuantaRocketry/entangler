#pragma once

#include <inttypes.h>

/** @brief An indicator event
 */
typedef enum {
	INDICATOR_ERROR,   /**< An error has occured */
	INDICATOR_MESSAGE, /**< A telemetry message has been received */
} indicator_event_t;
