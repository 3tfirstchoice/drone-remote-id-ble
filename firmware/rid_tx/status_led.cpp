// status_led.cpp - dieu khien LED khong dung delay(): led_tick dat che do, led_service bat/tat
#include "status_led.h"
#include "config.h"

enum LedMode : uint8_t {
  LED_OK,          // 1 lan nhay moi giay
  LED_NO_FIX,      // 2 lan nhay moi giay
  LED_BLE_ERROR,   // nhay nhanh lien tuc (uu tien cao nhat)
};

static uint32_t t0   = 0;        // thoi diem bat dau chu ky 1 giay hien tai
static LedMode  mode = LED_OK;

void led_begin(void) {
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void led_tick(bool fix_ok, bool ble_ok) {
  mode = !ble_ok ? LED_BLE_ERROR : (!fix_ok ? LED_NO_FIX : LED_OK);
  t0 = millis();
}

void led_service(void) {
  const uint32_t t = millis() - t0;   // ms ke tu dau chu ky
  bool on;
  switch (mode) {
    case LED_OK:     on = (t < 80);                              break;
    case LED_NO_FIX: on = (t < 80) || (t >= 200 && t < 280);     break;
    default:         on = ((t / 100) % 2 == 0);                  break;   // 100 ms sang, 100 ms tat
  }
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
}
