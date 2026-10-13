// mock_source.h - nguon du lieu gia lap tu waypoints.h
#pragma once
#include <Arduino.h>
#include "id_open.h"

void        mock_select(int scenario);   // 0 = tinh, 1 = thang, 2 = chu nhat
void        mock_step(UTM_data *d);      // goi moi DATA_PERIOD_MS
int         mock_scenario(void);
size_t      mock_index(void);            // chi so diem SE duoc phat o lan mock_step ke tiep
const char *mock_name(void);
