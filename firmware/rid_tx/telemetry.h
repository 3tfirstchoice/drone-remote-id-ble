// telemetry.h - log CSV qua Serial, 1 dong/giay (dinh dang: firmware/README.md, muc "Log Serial")
#pragma once
#include <Arduino.h>
#include "utm.h"

#define SCN_NONE  (-1)   // gia tri cot scn khi khong chay kich ban gia lap (che do GPS)

void   telemetry_header(void);                                  // in "H,<ten cac cot>"
void   telemetry_line(const UTM_data *d, int scn, size_t idx);  // in 1 dong "D,..."

// Ham thuan (khong dung Serial/millis): ghi 1 dong "D,..." vao buf, tra ve so ky tu.
// Tach rieng de test tren may tinh va de header/dong du lieu luon cung so cot.
size_t telemetry_format(char *buf, size_t n, const UTM_data *d, int scn, size_t idx,
                        uint32_t ms, uint32_t tx, uint32_t err, uint32_t heap);
