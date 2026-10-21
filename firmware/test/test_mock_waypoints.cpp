// test_mock_waypoints.cpp - test waypoints.h va mock_source
#include <Arduino.h>
#include "check.h"
#include "config.h"
#include "waypoints.h"
#include "mock_source.h"

static long lround_e7(double deg) { return lround(deg * 1e7); }

template <size_t N>
static void check_ranges(const waypoint_t (&w)[N]) {
  for (size_t i = 0; i < N; i++) {
    CHECK(w[i].lat_e7 >= -900000000 && w[i].lat_e7 <= 900000000);
    CHECK(w[i].lon_e7 >= -1800000000 && w[i].lon_e7 <= 1800000000);
    CHECK(w[i].alt_msl_dm % 5 == 0 && w[i].alt_agl_dm % 5 == 0);   // boi so 0.5 m
    CHECK(w[i].alt_msl_dm == w[i].alt_agl_dm + (int) (HOME_ALT_MSL * 10));
    CHECK(w[i].heading_deg < 360);
  }
}

static void test_lengths_and_ranges(void) {
  CHECK(WP_STATIONARY_LEN == 60);
  CHECK(WP_STRAIGHT_LEN == 30);
  CHECK(WP_RECTANGLE_LEN == 40);
  check_ranges(WP_STATIONARY);
  check_ranges(WP_STRAIGHT);
  check_ranges(WP_RECTANGLE);
  CHECK(SCENARIO_EPOCH > 1700000000UL);
}

static void test_start_equals_home_in_config(void) {
  CHECK(WP_STATIONARY[0].lat_e7 == lround_e7(HOME_LAT));
  CHECK(WP_STATIONARY[0].lon_e7 == lround_e7(HOME_LON));
  CHECK(WP_STRAIGHT[0].lat_e7 == lround_e7(HOME_LAT));
  CHECK(WP_RECTANGLE[0].lat_e7 == lround_e7(HOME_LAT));
  CHECK(WP_RECTANGLE[0].lon_e7 == lround_e7(HOME_LON));
}

static void test_stationary(void) {
  for (size_t i = 0; i < WP_STATIONARY_LEN; i++) {
    CHECK(WP_STATIONARY[i].lat_e7 == WP_STATIONARY[0].lat_e7);
    CHECK(WP_STATIONARY[i].lon_e7 == WP_STATIONARY[0].lon_e7);
    CHECK(WP_STATIONARY[i].speed_kn == 0);
  }
}

static void test_straight_north(void) {
  for (size_t i = 0; i < WP_STRAIGHT_LEN; i++) {
    CHECK(WP_STRAIGHT[i].lon_e7 == WP_STRAIGHT[0].lon_e7);
    CHECK(WP_STRAIGHT[i].heading_deg == 0);
    CHECK(WP_STRAIGHT[i].speed_kn == 10);
    if (i) {
      long step = WP_STRAIGHT[i].lat_e7 - WP_STRAIGHT[i - 1].lat_e7;
      CHECK(step >= 460 && step <= 464);                    // ~462 x 1e-7 do moi giay
    }
  }
}

static void test_rectangle_legs_and_closure(void) {
  const int heading[4] = {90, 0, 270, 180};
  for (size_t i = 0; i < WP_RECTANGLE_LEN; i++) CHECK(WP_RECTANGLE[i].heading_deg == heading[i / 10]);
  // buoc tu diem cuoi ve diem 0 bang buoc tren canh cuoi
  long step_leg  = WP_RECTANGLE[38].lat_e7 - WP_RECTANGLE[39].lat_e7;
  long step_back = WP_RECTANGLE[39].lat_e7 - WP_RECTANGLE[0].lat_e7;
  CHECK(labs(step_leg - step_back) <= 1);
  CHECK(WP_RECTANGLE[39].lon_e7 == WP_RECTANGLE[0].lon_e7);
}

static void test_mock_straight_stops_at_end(void) {
  Serial.clear();
  mock_select(1);
  CHECK(mock_scenario() == 1);
  CHECK(Serial.out == "# scenario=1 name=straight len=30\n");
  UTM_data d; memset(&d, 0, sizeof d);
  for (int i = 0; i < 30; i++) {
    CHECK(mock_index() == (size_t) i);
    mock_step(&d);
    CHECK(d.speed_kn == 10);
  }
  CHECK(mock_index() == 29);
  mock_step(&d);
  CHECK(d.speed_kn == 0);
  CHECK_NEAR(d.latitude_d, WP_STRAIGHT[29].lat_e7 / 1e7, 1e-9);
  mock_step(&d);
  CHECK(d.speed_kn == 0);
  CHECK(mock_index() == 29);
  CHECK(d.satellites == MOCK_SATS);
}

static void test_mock_rectangle_loops(void) {
  mock_select(2);
  UTM_data d; memset(&d, 0, sizeof d);
  for (int i = 0; i < 40; i++) mock_step(&d);
  CHECK(mock_index() == 0);
  mock_step(&d);
  CHECK_NEAR(d.latitude_d, WP_RECTANGLE[0].lat_e7 / 1e7, 1e-9);
  CHECK(d.heading == 90);
}

static void test_mock_converts_units(void) {
  mock_select(1);
  UTM_data d; memset(&d, 0, sizeof d);
  mock_step(&d);
  mock_step(&d);
  CHECK_NEAR(d.latitude_d, 10.7725462, 1e-9);
  CHECK_NEAR(d.longitude_d, 106.6578, 1e-9);
  CHECK_NEAR(d.alt_msl_m, 60.0, 1e-4);
  CHECK_NEAR(d.alt_agl_m, 50.0, 1e-4);
}

static void test_mock_select_invalid_ignored(void) {
  mock_select(1);
  mock_select(7);
  mock_select(-1);
  CHECK(mock_scenario() == 1);
  mock_select(0);
  CHECK(mock_scenario() == 0);
}

void run_mock_waypoint_tests(void) {
  RUN(test_lengths_and_ranges);
  RUN(test_start_equals_home_in_config);
  RUN(test_stationary);
  RUN(test_straight_north);
  RUN(test_rectangle_legs_and_closure);
  RUN(test_mock_straight_stops_at_end);
  RUN(test_mock_rectangle_loops);
  RUN(test_mock_converts_units);
  RUN(test_mock_select_invalid_ignored);
}
