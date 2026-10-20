// telemetry.h - log CSV qua Serial, 1 dong/giay
#pragma once
#include <Arduino.h>
#include "utm.h"

void   telemetry_header(void);
void   telemetry_line(const UTM_data *d, int scn, size_t idx);

// Ham thuan (khong dung Serial/millis): ghi 1 dong "D,..." vao buf, tra ve so ky tu.
// Tach rieng de test tren may tinh va de header/dong du lieu luon cung so cot.
size_t telemetry_format(char *buf, size_t n, const UTM_data *d, int scn, size_t idx,
                        uint32_t ms, uint32_t tx, uint32_t err, uint32_t heap);
