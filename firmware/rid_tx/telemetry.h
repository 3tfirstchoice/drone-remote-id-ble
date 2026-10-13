// telemetry.h - log CSV qua Serial, 1 dong/giay
#pragma once
#include <Arduino.h>
#include "id_open.h"

void telemetry_header(void);
void telemetry_line(const UTM_data *d, int scn, size_t idx);
