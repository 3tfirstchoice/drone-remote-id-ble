// stubs.cpp - bien va ham gia lap dung chung cho test
#include <Arduino.h>
#include <sys/time.h>
#include "check.h"

// --- check.h ---
int g_pass = 0, g_fail = 0;

// --- Arduino.h gia lap ---
uint32_t   stub_millis = 0;
int        stub_pin_level[64] = {0};
StubSerial Serial, Serial1;
StubESP    ESP;

// --- bo dem BLE ---
uint32_t ble_tx_cnt = 0, ble_err_cnt = 0;

// --- dong ho gia lap: gio da dat + thoi gian millis() da troi ---
int            stub_settime_calls = 0;
struct timeval stub_settime_last  = {0, 0};
static uint32_t stub_settime_ms   = 0;

int stub_settimeofday(const struct timeval *tv, const struct timezone *) noexcept {
  stub_settime_calls++;
  stub_settime_last = *tv;
  stub_settime_ms   = stub_millis;
  return 0;
}

int stub_gettimeofday(struct timeval *tv, void *) noexcept {
  const int64_t us = (int64_t) stub_settime_last.tv_sec * 1000000 + stub_settime_last.tv_usec +
                     (int64_t) (uint32_t) (stub_millis - stub_settime_ms) * 1000;
  tv->tv_sec  = (time_t) (us / 1000000);
  tv->tv_usec = (suseconds_t) (us % 1000000);
  return 0;
}

void stub_reset_clock(void) {
  stub_settime_calls = 0;
  stub_settime_last  = {0, 0};
  stub_settime_ms    = 0;
}
