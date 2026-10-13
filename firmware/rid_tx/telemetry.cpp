// telemetry.cpp - dinh dang: H,... (header) va D,... (du lieu)
#include "telemetry.h"

extern uint32_t ble_tx_cnt, ble_err_cnt;   // PATCH #2

void telemetry_header(void) {
  Serial.println("H,ms,utc,scn,idx,lat,lon,alt_msl,alt_agl,speed_kn,heading,sats,tx,err,heap");
}

void telemetry_line(const UTM_data *d, int scn, size_t idx) {
  Serial.printf("D,%lu,%04d-%02d-%02dT%02d:%02d:%02dZ,%d,%u,%.7f,%.7f,%.1f,%.1f,%d,%d,%d,%lu,%lu,%lu\n",
    (unsigned long) millis(),
    d->years, d->months, d->days, d->hours, d->minutes, d->seconds,
    scn, (unsigned) idx, d->latitude_d, d->longitude_d, d->alt_msl_m, d->alt_agl_m,
    d->speed_kn, d->heading, d->satellites,
    (unsigned long) ble_tx_cnt, (unsigned long) ble_err_cnt, (unsigned long) ESP.getFreeHeap());
}
