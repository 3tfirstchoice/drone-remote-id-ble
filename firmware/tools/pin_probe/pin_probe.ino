void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== PIN PROBE ===");
#define P(x) do { Serial.printf(#x " = %d\n", (int)(x)); } while (0)
  P(D2); P(D3); P(D4); P(D5); P(D6); P(D7);
  P(D8); P(D9); P(D10); P(D11); P(D12); P(D13);
  P(A0); P(A1); P(A2); P(A3); P(A4); P(A5);
  P(RX); P(TX); P(SDA); P(SCL);
#ifdef LED_BUILTIN
  P(LED_BUILTIN);
#endif
#ifdef RGB_BUILTIN
  P(RGB_BUILTIN);
#endif
#ifdef PIN_NEOPIXEL
  P(PIN_NEOPIXEL);
#endif
  Serial.println("=== DONE ===");
}
void loop() { delay(1000); }