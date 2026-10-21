// rid_tx.ino - phat Remote ID (ASTM F3411) qua BLE Legacy Advertising tren ESP32-S3
#include "id_open.h"
#include "config.h"
#include "waypoints.h"
#include "ble_stats.h"
#include "telemetry.h"
#include "status_led.h"
#if USE_GPS
#include "gps_source.h"
#else
#include "mock_source.h"
#endif

static ID_OpenDrone   squitter;
static UTM_parameters utm_parameters;
static UTM_data       utm_data;        // du lieu bay dung chung cho nguon du lieu va id_open

// ================================================================ thoi gian, dinh danh

// Ghi ngay gio UTC hien tai vao UTM_data
static void fill_time(UTM_data *d) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm g;
  gmtime_r(&tv.tv_sec, &g);
  d->years = g.tm_year + 1900;  d->months  = g.tm_mon + 1;  d->days    = g.tm_mday;
  d->hours = g.tm_hour;         d->minutes = g.tm_min;      d->seconds = g.tm_sec;
  d->csecs = tv.tv_usec / 10000;
}

// Khoi tao ban tin Basic ID, Operator ID, Self ID
static void init_identity(void) {
  memset(&utm_parameters, 0, sizeof(utm_parameters));
  strncpy(utm_parameters.UAV_id,       UAV_ID,      sizeof(utm_parameters.UAV_id) - 1);
  strncpy(utm_parameters.UAS_operator, OPERATOR_ID, sizeof(utm_parameters.UAS_operator) - 1);
  utm_parameters.UA_type  = ODID_UATYPE_HELICOPTER_OR_MULTIROTOR;
  utm_parameters.ID_type  = ODID_IDTYPE_SERIAL_NUMBER;
  utm_parameters.ID_type2 = ODID_IDTYPE_NONE;
  squitter.init(&utm_parameters);
  squitter.set_self_id((char *) SELF_ID_TEXT);
  Serial.printf("# identity uav=%s operator=%s self_id=%s\n", UAV_ID, OPERATOR_ID, SELF_ID_TEXT);
}

// ================================================================ nguon du lieu
//   source_begin   : khoi tao nguon, dat home neu co
//   source_poll    : goi moi vong loop()
//   source_step    : dien utm_data moi DATA_PERIOD_MS, tra ve true khi vi tri hop le
//   source_command : xu ly lenh Serial cua nguon, tra ve true neu da xu ly
#if USE_GPS

#define SOURCE_HELP ""

static void source_begin(UTM_data *d) {
  gps_begin();
  Serial.printf("# gps uart rx=GPIO%d tx=GPIO%d baud=%d %s, cho fix...\n",
                GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD, GPS_UART_7N1 ? "7N1" : "8N1");
  d->base_valid = 0;
}

static void source_poll(void) { gps_poll(); }

static bool source_step(UTM_data *d, int *scn, size_t *idx) {
  *scn = SCN_NONE;
  *idx = 0;
  const bool ok = gps_step(d);
  if (!d->base_valid && gps_home(&d->base_latitude, &d->base_longitude, &d->base_alt_m)) {
    d->base_valid = 1;
  }
  return ok;
}

static bool source_command(char c) { (void) c; return false; }

#else

#define SOURCE_HELP "1=static 2=straight 3=rectangle r=restart "

static void source_begin(UTM_data *d) {
  mock_step(d);
  mock_select(0);
  d->base_latitude  = HOME_LAT;
  d->base_longitude = HOME_LON;
  d->base_alt_m     = HOME_ALT_MSL;
  d->base_valid     = 1;
}

static void source_poll(void) {}

static bool source_step(UTM_data *d, int *scn, size_t *idx) {
  *scn = mock_scenario();
  *idx = mock_index();
  mock_step(d);
  return true;
}

static bool source_command(char c) {
  switch (c) {
    case '1': case '2': case '3': mock_select(c - '1');        return true;
    case 'r':                     mock_select(mock_scenario()); return true;
    default:                      return false;
  }
}

#endif

// ================================================================ lenh Serial
static void handle_serial(void) {
  while (Serial.available()) {
    const char c = Serial.read();
    if (source_command(c)) continue;
    switch (c) {
      case 'h': telemetry_header(); break;
      case 'v': Serial.printf("# fw=%s use_gps=%d\n", FW_VERSION, USE_GPS); break;
      case '?': Serial.println("# " SOURCE_HELP "h=header v=version"); break;
      default:  break;
    }
  }
}

// ================================================================ tinh trang BLE

// In thoi diem phat goi BLE dau tien, hoac canh bao neu qua FIRST_TX_WARN_MS chua phat duoc
static void report_first_tx(void) {
  static bool done = false, warned = false;
  if (done) return;
  if (ble_tx_cnt > 0) {
    done = true;
    Serial.printf("# first_tx ms=%lu\n", (unsigned long) millis());
  } else if (!warned && millis() > FIRST_TX_WARN_MS) {
    warned = true;
    Serial.printf("# CANH BAO: sau %d ms van chua phat duoc goi BLE nao (ble_err=%lu)\n",
                  FIRST_TX_WARN_MS, (unsigned long) ble_err_cnt);
  }
}

// true neu khong co loi BLE moi tu lan goi truoc
static bool ble_healthy(void) {
  static uint32_t last_err = 0;
  const bool ok = (ble_err_cnt == last_err);
  last_err = ble_err_cnt;
  return ok;
}

// ================================================================ Arduino
void setup() {
  Serial.begin(115200);
  delay(BOOT_DELAY_MS);
  Serial.printf("# rid_tx boot fw=%s use_gps=%d reset_reason=%d\n",
                FW_VERSION, USE_GPS, (int) esp_reset_reason());

  struct timeval tv = { (time_t) SCENARIO_EPOCH, 0 };
  settimeofday(&tv, NULL);

  led_begin();
  init_identity();
  memset(&utm_data, 0, sizeof(utm_data));
  source_begin(&utm_data);
  fill_time(&utm_data);
  telemetry_header();
}

void loop() {
  static uint32_t last_tick = millis();

  handle_serial();
  report_first_tx();
  source_poll();

  if (millis() - last_tick >= DATA_PERIOD_MS) {
    last_tick += DATA_PERIOD_MS;
    int    scn;
    size_t idx;
    const bool fix_ok = source_step(&utm_data, &scn, &idx);
    fill_time(&utm_data);
    telemetry_line(&utm_data, scn, idx);
    led_tick(fix_ok, ble_healthy());
  }

  led_service();
  squitter.transmit(&utm_data);
}
