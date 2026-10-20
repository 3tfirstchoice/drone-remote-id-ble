// nmea_fixture.h - tao cau NMEA gia lap (co checksum dung) cho test GPS
#pragma once
#include <stdio.h>
#include <string>

// Dong goi "$<body>*HH\r\n", HH = XOR moi ky tu cua body
static inline std::string nmea(const char *body) {
  unsigned x = 0;
  for (const char *p = body; *p; p++) x ^= (unsigned char) *p;
  char tail[16];
  snprintf(tail, sizeof tail, "*%02X\r\n", x);
  return std::string("$") + body + tail;
}

// Toa do HCMUT: 10.7725 N = 10 do 46.3500 phut; 106.6578 E = 106 do 39.4680 phut. Ngay 20/10/2026.
// quality = 0: chua fix (truong toa do rong)
static inline std::string gga(int quality, int sats, const char *hdop, const char *alt,
                              const char *hhmmss = "030000.000") {
  char b[200];
  if (quality == 0)
    snprintf(b, sizeof b, "GNGGA,%s,,,,,0,00,99.99,,M,,M,,", hhmmss);
  else
    snprintf(b, sizeof b, "GNGGA,%s,1046.3500,N,10639.4680,E,%d,%02d,%s,%s,M,0.0,M,,",
             hhmmss, quality, sats, hdop, alt);
  return nmea(b);
}

// status = 'A': co fix; 'V': chua fix (module van gui ngay gio)
static inline std::string rmc(char status, const char *speed_kn, const char *course,
                              const char *hhmmss = "030000.000") {
  char b[200];
  if (status == 'A')
    snprintf(b, sizeof b, "GNRMC,%s,A,1046.3500,N,10639.4680,E,%s,%s,201026,,,A", hhmmss, speed_kn, course);
  else
    snprintf(b, sizeof b, "GNRMC,%s,V,,,,,,,201026,,,N", hhmmss);
  return nmea(b);
}
