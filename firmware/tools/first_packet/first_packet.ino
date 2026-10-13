// first_packet.ino - phat Remote ID tinh (T01), ESP32-S3, BLE Legacy Advertising
#include "id_open.h"

#define UAV_ID         "LAB-ESP32-0001"
#define OPERATOR_ID    "LAB-OPERATOR-01"
#define SELF_ID_TEXT   "LAB SIMULATION"
#define HOME_LAT       10.7725
#define HOME_LON       106.6578
#define HOME_ALT_MSL   10.0f
#define EPOCH_UTC      1791849600UL      // 2026-10-13 00:00:00 UTC

ID_OpenDrone   squitter;
UTM_parameters utm_parameters;
UTM_data       utm_data;
static char    dop_str[] = "1.0";

extern uint32_t ble_tx_cnt, ble_err_cnt;   // PATCH #2

static void fill_time(UTM_data *d) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm g;
  gmtime_r(&tv.tv_sec, &g);
  d->years = g.tm_year + 1900;  d->months  = g.tm_mon + 1;  d->days    = g.tm_mday;
  d->hours = g.tm_hour;         d->minutes = g.tm_min;      d->seconds = g.tm_sec;
  d->csecs = tv.tv_usec / 10000;
}

void setup() {
  Serial.begin(115200);
  delay(3000);                                   // USB native: cho may tinh nhan cong
  Serial.println("# first_packet boot");

  struct timeval tv = { (time_t) EPOCH_UTC, 0 }; // khong co gio thi System timestamp khong cap nhat
  settimeofday(&tv, NULL);

  memset(&utm_parameters, 0, sizeof(utm_parameters));
  strcpy(utm_parameters.UAV_id,       UAV_ID);
  strcpy(utm_parameters.UAS_operator, OPERATOR_ID);
  utm_parameters.UA_type  = ODID_UATYPE_HELICOPTER_OR_MULTIROTOR;
  utm_parameters.ID_type  = ODID_IDTYPE_SERIAL_NUMBER;
  utm_parameters.ID_type2 = ODID_IDTYPE_NONE;
  squitter.init(&utm_parameters);
  squitter.set_self_id((char *) SELF_ID_TEXT);

  memset(&utm_data, 0, sizeof(utm_data));
  utm_data.latitude_d  = HOME_LAT;
  utm_data.longitude_d = HOME_LON;
  utm_data.alt_msl_m   = HOME_ALT_MSL;
  utm_data.alt_agl_m   = 0.0f;
  utm_data.speed_kn    = 0;
  utm_data.heading     = 0;
  utm_data.satellites  = 10;                     // BAT BUOC >= 7, neu khong vi tri bi chan
  utm_data.hdop_s = utm_data.vdop_s = dop_str;
  utm_data.base_latitude  = HOME_LAT;            // vi tri nguoi dieu khien (ghi 1 lan)
  utm_data.base_longitude = HOME_LON;
  utm_data.base_alt_m     = HOME_ALT_MSL;
  utm_data.base_valid     = 1;
  fill_time(&utm_data);
}

void loop() {
  static uint32_t last = millis();
  if (millis() - last >= 1000) {                 // 1 Hz, khong dung delay
    last += 1000;
    fill_time(&utm_data);
    Serial.printf("%lu,%.7f,%.7f,tx=%lu,err=%lu\n", millis(),
                  utm_data.latitude_d, utm_data.longitude_d,
                  (unsigned long) ble_tx_cnt, (unsigned long) ble_err_cnt);
  }
  squitter.transmit(&utm_data);                  // goi lien tuc, khong delay
}