// ble_stats.h - bo dem goi BLE, dinh nghia trong libraries/id_open/id_open_esp32.cpp (PATCH #2)
// Khai bao extern o MOT cho nay; rid_tx.ino va telemetry.cpp deu include file nay.
#pragma once
#include <stdint.h>

extern uint32_t ble_tx_cnt;    // so lan start_advertising thanh cong
extern uint32_t ble_err_cnt;   // so lan config/start_advertising bao loi
