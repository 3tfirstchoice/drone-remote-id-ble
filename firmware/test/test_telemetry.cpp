// test_telemetry.cpp - dong log CSV: giu nguyen dinh dang cu va header/du lieu cung so cot
#include <Arduino.h>
#include "check.h"
#include "telemetry.h"

static size_t count_commas(const char *s) { size_t n = 0; for (; *s; s++) if (*s == ',') n++; return n; }

static UTM_data sample(void) {
  UTM_data d; memset(&d, 0, sizeof d);
  d.years = 2026; d.months = 10; d.days = 16; d.hours = 3; d.minutes = 49; d.seconds = 17;
  d.latitude_d = 10.7725462; d.longitude_d = 106.6578; d.alt_msl_m = 60.0f; d.alt_agl_m = 50.0f;
  d.speed_kn = 10; d.heading = 0; d.satellites = 10;
  return d;
}

static void test_same_output_as_original_printf(void) {
  UTM_data d = sample();
  char got[200], want[200];
  telemetry_format(got, sizeof got, &d, 1, 1, 19446, 105, 0, 265600);
  // Dinh dang goc cua telemetry_line truoc khi tach ham (fw 0.3.0), giu nguyen tung ky tu
  snprintf(want, sizeof want,
    "D,%lu,%04d-%02d-%02dT%02d:%02d:%02dZ,%d,%u,%.7f,%.7f,%.1f,%.1f,%d,%d,%d,%lu,%lu,%lu\n",
    (unsigned long) 19446, d.years, d.months, d.days, d.hours, d.minutes, d.seconds,
    1, (unsigned) 1, d.latitude_d, d.longitude_d, d.alt_msl_m, d.alt_agl_m,
    d.speed_kn, d.heading, d.satellites, (unsigned long) 105, (unsigned long) 0, (unsigned long) 265600);
  CHECK_STR(got, want);
  CHECK_STR(got, "D,19446,2026-10-16T03:49:17Z,1,1,10.7725462,106.6578000,60.0,50.0,10,0,10,105,0,265600\n");
}

static void test_header_and_line_have_same_columns(void) {
  Serial.clear();
  telemetry_header();
  std::string header = Serial.out;
  UTM_data d = sample();
  char line[200];
  telemetry_format(line, sizeof line, &d, 0, 0, 1, 2, 3, 4);
  CHECK(header.rfind("H,ms,utc,scn,idx,", 0) == 0);
  CHECK(count_commas(header.c_str()) == count_commas(line));
}

static void test_line_goes_to_serial_without_extra_crlf(void) {
  Serial.clear();
  stub_millis = 12345;
  UTM_data d = sample();
  telemetry_line(&d, -1, 0);
  CHECK(Serial.out.size() > 20);
  CHECK(Serial.out.back() == '\n');
  CHECK(Serial.out.find("\r") == std::string::npos);   // printf goc chi co '\n'
  CHECK(Serial.out.rfind("D,12345,", 0) == 0);
}

static void test_buffer_too_small_is_safe(void) {
  UTM_data d = sample();
  char tiny[10];
  telemetry_format(tiny, sizeof tiny, &d, 0, 0, 1, 2, 3, 4);
  CHECK(strlen(tiny) < sizeof tiny);                    // snprintf luon ket thuc chuoi
}

void run_telemetry_tests(void) {
  RUN(test_same_output_as_original_printf);
  RUN(test_header_and_line_have_same_columns);
  RUN(test_line_goes_to_serial_without_extra_crlf);
  RUN(test_buffer_too_small_is_safe);
}
