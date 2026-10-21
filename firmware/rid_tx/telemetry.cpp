// telemetry.cpp - log CSV qua Serial
#include "telemetry.h"
#include "ble_stats.h"

// Ten cot, phai khop thu tu voi telemetry_format
#define TELEMETRY_COLUMNS "ms,utc,scn,idx,lat,lon,alt_msl,alt_agl,speed_kn,heading,sats,tx,err,heap"

void telemetry_header(void) {
  Serial.println("H," TELEMETRY_COLUMNS);
}

size_t telemetry_format(char *buf, size_t n, const UTM_data *d, int scn, size_t idx,
                        uint32_t ms, uint32_t tx, uint32_t err, uint32_t heap) {
  int k = snprintf(buf, n,
    "D,%lu,%04d-%02d-%02dT%02d:%02d:%02dZ,%d,%u,%.7f,%.7f,%.1f,%.1f,%d,%d,%d,%lu,%lu,%lu\n",
    (unsigned long) ms,
    d->years, d->months, d->days, d->hours, d->minutes, d->seconds,
    scn, (unsigned) idx, d->latitude_d, d->longitude_d, d->alt_msl_m, d->alt_agl_m,
    d->speed_kn, d->heading, d->satellites,
    (unsigned long) tx, (unsigned long) err, (unsigned long) heap);
  return (k < 0) ? 0 : (size_t) k;
}

void telemetry_line(const UTM_data *d, int scn, size_t idx) {
  char buf[200];
  telemetry_format(buf, sizeof buf, d, scn, idx, millis(), ble_tx_cnt, ble_err_cnt, ESP.getFreeHeap());
  Serial.print(buf);
}
