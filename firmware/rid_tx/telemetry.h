// telemetry.h - log CSV qua Serial
#pragma once
#include <Arduino.h>
#include "utm.h"

#define SCN_NONE  (-1)   // cot scn o che do GPS

void   telemetry_header(void);                                  // in dong header
void   telemetry_line(const UTM_data *d, int scn, size_t idx);  // in dong du lieu

// Ghi dong du lieu vao buf, tra ve so ky tu
size_t telemetry_format(char *buf, size_t n, const UTM_data *d, int scn, size_t idx,
                        uint32_t ms, uint32_t tx, uint32_t err, uint32_t heap);
