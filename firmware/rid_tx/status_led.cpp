// status_led.cpp - dieu khien LED khong dung delay()
#include "status_led.h"
#include "config.h"

static uint32_t t0   = 0;      // thoi diem bat dau chu ky 1 giay
static uint8_t  mode = 0;      // 0 = OK, 1 = chua fix, 2 = loi BLE

void led_begin(void) {
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void led_tick(bool fix_ok, bool ble_ok) {
  mode = !ble_ok ? 2 : (!fix_ok ? 1 : 0);
  t0 = millis();
}

void led_service(void) {
  uint32_t t = millis() - t0;
  bool on = false;
  if (mode == 0)      on = (t < 80);
  else if (mode == 1) on = (t < 80) || (t >= 200 && t < 280);
  else                on = ((t / 100) % 2 == 0);
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
}
