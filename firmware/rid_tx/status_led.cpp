// status_led.cpp - dieu khien LED trang thai khong dung delay()
#include "status_led.h"
#include "config.h"

enum LedMode : uint8_t {
  LED_OK,          // 1 nhay moi giay
  LED_NO_FIX,      // 2 nhay moi giay
  LED_BLE_ERROR,   // nhay nhanh lien tuc
};

static uint32_t t0   = 0;        // thoi diem bat dau chu ky hien tai
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
  const uint32_t t = millis() - t0;
  bool on;
  switch (mode) {
    case LED_OK:     on = (t < 80);                              break;
    case LED_NO_FIX: on = (t < 80) || (t >= 200 && t < 280);     break;
    default:         on = ((t / 100) % 2 == 0);                  break;
  }
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
}
