// rid_tx.ino - phat Remote ID (ASTM F3411 / OpenDroneID) qua BLE Legacy Advertising, ESP32-S3
//
// File nay chi noi cac module lai voi nhau, khong chua logic rieng cua module nao:
//   nguon du lieu : mock_source (gia lap tu waypoints.h, USE_GPS 0)
//                   gps_source  (GPS that M5Stack GPS/BDS Unit v1.1, USE_GPS 1)
//   phat BLE      : thu vien id_open (ID_OpenDrone::transmit tu chia nhip 75 ms)
//   log Serial    : telemetry       den bao: status_led       hang so: config.h
// So do luong du lieu: firmware/README.md, muc "Cau truc code".
#include "id_open.h"
#include "config.h"
#include "waypoints.h"      // SCENARIO_EPOCH
#include "ble_stats.h"      // ble_tx_cnt, ble_err_cnt (PATCH #2)
#include "telemetry.h"
#include "status_led.h"
#if USE_GPS
#include "gps_source.h"
#else
#include "mock_source.h"
#endif

// utm_data = "phieu du lieu" dung chung: nguon du lieu ghi vao, id_open doc ra de dong goi
static ID_OpenDrone   squitter;
static UTM_parameters utm_parameters;
static UTM_data       utm_data;

// ================================================================ thoi gian, dinh danh

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

// Ma hoa 1 lan cac ban tin khong doi (Basic ID, Operator ID, Self ID)
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
// Cho DUY NHAT chon giua gia lap va GPS that. setup()/loop() chi goi 4 ham source_*:
//   source_begin   : khoi tao, dien vi tri nguoi dieu khien (home) neu da biet
//   source_poll    : goi moi vong loop()
//   source_step    : goi 1 lan moi DATA_PERIOD_MS, dien utm_data; tra ve true khi vi tri hop le
//   source_command : xu ly lenh Serial rieng cua nguon; tra ve true neu da xu ly
#if USE_GPS

#define SOURCE_HELP ""

static void source_begin(UTM_data *d) {
  gps_begin();
  Serial.printf("# gps uart rx=GPIO%d tx=GPIO%d baud=%d %s, cho fix...\n",
                GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD, GPS_UART_7N1 ? "7N1" : "8N1");
  d->base_valid = 0;                   // home = fix dat chuan dau tien, xem source_step
}

static void source_poll(void) { gps_poll(); }

static bool source_step(UTM_data *d, int *scn, size_t *idx) {
  *scn = SCN_NONE;
  *idx = 0;
  const bool ok = gps_step(d);
  if (!d->base_valid && gps_home(&d->base_latitude, &d->base_longitude, &d->base_alt_m)) {
    d->base_valid = 1;                 // id_open ma hoa System message 1 lan khi base_valid len 1
  }
  return ok;
}

static bool source_command(char c) { (void) c; return false; }

#else  // gia lap

#define SOURCE_HELP "1=static 2=straight 3=rectangle r=restart "

static void source_begin(UTM_data *d) {
  mock_step(d);                        // dien san diem dau tien de goi BLE dau tien da co toa do
  mock_select(0);                      // dat lai chi so ve 0 va in dong "# scenario=0"
  d->base_latitude  = HOME_LAT;
  d->base_longitude = HOME_LON;
  d->base_alt_m     = HOME_ALT_MSL;
  d->base_valid     = 1;
}

static void source_poll(void) {}

static bool source_step(UTM_data *d, int *scn, size_t *idx) {
  *scn = mock_scenario();              // doc TRUOC mock_step: chi so cua diem sap phat
  *idx = mock_index();
  mock_step(d);
  return true;                         // du lieu gia lap luon co "fix"
}

static bool source_command(char c) {
  switch (c) {
    case '1': case '2': case '3': mock_select(c - '1');        return true;
    case 'r':                     mock_select(mock_scenario()); return true;
    default:                      return false;
  }
}

#endif

// ================================================================ lenh Serial (1 ky tu)
static void handle_serial(void) {
  while (Serial.available()) {
    const char c = Serial.read();
    if (source_command(c)) continue;
    switch (c) {
      case 'h': telemetry_header(); break;
      case 'v': Serial.printf("# fw=%s use_gps=%d\n", FW_VERSION, USE_GPS); break;
      case '?': Serial.println("# " SOURCE_HELP "h=header v=version"); break;
      default:  break;                 // bo qua \r \n
    }
  }
}

// ================================================================ tinh trang BLE

// In 1 lan thoi diem goi BLE dau tien ra (do T05). Neu qua FIRST_TX_WARN_MS van chua co goi nao
// thi in canh bao 1 lan (khong im lang), sau do van cho in first_tx neu BLE len lai.
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

// true neu trong chu ky vua qua khong co loi BLE moi
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

  struct timeval tv = { (time_t) SCENARIO_EPOCH, 0 };   // che do GPS: ghi de khi co gio that
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

  if (millis() - last_tick >= DATA_PERIOD_MS) {   // nhip 1 Hz, khong dung delay()
    last_tick += DATA_PERIOD_MS;
    int    scn;
    size_t idx;
    const bool fix_ok = source_step(&utm_data, &scn, &idx);
    fill_time(&utm_data);
    telemetry_line(&utm_data, scn, idx);
    led_tick(fix_ok, ble_healthy());
  }

  led_service();
  squitter.transmit(&utm_data);        // goi lien tuc, thu vien tu chia nhip 75 ms
}
