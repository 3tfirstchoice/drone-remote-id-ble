// status_led.h - den xanh don GPIO48, bao trang thai bang so lan nhay moi giay
//   1 lan nhay/giay     : binh thuong
//   2 lan nhay/giay     : chua co GPS fix (chi dung o giai doan 2)
//   nhay nhanh lien tuc : BLE bao loi
#pragma once
#include <Arduino.h>

void led_begin(void);
void led_tick(bool fix_ok, bool ble_ok);   // goi 1 lan moi giay, dat lai chu ky nhay
void led_service(void);                    // goi moi vong loop(), khong chan
