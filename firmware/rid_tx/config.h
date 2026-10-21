// config.h - moi hang so cua firmware rid_tx dat o day
#pragma once

#define FW_VERSION       "0.4.2"

// --- Nguon du lieu: 0 = gia lap tu waypoints.h, 1 = GPS that (giai doan 2) ---
// USE_GPS 1 can thu vien "TinyGPSPlus" (Mikal Hart): Arduino IDE > Library Manager.
// Co the ghi de khi bien dich: -DUSE_GPS=1 (dung cho test tren may tinh).
#ifndef USE_GPS
#define USE_GPS          0
#endif

// --- Dinh danh ---
#define UAV_ID           "LAB-ESP32-0001"
// Operator ID theo dinh dang so dang ky EU: 3 chu cai quoc gia + 13 ky tu, dau '-', 3 ky tu bi mat (tong 20).
// Gia tri nay la GIA, chi de thu nghiem. Dinh dang cu "LAB-OPERATOR-01" bi Drone Scanner canh bao khong hop le.
#define OPERATOR_ID      "VNM87labtest0001-abc"
#define SELF_ID_TEXT     "LAB SIMULATION"

// --- Vi tri nguoi dieu khien (home) ---
// Che do gia lap dung 3 hang so nay. Che do GPS lay home tu lan fix dau tien (gps_home).
#define HOME_LAT         10.7725
#define HOME_LON         106.6578
#define HOME_ALT_MSL     10.0f
// SCENARIO_EPOCH (gio bat dau gia lap) nam trong waypoints.h, do data/generate_mock_data.py sinh

// --- Nhip va thoi gian ---
#define DATA_PERIOD_MS   1000              // cap nhat du lieu + in log 1 Hz
#define BOOT_DELAY_MS    500               // USB native: cho may tinh nhan cong COM
#define MOCK_SATS        10                // phai >= 7, neu khong thu vien khong phat Location
#define FIRST_TX_WARN_MS 5000              // qua muc nay ma chua co goi BLE nao thi in canh bao

// --- Chan (xem BOARD_PINS.md) ---
#define STATUS_LED_PIN   48                // D13, den xanh la, sang khi HIGH
#define GPS_RX_PIN       6                 // D3: ESP32 nhan <- TX cua GPS
#define GPS_TX_PIN       7                 // D4: ESP32 gui  -> RX cua GPS (firmware khong gui lenh nao)

// --- GPS: M5Stack GPS/BDS Unit v1.1 (AT6668), UART mac dinh 115200 8N1 ---
// Dung tools/gps_passthrough de xac nhan baud/dinh dang truoc khi doi cac gia tri nay.
#define GPS_BAUD          115200
#define GPS_UART_7N1      0                // 1 neu gps_passthrough bao module chay 7N1
#define GPS_FIX_MAX_AGE_MS  2000           // du lieu GPS cu hon muc nay coi nhu mat fix
#define GPS_CLOCK_SYNC_MS   60000          // dong bo lai dong ho he thong theo gio GPS moi 60 giay
#define GPS_MIN_SPEED_KN    1              // cham hon muc nay thi giu huong cu (huong GPS khong tin cay)

// --- Kiem tra cau hinh luc bien dich (sai thi bao loi ngay, khong cho nap) ---
static_assert(sizeof(UAV_ID) > 1,      "UAV_ID khong duoc rong");
static_assert(sizeof(OPERATOR_ID) > 1, "OPERATOR_ID khong duoc rong");
static_assert(sizeof(UAV_ID) - 1      <= 20, "UAV_ID toi da 20 ky tu (ODID_ID_SIZE)");
static_assert(sizeof(OPERATOR_ID) - 1 <= 20, "OPERATOR_ID toi da 20 ky tu (ODID_ID_SIZE)");
static_assert(sizeof(SELF_ID_TEXT) - 1 <= 23, "SELF_ID_TEXT toi da 23 ky tu (ODID_STR_SIZE)");
static_assert(HOME_LAT >= -90.0  && HOME_LAT <= 90.0,   "HOME_LAT ngoai mien WGS84");
static_assert(HOME_LON >= -180.0 && HOME_LON <= 180.0,  "HOME_LON ngoai mien WGS84");
static_assert(MOCK_SATS >= 7, "MOCK_SATS phai >= 7, neu khong thu vien khong phat Location");
static_assert(DATA_PERIOD_MS >= 100, "DATA_PERIOD_MS qua nho");
