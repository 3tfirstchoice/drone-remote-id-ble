// led_test.ino - thu LED GPIO48 o che do NeoPixel va LED thuong
void setup() {
  Serial.begin(115200);
  delay(2000);
}
void loop() {
  Serial.println("1) NeoPixel DO sang 4 giay");
  neopixelWrite(48, 60, 0, 0);  delay(4000);
  neopixelWrite(48, 0, 0, 0);   delay(1500);

  Serial.println("2) NeoPixel XANH DUONG sang 4 giay");
  neopixelWrite(48, 0, 0, 60);  delay(4000);
  neopixelWrite(48, 0, 0, 0);   delay(1500);

  Serial.println("3) LED thuong: HIGH 3 giay, roi LOW 3 giay");
  pinMode(48, OUTPUT);
  digitalWrite(48, HIGH);       delay(3000);
  digitalWrite(48, LOW);        delay(3000);

  Serial.println("--- het mot vong ---");
}