// mock_source.h - nguon du lieu gia lap (USE_GPS 0) tu waypoints.h
#pragma once
#include <Arduino.h>
#include "utm.h"

// Chon kich ban 0 = dung yen, 1 = bay thang, 2 = hinh chu nhat va quay ve diem 0
void   mock_select(int scenario);

// Dien UTM_data bang diem hien tai va chuyen sang diem ke tiep
void   mock_step(UTM_data *d);

int    mock_scenario(void);   // kich ban dang chay
size_t mock_index(void);      // chi so diem se phat o lan mock_step ke tiep
