// test_rid_tx.cpp - bien dich rid_tx.ino tren may tinh va kiem tra phan noi day trong setup()/loop().
// run_tests.sh build file nay 2 lan: -DUSE_GPS=0 (gia lap) va -DUSE_GPS=1 (GPS that).
// Cac test chay NOI TIEP tren cung 1 lan khoi dong (bien static trong loop() khong reset duoc).
#include "rid_tx.ino"
#include "check.h"
#include "nmea_fixture.h"
#include <vector>

// ---------------------------------------------------------------- tien ich
static const uint32_t STEP_MS  = 5;     // moi vong loop() gia lap cach nhau 5 ms
static const uint32_t SETUP_MS = BOOT_DELAY_MS;
// Vong loop() dau tien chay o SETUP_MS + STEP_MS -> nhip 1 Hz roi vao FIRST_TICK + k * 1000
static const uint32_t FIRST_TICK = SETUP_MS + STEP_MS + DATA_PERIOD_MS;

static void run_until(uint32_t t_ms) {
  while (stub_millis + STEP_MS <= t_ms) { stub_millis += STEP_MS; loop(); }
}
static uint32_t tick_at(int k) { return FIRST_TICK + (uint32_t) k * DATA_PERIOD_MS; }   // nhip thu k

static void send_cmd(const char *s) { Serial.feed(s); run_until(stub_millis + STEP_MS); }

static bool has(const char *text) { return Serial.out.find(text) != std::string::npos; }

static int count(const char *text) {
  int n = 0;
  for (size_t p = Serial.out.find(text); p != std::string::npos; p = Serial.out.find(text, p + 1)) n++;
  return n;
}

// Cac dong "D,..." trong Serial.out, moi dong tach thanh cac truong
static std::vector<std::vector<std::string>> data_lines(void) {
  std::vector<std::vector<std::string>> rows;
  size_t p = 0;
  while ((p = Serial.out.find("D,", p)) != std::string::npos) {
    if (p > 0 && Serial.out[p - 1] != '\n') { p++; continue; }
    const size_t e = Serial.out.find('\n', p);
    std::string line = Serial.out.substr(p, e - p);
    std::vector<std::string> f;
    size_t a = 0, b;
    while ((b = line.find(',', a)) != std::string::npos) { f.push_back(line.substr(a, b - a)); a = b + 1; }
    f.push_back(line.substr(a));
    rows.push_back(f);
    p = e;
  }
  return rows;
}
// Chi so cot (D = 0): ms=1 utc=2 scn=3 idx=4 lat=5 lon=6 alt_msl=7 alt_agl=8 speed=9 heading=10 sats=11
enum { F_MS = 1, F_UTC, F_SCN, F_IDX, F_LAT, F_LON, F_ALT_MSL, F_ALT_AGL, F_SPEED, F_HEADING, F_SATS };

// ---------------------------------------------------------------- test chung cho 2 che do
static void test_setup(void) {
  stub_millis = 0;
  Serial.clear();
  stub_reset_clock();
  setup();

  char boot[64];
  snprintf(boot, sizeof boot, "# rid_tx boot fw=%s use_gps=%d", FW_VERSION, USE_GPS);
  CHECK(has(boot));
  CHECK(has("# identity uav=LAB-ESP32-0001 operator=LAB-OPERATOR-01 self_id=LAB SIMULATION\n"));
  CHECK(has("H,ms,utc,scn,idx,lat,lon,alt_msl,alt_agl,speed_kn,heading,sats,tx,err,heap\r\n"));
  CHECK_STR(squitter.params.UAV_id, UAV_ID);
  CHECK_STR(squitter.params.UAS_operator, OPERATOR_ID);
  CHECK(squitter.self_id == SELF_ID_TEXT);
  CHECK(squitter.params.UA_type == ODID_UATYPE_HELICOPTER_OR_MULTIROTOR);
  CHECK(stub_settime_calls == 1);
  CHECK((unsigned long) stub_settime_last.tv_sec == SCENARIO_EPOCH);   // dong ho bat dau o moc gia lap
  CHECK(stub_pin_level[STATUS_LED_PIN] == LOW);
  CHECK(stub_millis == SETUP_MS);
}

static void test_one_line_per_second(void) {
  Serial.clear();
  run_until(tick_at(8));                               // 9 nhip: k = 0..8
  auto rows = data_lines();
  CHECK(rows.size() == 9);
  for (size_t i = 1; i < rows.size(); i++) {
    CHECK(atol(rows[i][F_MS].c_str()) - atol(rows[i - 1][F_MS].c_str()) == (long) DATA_PERIOD_MS);
  }
  CHECK(squitter.transmit_calls == (int) (tick_at(8) - SETUP_MS) / (int) STEP_MS);   // moi vong loop() 1 lan
}

static void test_ble_warning_then_first_tx(void) {
  // ble_tx_cnt = 0 tu dau -> sau FIRST_TX_WARN_MS phai co dung 1 canh bao, chua co first_tx
  CHECK(stub_millis > FIRST_TX_WARN_MS);
  CHECK(count("# CANH BAO: sau 5000 ms van chua phat duoc goi BLE nao") == 1);
  CHECK(!has("# first_tx"));
  ble_tx_cnt = 1;                                      // BLE len lai
  run_until(stub_millis + 100);
  CHECK(count("# first_tx ms=") == 1);
  run_until(stub_millis + 2000);
  CHECK(count("# first_tx ms=") == 1);                 // chi in 1 lan
  CHECK(count("# CANH BAO") == 1);
}

static void test_led_ble_error_then_recovery(void) {
  run_until(tick_at(12) - 100);
  ble_err_cnt++;                                       // 1 loi BLE trong giay nay
  run_until(tick_at(12) + 450);
  CHECK(stub_pin_level[STATUS_LED_PIN] == HIGH);       // nhay nhanh: sang o 400-499 ms
  run_until(tick_at(13) + 450);
  CHECK(stub_pin_level[STATUS_LED_PIN] == LOW);        // khong co loi moi: het nhay nhanh
}

static void test_common_commands(void) {
  Serial.clear();
  send_cmd("v");
  char v[48];
  snprintf(v, sizeof v, "# fw=%s use_gps=%d\n", FW_VERSION, USE_GPS);
  CHECK(has(v));
  send_cmd("h\r\n");
  CHECK(has("H,ms,utc,"));
  send_cmd("x");                                       // lenh la: bo qua, khong in gi
}

// ---------------------------------------------------------------- che do gia lap
#if !USE_GPS
static std::string utc_string(time_t t) {
  struct tm g; gmtime_r(&t, &g);
  char b[32];
  strftime(b, sizeof b, "%Y-%m-%dT%H:%M:%SZ", &g);
  return b;
}

static void test_mock_mode(void) {
  // setup(): home tu config.h, kich ban 0
  CHECK(utm_data.base_valid == 1);
  CHECK_NEAR(utm_data.base_latitude, HOME_LAT, 1e-12);
  CHECK_NEAR(utm_data.base_longitude, HOME_LON, 1e-12);

  Serial.clear();
  send_cmd("?");
  CHECK(has("# 1=static 2=straight 3=rectangle r=restart h=header v=version\r\n"));

  send_cmd("2");
  CHECK(has("# scenario=1 name=straight len=30\n"));
  Serial.clear();
  run_until(tick_at(16) + 10);                         // 2 nhip sau khi doi kich ban
  auto rows = data_lines();
  CHECK(rows.size() >= 2);
  if (rows.size() >= 2) {
    CHECK(rows[0][F_SCN] == "1" && rows[0][F_IDX] == "0");
    CHECK(rows[1][F_SCN] == "1" && rows[1][F_IDX] == "1");
    CHECK(rows[0][F_LAT] == "10.7725000" && rows[1][F_LAT] == "10.7725462");
    CHECK(rows[1][F_SPEED] == "10" && rows[1][F_SATS] == "10");
    // gio trong log = SCENARIO_EPOCH + so giay da troi tu luc setup() dat dong ho
    const time_t expect = (time_t) SCENARIO_EPOCH + (atol(rows[1][F_MS].c_str()) - SETUP_MS) / 1000;
    CHECK(rows[1][F_UTC] == utc_string(expect));
  }

  send_cmd("r");
  CHECK(has("# scenario=1 name=straight len=30\n"));   // r = chay lai kich ban hien tai
  send_cmd("3");
  CHECK(has("# scenario=2 name=rectangle len=40\n"));
}
#endif

// ---------------------------------------------------------------- che do GPS
#if USE_GPS
static void test_gps_mode(void) {
  CHECK(Serial1.baud == 115200 && Serial1.rx_pin == GPS_RX_PIN && Serial1.tx_pin == GPS_TX_PIN);
  CHECK(utm_data.base_valid == 0);                     // chua co home cho den khi fix

  Serial.clear();
  send_cmd("?");
  CHECK(has("# h=header v=version\r\n"));
  send_cmd("1");
  CHECK(!has("# scenario"));                           // lenh kich ban khong co tac dung o che do GPS

  // Chua fix: cot scn = -1, sats = 0, den nhay 2 lan (sang o 200-279 ms)
  Serial.clear();
  run_until(tick_at(16) + 250);
  auto rows = data_lines();
  CHECK(!rows.empty() && rows.back()[F_SCN] == "-1" && rows.back()[F_SATS] == "0");
  CHECK(stub_pin_level[STATUS_LED_PIN] == HIGH);

  // Co fix: home duoc chot, dong ho theo gio GPS, den ve 1 lan nhay
  Serial1.feed(gga(1, 10, "0.9", "10.0") + rmc('A', "0.0", "0.0"));
  Serial.clear();
  run_until(tick_at(17) + 250);
  CHECK(utm_data.base_valid == 1);
  CHECK_NEAR(utm_data.base_latitude, 10.7725, 1e-9);
  CHECK_NEAR(utm_data.base_longitude, 106.6578, 1e-9);
  rows = data_lines();
  CHECK(rows.size() == 1);
  if (!rows.empty()) {
    CHECK(rows[0][F_LAT] == "10.7725000" && rows[0][F_SATS] == "10");
    CHECK(rows[0][F_UTC].rfind("2026-10-20T03:00:0", 0) == 0);   // gio lay tu cau RMC
  }
  CHECK(stub_pin_level[STATUS_LED_PIN] == LOW);

  // Mat fix (khong con cau moi qua GPS_FIX_MAX_AGE_MS): sats = 0, home giu nguyen
  Serial.clear();
  run_until(tick_at(20) + 10);
  rows = data_lines();
  CHECK(!rows.empty() && rows.back()[F_SATS] == "0");
  CHECK(utm_data.base_valid == 1);
}
#endif

int main(void) {
  RUN(test_setup);
  RUN(test_one_line_per_second);
  RUN(test_ble_warning_then_first_tx);
  RUN(test_led_ble_error_then_recovery);
  RUN(test_common_commands);
#if USE_GPS
  RUN(test_gps_mode);
  return report("rid_tx.ino (USE_GPS 1)");
#else
  RUN(test_mock_mode);
  return report("rid_tx.ino (USE_GPS 0)");
#endif
}
