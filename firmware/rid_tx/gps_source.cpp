// gps_source.cpp - doc GPS that qua UART1 (TinyGPSPlus), dien UTM_data moi giay.
// Toan bo file chi duoc bien dich khi USE_GPS 1, nen build gia lap khong can cai TinyGPSPlus.
#include "config.h"

#if USE_GPS

#include "gps_source.h"
#include <TinyGPSPlus.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>

static TinyGPSPlus gps;

static char    hdop_str[8] = "99.9";   // HDOP = sai so ngang cua GPS, cang nho cang tot
static char    vdop_str[8] = "99.9";   // TinyGPSPlus khong doc cau GSA nen khong biet VDOP
static bool    home_set    = false;
static double  home_lat    = 0.0, home_lon = 0.0;
static float   home_alt    = 0.0f;
static int     last_heading = 0;       // huong GPS khong tin cay khi dung yen -> giu huong cu
static bool    clock_synced = false;
static uint32_t last_sync_ms = 0;

// ---------------------------------------------------------------- thoi gian
// Ngay gio UTC -> giay Unix (thuat toan "days from civil", khong phu thuoc timegm cua libc).
static int64_t utc_to_unix(int y, int m, int d, int hh, int mm, int ss) {
  y -= (m <= 2);
  const int      era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned) (y - era * 400);
  const unsigned doy = (unsigned) ((153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1);
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  const int64_t  days = (int64_t) era * 146097 + (int64_t) doe - 719468;
  return days * 86400 + hh * 3600 + mm * 60 + ss;
}

// Dat dong ho he thong theo gio GPS (chi khi dang co fix, de tranh gio rac cua module chua dinh vi).
static void sync_clock(void) {
  const uint32_t now = millis();
  if (clock_synced && (uint32_t) (now - last_sync_ms) < GPS_CLOCK_SYNC_MS) return;
  if (!(gps.location.isValid() && gps.date.isValid() && gps.time.isValid())) return;
  if (gps.location.age() > GPS_FIX_MAX_AGE_MS || gps.date.age() > GPS_FIX_MAX_AGE_MS ||
      gps.time.age() > GPS_FIX_MAX_AGE_MS) return;
  if (gps.date.year() < 2024) return;   // ngay vo ly: bo qua

  int64_t t  = utc_to_unix(gps.date.year(), gps.date.month(), gps.date.day(),
                           gps.time.hour(), gps.time.minute(), gps.time.second());
  // Cong them thoi gian tu luc nhan cau NMEA den bay gio (do tre truyen UART + xu ly)
  int64_t us = (int64_t) gps.time.centisecond() * 10000 + (int64_t) gps.time.age() * 1000;
  t  += us / 1000000;
  us %= 1000000;

  struct timeval tv;
  tv.tv_sec  = (time_t) t;
  tv.tv_usec = (suseconds_t) us;
  settimeofday(&tv, NULL);
  clock_synced = true;
  last_sync_ms = now;
}

// ---------------------------------------------------------------- API
void gps_begin(void) {
  Serial1.setRxBufferSize(1024);   // phai goi truoc begin(); du chua vai chu ky NMEA khi loop() ban
  Serial1.begin(GPS_BAUD, GPS_UART_7N1 ? SERIAL_7N1 : SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void gps_poll(void) {
  while (Serial1.available() > 0) {
    gps.encode((char) Serial1.read());
  }
}

bool gps_step(UTM_data *d) {
  sync_clock();

  const bool fresh = gps.location.isValid() && gps.location.age() <= GPS_FIX_MAX_AGE_MS &&
                     gps.altitude.isValid() && gps.altitude.age() <= GPS_FIX_MAX_AGE_MS;
  const bool sats_ok = gps.satellites.isValid() && gps.satellites.age() <= GPS_FIX_MAX_AGE_MS;

  d->satellites = (fresh && sats_ok) ? (int) gps.satellites.value() : 0;
  d->hdop_s = hdop_str;
  d->vdop_s = vdop_str;

  if (!fresh) {
    snprintf(hdop_str, sizeof hdop_str, "99.9");
    return false;                  // giu nguyen toa do cu; satellites = 0 -> thu vien bao loi
  }

  d->latitude_d  = gps.location.lat();
  d->longitude_d = gps.location.lng();
  d->alt_msl_m   = (float) gps.altitude.meters();

  if (gps.hdop.isValid()) snprintf(hdop_str, sizeof hdop_str, "%.1f", gps.hdop.hdop());
  else                    snprintf(hdop_str, sizeof hdop_str, "99.9");

  const bool speed_ok = gps.speed.isValid() && gps.speed.age() <= GPS_FIX_MAX_AGE_MS;
  d->speed_kn = speed_ok ? (int) lround(gps.speed.knots()) : 0;

  if (gps.course.isValid() && gps.course.age() <= GPS_FIX_MAX_AGE_MS && d->speed_kn >= GPS_MIN_SPEED_KN) {
    last_heading = ((int) lround(gps.course.deg())) % 360;
  }
  d->heading = last_heading;

  const bool ok = (d->satellites >= SATS_LEVEL_2);

  if (ok && !home_set) {           // fix dat chuan dau tien = diem cat canh
    home_lat = d->latitude_d;
    home_lon = d->longitude_d;
    home_alt = d->alt_msl_m;
    home_set = true;
  }
  d->alt_agl_m = home_set ? (d->alt_msl_m - home_alt) : 0.0f;

  return ok;
}

bool gps_home(double *lat, double *lon, float *alt_msl_m) {
  if (!home_set) return false;
  *lat = home_lat;
  *lon = home_lon;
  *alt_msl_m = home_alt;
  return true;
}

#ifdef GPS_SOURCE_TESTING
void gps_test_reset(void) {         // chi dung trong test tren may tinh
  gps = TinyGPSPlus();
  snprintf(hdop_str, sizeof hdop_str, "99.9");
  home_set = false; home_lat = home_lon = 0.0; home_alt = 0.0f;
  last_heading = 0; clock_synced = false; last_sync_ms = 0;
}
#endif

#endif  // USE_GPS
