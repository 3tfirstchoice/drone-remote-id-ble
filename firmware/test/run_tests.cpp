// run_tests.cpp - diem vao cua bo test tren may tinh (xem run_tests.sh)
#include <Arduino.h>
#include "check.h"

uint32_t   stub_millis = 0;
int        stub_pin_level[64] = {0};
StubSerial Serial, Serial1;
StubESP    ESP;
int g_pass = 0, g_fail = 0;
uint32_t ble_tx_cnt = 105, ble_err_cnt = 0;   // that: dinh nghia trong libraries/id_open/id_open_esp32.cpp

void run_gps_source_tests(void);
void run_telemetry_tests(void);
void run_status_led_tests(void);
void run_mock_waypoint_tests(void);
void run_operator_id_tests(void);

int main(void) {
  run_mock_waypoint_tests();
  run_telemetry_tests();
  run_status_led_tests();
  run_operator_id_tests();
  run_gps_source_tests();
  printf("\n%d kiem tra dat, %d that bai\n", g_pass, g_fail);
  return g_fail ? 1 : 0;
}
