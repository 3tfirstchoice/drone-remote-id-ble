// gps_source.h - nguon du lieu GPS that (USE_GPS 1), thay mock_source.
// Module: M5Stack GPS/BDS Unit v1.1 (AT6668), NMEA 0183 qua UART1, phan tich bang TinyGPSPlus.
#pragma once
#include <Arduino.h>
#include "utm.h"

// Mo UART GPS (chan, baud, dinh dang lay tu config.h). Goi 1 lan trong setup().
void gps_begin(void);

// Doc moi ky tu dang cho tren UART va dua vao bo phan tich NMEA. Goi MOI vong loop().
void gps_poll(void);

// Dien UTM_data tu cau NMEA moi nhat. Goi 1 lan moi DATA_PERIOD_MS.
// Tra ve true khi vi tri con moi (< GPS_FIX_MAX_AGE_MS) va du ve tinh (>= SATS_LEVEL_2 = 7).
// Mat fix: d->satellites = 0, thu vien id_open se phat Location voi trang thai loi.
// Khong dien hdop_s/vdop_s: id_open khong doc 2 truong nay (do chinh xac ngang co dinh 10 m).
// Dong thoi dong bo dong ho he thong theo gio GPS (cho den luc co fix dong ho chay theo SCENARIO_EPOCH).
bool gps_step(UTM_data *d);

// Home = vi tri fix dau tien dat chuan. Tra ve false cho den khi co home. Tra ve cung 1 gia tri ve sau.
bool gps_home(double *lat, double *lon, float *alt_msl_m);
