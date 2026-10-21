// status_led.h - LED trang thai: 1 nhay/giay binh thuong, 2 nhay/giay chua fix, nhay nhanh loi BLE
#pragma once
#include <Arduino.h>

void led_begin(void);
void led_tick(bool fix_ok, bool ble_ok);   // cap nhat che do, goi moi DATA_PERIOD_MS
void led_service(void);                    // bat/tat LED, goi moi vong loop()
