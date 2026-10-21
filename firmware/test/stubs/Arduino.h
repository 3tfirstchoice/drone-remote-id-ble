// Arduino.h gia lap cho test tren may tinh
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <string>

typedef uint8_t byte;
#define PI      3.1415926535897932384626433832795
#define TWO_PI  6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define radians(deg) ((deg) * DEG_TO_RAD)
#define degrees(rad) ((rad) * RAD_TO_DEG)
#define sq(x) ((x) * (x))
#define HIGH 1
#define LOW  0
#define OUTPUT 1
#define SERIAL_8N1 0x800001c
#define SERIAL_7N1 0x8000014

// --- thoi gian va chan ---
extern uint32_t stub_millis;
inline uint32_t millis() { return stub_millis; }
inline void delay(uint32_t ms) { stub_millis += ms; }
inline void pinMode(int, int) {}
extern int stub_pin_level[64];
inline void digitalWrite(int pin, int v) { stub_pin_level[pin] = v; }

// --- UART gia lap ---
struct StubSerial {
  std::string rx;               // du lieu nhan, nap bang feed()
  size_t      pos = 0;
  uint32_t    baud = 0, cfg = 0; int rx_pin = -1, tx_pin = -1; size_t rxbuf = 0;
  std::string out;              // du lieu da in
  void   begin(uint32_t b, uint32_t c = SERIAL_8N1, int r = -1, int t = -1) { baud = b; cfg = c; rx_pin = r; tx_pin = t; }
  size_t setRxBufferSize(size_t n) { rxbuf = n; return n; }
  void   end() {}
  int    available() { return (int) (rx.size() - pos); }
  int    read() { return pos < rx.size() ? (unsigned char) rx[pos++] : -1; }
  void   feed(const std::string &s) { rx += s; }
  void   clear() { rx.clear(); pos = 0; out.clear(); }
  void   print(const char *s) { out += s; }
  void   println(const char *s = "") { out += s; out += "\r\n"; }
  void   printf(const char *f, ...) { char b[512]; va_list a; va_start(a, f); vsnprintf(b, sizeof b, f, a); va_end(a); out += b; }
  operator bool() const { return true; }
};
extern StubSerial Serial, Serial1;
struct StubESP { uint32_t getFreeHeap() { return 265600; } };
extern StubESP ESP;
inline int esp_reset_reason() { return 1; }
