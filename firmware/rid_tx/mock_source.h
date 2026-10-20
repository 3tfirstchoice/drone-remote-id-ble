// mock_source.h - nguon du lieu gia lap (USE_GPS 0): doc 3 kich ban trong waypoints.h
#pragma once
#include <Arduino.h>
#include "utm.h"

// Chon kich ban: 0 = dung yen, 1 = bay thang, 2 = hinh chu nhat. Gia tri khac bi bo qua.
// Dat chi so ve diem 0 va in dong "# scenario=..." ra Serial.
void   mock_select(int scenario);

// Dien UTM_data bang diem hien tai roi tien sang diem ke tiep. Goi 1 lan moi DATA_PERIOD_MS.
// Kich ban 0 va 2 lap lai; kich ban 1 dung o diem cuoi voi toc do 0.
void   mock_step(UTM_data *d);

int    mock_scenario(void);   // kich ban dang chay
size_t mock_index(void);      // chi so diem SE duoc phat o lan mock_step ke tiep
