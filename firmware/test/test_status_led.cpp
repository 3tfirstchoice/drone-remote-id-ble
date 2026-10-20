// test_status_led.cpp - 3 che do den: binh thuong, chua fix GPS, loi BLE
#include <Arduino.h>
#include "check.h"
#include "status_led.h"
#include "config.h"

static int level_at(uint32_t t_ms) {           // trang thai den sau t_ms ke tu led_tick
  stub_millis = 5000 + t_ms;
  led_service();
  return stub_pin_level[STATUS_LED_PIN];
}
static void tick(bool fix_ok, bool ble_ok) { stub_millis = 5000; led_tick(fix_ok, ble_ok); }

static void test_normal_one_blink(void) {
  tick(true, true);
  CHECK(level_at(0) == HIGH);
  CHECK(level_at(79) == HIGH);
  CHECK(level_at(80) == LOW);
  CHECK(level_at(240) == LOW);                 // khong co nhay thu hai
  CHECK(level_at(999) == LOW);
}

static void test_no_fix_two_blinks(void) {
  tick(false, true);                           // che do "chua fix" (chi gap khi USE_GPS 1)
  CHECK(level_at(0) == HIGH);
  CHECK(level_at(100) == LOW);
  CHECK(level_at(200) == HIGH);                // nhay lan 2
  CHECK(level_at(279) == HIGH);
  CHECK(level_at(280) == LOW);
  CHECK(level_at(900) == LOW);
}

static void test_ble_error_fast_blink(void) {
  tick(true, false);
  CHECK(level_at(0) == HIGH);
  CHECK(level_at(100) == LOW);
  CHECK(level_at(200) == HIGH);
  CHECK(level_at(300) == LOW);
}

static void test_ble_error_has_priority_over_no_fix(void) {
  tick(false, false);
  CHECK(level_at(0) == HIGH);
  CHECK(level_at(100) == LOW);                 // nhay nhanh lien tuc (che do loi BLE), khong phai 2 lan nhay
  CHECK(level_at(200) == HIGH);
}

static void test_recovery_returns_to_normal(void) {
  tick(false, true);
  tick(true, true);
  CHECK(level_at(240) == LOW);                 // co fix lai: chi con 1 lan nhay
}

void run_status_led_tests(void) {
  RUN(test_normal_one_blink);
  RUN(test_no_fix_two_blinks);
  RUN(test_ble_error_fast_blink);
  RUN(test_ble_error_has_priority_over_no_fix);
  RUN(test_recovery_returns_to_normal);
}
