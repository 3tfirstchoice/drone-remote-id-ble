// config.h - moi hang so cua firmware rid_tx dat o day
#pragma once

#define FW_VERSION       "0.2.0"

// --- Nguon du lieu: 0 = gia lap tu waypoints.h, 1 = GPS NEO-6M (giai doan 2) ---
#define USE_GPS          0

// --- Dinh danh ---
#define UAV_ID           "LAB-ESP32-0001"
#define OPERATOR_ID      "LAB-OPERATOR-01"
#define SELF_ID_TEXT     "LAB SIMULATION"

// --- Vi tri nguoi dieu khien (home) va gio bat dau ---
#define HOME_LAT         10.7725
#define HOME_LON         106.6578
#define HOME_ALT_MSL     10.0f
#define SCENARIO_EPOCH   1791849600UL      // 2026-10-13 00:00:00 UTC

// --- Nhip va thoi gian ---
#define DATA_PERIOD_MS   1000              // cap nhat du lieu + in log 1 Hz
#define BOOT_DELAY_MS    500               // USB native: cho may tinh nhan cong COM
#define MOCK_SATS        10                // phai >= 7, neu khong thu vien khong phat Location

// --- Chan (xem BOARD_PINS.md) ---
#define STATUS_LED_PIN   48                // D13, den xanh la, sang khi HIGH
#define GPS_RX_PIN       6                 // D3: ESP32 nhan <- TX cua GPS
#define GPS_TX_PIN       7                 // D4: ESP32 gui  -> RX cua GPS
#define GPS_BAUD         9600
