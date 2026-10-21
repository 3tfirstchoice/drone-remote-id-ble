// config.h - hang so cau hinh cua firmware rid_tx
#pragma once

#define FW_VERSION       "0.4.3"

// --- Nguon du lieu: 0 = gia lap (waypoints.h), 1 = GPS that (can thu vien TinyGPSPlus) ---
#ifndef USE_GPS
#define USE_GPS          0
#endif

// --- Dinh danh ---
#define UAV_ID           "LAB-ESP32-0001"
#define OPERATOR_ID      "LAB-OPERATOR-01"
#define SELF_ID_TEXT     "LAB SIMULATION"

// --- Vi tri nguoi dieu khien (home), dung o che do gia lap ---
#define HOME_LAT         10.7725
#define HOME_LON         106.6578
#define HOME_ALT_MSL     10.0f

// --- Nhip va thoi gian ---
#define DATA_PERIOD_MS   1000              // chu ky cap nhat du lieu va in log (ms)
#define BOOT_DELAY_MS    500               // cho cong USB san sang (ms)
#define MOCK_SATS        10                // so ve tinh gia lap, >= 7
#define FIRST_TX_WARN_MS 5000              // nguong canh bao chua phat duoc goi BLE (ms)

// --- Chan ---
#define STATUS_LED_PIN   48                // D13, LED xanh, sang khi HIGH
#define GPS_RX_PIN       6                 // D3, noi TX cua GPS
#define GPS_TX_PIN       7                 // D4, noi RX cua GPS

// --- GPS (M5Stack GPS/BDS Unit v1.1) ---
#define GPS_BAUD          115200
#define GPS_UART_7N1      0                // 1: UART 7N1, 0: UART 8N1
#define GPS_FIX_MAX_AGE_MS  2000           // du lieu cu hon muc nay coi nhu mat fix (ms)
#define GPS_CLOCK_SYNC_MS   60000          // chu ky dong bo dong ho theo gio GPS (ms)
#define GPS_MIN_SPEED_KN    1              // duoi muc nay giu huong cu (kn)

// --- Kiem tra cau hinh luc bien dich ---
static_assert(sizeof(UAV_ID) > 1,      "UAV_ID khong duoc rong");
static_assert(sizeof(OPERATOR_ID) > 1, "OPERATOR_ID khong duoc rong");
static_assert(sizeof(UAV_ID) - 1      <= 20, "UAV_ID toi da 20 ky tu (ODID_ID_SIZE)");
static_assert(sizeof(OPERATOR_ID) - 1 <= 20, "OPERATOR_ID toi da 20 ky tu (ODID_ID_SIZE)");
static_assert(sizeof(SELF_ID_TEXT) - 1 <= 23, "SELF_ID_TEXT toi da 23 ky tu (ODID_STR_SIZE)");
static_assert(HOME_LAT >= -90.0  && HOME_LAT <= 90.0,   "HOME_LAT ngoai mien WGS84");
static_assert(HOME_LON >= -180.0 && HOME_LON <= 180.0,  "HOME_LON ngoai mien WGS84");
static_assert(MOCK_SATS >= 7, "MOCK_SATS phai >= 7, neu khong thu vien khong phat Location");
static_assert(DATA_PERIOD_MS >= 100, "DATA_PERIOD_MS qua nho");
