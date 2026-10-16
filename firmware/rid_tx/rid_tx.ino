// rid_tx.ino - phat Remote ID (ASTM F3411) qua BLE Legacy Advertising, ESP32-S3
// Giai doan 1: du lieu gia lap tu waypoints.h. Giai doan 2: GPS NEO-6M (USE_GPS 1).
#include "id_open.h"
#include "config.h"
#include "waypoints.h"      // SCENARIO_EPOCH
#include "mock_source.h"
#include "telemetry.h"
#include "status_led.h"
#if USE_GPS
#include "gps_source.h"
#endif

ID_OpenDrone   squitter;
UTM_parameters utm_parameters;
UTM_data       utm_data;

extern uint32_t ble_err_cnt, ble_tx_cnt;   // PATCH #2

// Dien ngay gio UTC tu dong ho he thong vao UTM_data
static void fill_time(UTM_data *d) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm g;
  gmtime_r(&tv.tv_sec, &g);
  d->years = g.tm_year + 1900;  d->months  = g.tm_mon + 1;  d->days    = g.tm_mday;
  d->hours = g.tm_hour;         d->minutes = g.tm_min;      d->seconds = g.tm_sec;
  d->csecs = tv.tv_usec / 10000;
}

// Khoi tao cac ban tin dinh danh (Basic ID, Operator ID, Self ID)
static void init_identity(void) {
  memset(&utm_parameters, 0, sizeof(utm_parameters));
  strncpy(utm_parameters.UAV_id,       UAV_ID,      sizeof(utm_parameters.UAV_id) - 1);
  strncpy(utm_parameters.UAS_operator, OPERATOR_ID, sizeof(utm_parameters.UAS_operator) - 1);
  utm_parameters.UA_type  = ODID_UATYPE_HELICOPTER_OR_MULTIROTOR;
  utm_parameters.ID_type  = ODID_IDTYPE_SERIAL_NUMBER;
  utm_parameters.ID_type2 = ODID_IDTYPE_NONE;
  squitter.init(&utm_parameters);
  squitter.set_self_id((char *) SELF_ID_TEXT);
}

// Lenh 1 ky tu tu Serial Monitor
static void handle_serial(void) {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
#if !USE_GPS
      case '1': mock_select(0); break;
      case '2': mock_select(1); break;
      case '3': mock_select(2); break;
      case 'r': mock_select(mock_scenario()); break;
#endif
      case 'h': telemetry_header(); break;
      case 'v': Serial.printf("# fw=%s use_gps=%d\n", FW_VERSION, USE_GPS); break;
      case '?': Serial.println("# 1=static 2=straight 3=rectangle r=restart h=header v=version"); break;
      default:  break;   // bo qua \r \n
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(BOOT_DELAY_MS);
  Serial.printf("# rid_tx boot fw=%s use_gps=%d reset_reason=%d\n",
                FW_VERSION, USE_GPS, (int) esp_reset_reason());

  struct timeval tv = { (time_t) SCENARIO_EPOCH, 0 };   // GPS se ghi de khi co gio that
  settimeofday(&tv, NULL);

  led_begin();
  init_identity();
  memset(&utm_data, 0, sizeof(utm_data));

#if USE_GPS
  gps_begin();
  utm_data.base_valid = 0;            // cho home tu GPS
#else
  mock_step(&utm_data);               // dien san du lieu diem dau tien
  mock_select(0);                     // dat lai chi so ve 0 va in dong "# scenario=0"
  utm_data.base_latitude  = HOME_LAT; // vi tri nguoi dieu khien, thu vien chi ghi 1 lan
  utm_data.base_longitude = HOME_LON;
  utm_data.base_alt_m     = HOME_ALT_MSL;
  utm_data.base_valid     = 1;
#endif

  fill_time(&utm_data);
  telemetry_header();
}

void loop() {
  static uint32_t last_tick = millis();
  static uint32_t last_err  = 0;

  handle_serial();
  static bool first_tx_done = false;
  if (!first_tx_done && ble_tx_cnt > 0) {
    first_tx_done = true;
    Serial.printf("# first_tx ms=%lu\n", (unsigned long) millis());
  }
#if USE_GPS
  gps_poll();                          // doc UART GPS lien tuc
#endif

  if (millis() - last_tick >= DATA_PERIOD_MS) {   // nhip 1 Hz, khong dung delay
    last_tick += DATA_PERIOD_MS;
    bool   fix_ok = true;
    int    scn    = -1;
    size_t idx    = 0;

#if USE_GPS
    fix_ok = gps_step(&utm_data);
    if (!utm_data.base_valid &&
        gps_home(&utm_data.base_latitude, &utm_data.base_longitude, &utm_data.base_alt_m)) {
      utm_data.base_valid = 1;
    }
#else
    scn = mock_scenario();
    idx = mock_index();
    mock_step(&utm_data);
#endif

    fill_time(&utm_data);
    telemetry_line(&utm_data, scn, idx);

    bool ble_ok = (ble_err_cnt == last_err);
    last_err = ble_err_cnt;
    led_tick(fix_ok, ble_ok);
  }

  led_service();
  squitter.transmit(&utm_data);        // goi lien tuc, thu vien tu chia nhip 75 ms
}
