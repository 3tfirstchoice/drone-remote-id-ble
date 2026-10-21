// gps_source.h - nguon du lieu GPS that (USE_GPS 1), doc NMEA qua UART1
#pragma once
#include <Arduino.h>
#include "utm.h"

// Mo UART GPS
void gps_begin(void);

// Doc du lieu UART vao bo phan tich NMEA, goi moi vong loop()
void gps_poll(void);

// Dien UTM_data tu du lieu GPS moi nhat va dong bo dong ho; tra ve true khi co fix va du ve tinh
bool gps_step(UTM_data *d);

// Lay vi tri home (fix dat chuan dau tien); tra ve false khi chua co
bool gps_home(double *lat, double *lon, float *alt_msl_m);
