#pragma once

#ifndef TARGET_ENV_NATIVE

#include <Arduino.h>

#ifdef ENABLE_DEBUG

#ifdef USE_USBSERIAL
#define _SERIAL USBSerial
#else
#define _SERIAL Serial
#endif  // USE_USBSERIAL

#define DBG(...)                  \
  {                               \
    _SERIAL.print("[");           \
    _SERIAL.print(__FUNCTION__);  \
    _SERIAL.print("(): ");        \
    _SERIAL.print(__LINE__);      \
    _SERIAL.print("] ");          \
    _SERIAL.println(__VA_ARGS__); \
  }

#define INITIALISE_DBG(...) \
  { _SERIAL.begin(__VA_ARGS__); }

#else

#define DBG(...)
#define INITIALISE_DBG(...)

#endif  // ENABLE_DEBUG

#else

#define DBG(...)
#define INITIALISE_DBG(...)

#endif  // TARGET_ENV_NATIVE