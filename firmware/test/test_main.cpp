// test_main.cpp - chay test tung module
#include "check.h"

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
  return report("modules");
}
