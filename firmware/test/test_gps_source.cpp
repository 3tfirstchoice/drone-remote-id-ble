// test_gps_source.cpp - test gps_source voi cau NMEA gia lap
#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include "check.h"
#include "nmea_fixture.h"
#include "gps_source.h"
#include "config.h"

void gps_test_reset(void);

static void feed_and_poll(const std::string &s) { Serial1.feed(s); gps_poll(); }

static void reset(void) {
  gps_test_reset();
  Serial1.clear();
  stub_millis = 10000;
  stub_reset_clock();
}

static void test_begin_uart_config(void) {
  Serial1 = StubSerial();
  gps_begin();
  CHECK(Serial1.baud == (uint32_t) GPS_BAUD);
  CHECK(Serial1.baud == 115200);
  CHECK(Serial1.cfg == SERIAL_8N1);
  CHECK(Serial1.rx_pin == 6);
  CHECK(Serial1.tx_pin == 7);
  CHECK(Serial1.rxbuf >= 1024);
}

static void test_no_data(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  CHECK(!gps_step(&d));
  CHECK(d.satellites == 0);
  double la, lo; float al;
  CHECK(!gps_home(&la, &lo, &al));
  CHECK(stub_settime_calls == 0);
}

static void test_no_fix_sentences(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  feed_and_poll(gga(0, 0, "", "") + rmc('V', "", ""));
  CHECK(!gps_step(&d));
  CHECK(d.satellites == 0);
  CHECK(stub_settime_calls == 0);
  double la, lo; float al;
  CHECK(!gps_home(&la, &lo, &al));
}

static void test_fix_fills_utm_data(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  feed_and_poll(gga(1, 10, "0.9", "10.0") + rmc('A', "5.14", "90.0"));
  CHECK(gps_step(&d));
  CHECK_NEAR(d.latitude_d, 10.7725, 1e-9);
  CHECK_NEAR(d.longitude_d, 106.6578, 1e-9);
  CHECK_NEAR(d.alt_msl_m, 10.0, 1e-4);
  CHECK_NEAR(d.alt_agl_m, 0.0, 1e-4);
  CHECK(d.satellites == 10);
  CHECK(d.speed_kn == 5);
  CHECK(d.heading == 90);

  double la = 0, lo = 0; float al = 0;
  CHECK(gps_home(&la, &lo, &al));
  CHECK_NEAR(la, 10.7725, 1e-9);
  CHECK_NEAR(lo, 106.6578, 1e-9);
  CHECK_NEAR(al, 10.0, 1e-4);
}

static void test_agl_relative_to_first_fix(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  feed_and_poll(gga(1, 9, "1.0", "10.0") + rmc('A', "0.0", "0.0"));
  CHECK(gps_step(&d));
  stub_millis += 1000;
  feed_and_poll(gga(1, 9, "1.0", "60.5", "030001.000") + rmc('A', "0.0", "0.0", "030001.000"));
  CHECK(gps_step(&d));
  CHECK_NEAR(d.alt_msl_m, 60.5, 1e-4);
  CHECK_NEAR(d.alt_agl_m, 50.5, 1e-4);
  double la, lo; float al;
  CHECK(gps_home(&la, &lo, &al));
  CHECK_NEAR(al, 10.0, 1e-4);
}

static void test_heading_held_when_slow(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  feed_and_poll(gga(1, 10, "0.9", "10.0") + rmc('A', "6.0", "90.0"));
  CHECK(gps_step(&d));
  CHECK(d.heading == 90);
  stub_millis += 1000;
  feed_and_poll(gga(1, 10, "0.9", "10.0", "030001.000") + rmc('A', "0.2", "270.0", "030001.000"));
  CHECK(gps_step(&d));
  CHECK(d.speed_kn == 0);
  CHECK(d.heading == 90);
  stub_millis += 1000;
  feed_and_poll(gga(1, 10, "0.9", "10.0", "030002.000") + rmc('A', "4.0", "359.6", "030002.000"));
  CHECK(gps_step(&d));
  CHECK(d.heading == 0);
}

static void test_fix_lost_after_timeout(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  feed_and_poll(gga(1, 10, "0.9", "10.0") + rmc('A', "5.0", "90.0"));
  CHECK(gps_step(&d));
  stub_millis += GPS_FIX_MAX_AGE_MS + 500;
  CHECK(!gps_step(&d));
  CHECK(d.satellites == 0);
  CHECK_NEAR(d.latitude_d, 10.7725, 1e-9);
  double la, lo; float al;
  CHECK(gps_home(&la, &lo, &al));
  CHECK_NEAR(la, 10.7725, 1e-9);
}

static void test_too_few_satellites(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  feed_and_poll(gga(1, 5, "3.5", "10.0") + rmc('A', "0.0", "0.0"));
  CHECK(!gps_step(&d));
  CHECK(d.satellites == 5);
  double la, lo; float al;
  CHECK(!gps_home(&la, &lo, &al));
  stub_millis += 1000;
  feed_and_poll(gga(1, 8, "1.2", "11.0", "030001.000") + rmc('A', "0.0", "0.0", "030001.000"));
  CHECK(gps_step(&d));
  CHECK(gps_home(&la, &lo, &al));
  CHECK_NEAR(al, 11.0, 1e-4);
}

static void test_bad_checksum_ignored(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  std::string bad = gga(1, 10, "0.9", "10.0");
  bad[bad.size() - 5] = (bad[bad.size() - 5] == '0') ? '1' : '0';
  feed_and_poll(bad + rmc('A', "5.0", "90.0"));
  CHECK(!gps_step(&d));
}

static void test_clock_sync(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  feed_and_poll(gga(1, 10, "0.9", "10.0") + rmc('A', "5.0", "90.0"));
  stub_millis += 300;
  CHECK(gps_step(&d));
  CHECK(stub_settime_calls == 1);
  // 2026-10-20 03:00:00 UTC
  struct tm t; memset(&t, 0, sizeof t);
  t.tm_year = 2026 - 1900; t.tm_mon = 9; t.tm_mday = 20; t.tm_hour = 3;
  CHECK((long long) stub_settime_last.tv_sec == (long long) timegm(&t));
  CHECK((long long) stub_settime_last.tv_sec == 1792465200LL);
  CHECK(stub_settime_last.tv_usec >= 290000 && stub_settime_last.tv_usec <= 310000);

  // chua dong bo lai trong GPS_CLOCK_SYNC_MS
  stub_millis += 1000;
  feed_and_poll(gga(1, 10, "0.9", "10.0", "030001.000") + rmc('A', "5.0", "90.0", "030001.000"));
  CHECK(gps_step(&d));
  CHECK(stub_settime_calls == 1);
  // dong bo lai sau GPS_CLOCK_SYNC_MS
  stub_millis += GPS_CLOCK_SYNC_MS;
  feed_and_poll(gga(1, 10, "0.9", "10.0", "030101.000") + rmc('A', "5.0", "90.0", "030101.000"));
  CHECK(gps_step(&d));
  CHECK(stub_settime_calls == 2);
}

static void test_gp_talker_also_works(void) {
  reset();
  UTM_data d; memset(&d, 0, sizeof d);
  std::string g = nmea("GPGGA,030000.000,1046.3500,N,10639.4680,E,1,08,1.1,10.0,M,0.0,M,,");
  std::string r = nmea("GPRMC,030000.000,A,1046.3500,N,10639.4680,E,3.0,45.0,201026,,,A");
  feed_and_poll(g + r);
  CHECK(gps_step(&d));
  CHECK(d.satellites == 8);
  CHECK(d.heading == 45);
}

void run_gps_source_tests(void) {
  RUN(test_begin_uart_config);
  RUN(test_no_data);
  RUN(test_no_fix_sentences);
  RUN(test_fix_fills_utm_data);
  RUN(test_agl_relative_to_first_fix);
  RUN(test_heading_held_when_slow);
  RUN(test_fix_lost_after_timeout);
  RUN(test_too_few_satellites);
  RUN(test_bad_checksum_ignored);
  RUN(test_clock_sync);
  RUN(test_gp_talker_also_works);
}
