#pragma once
#include "inttypes.h"
#define MMNL_printf(...) printf(__VA_ARGS__)

#if defined(ESP8266) || defined(ESP32)
#define MMNL_ISR ICACHE_RAM_ATTR void
#else
#define MMNL_ISR void
#endif

unsigned long mmnl_millis();

#define MMNL_MILLIS mmnl_millis

#ifndef MMNL_MILLIS
#define MMNL_MILLIS millis
#endif

//#define MMNL_PIW_FULL_LIST

#define MMNL_PIW_LAST_LIST_SIZE 32 //3 bytes for 1