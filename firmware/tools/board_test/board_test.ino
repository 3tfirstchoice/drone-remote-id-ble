// board_test.ino - in thong tin chip, flash, PSRAM cua board (ket qua ghi trong BOARD_PINS.md)
void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== BOARD TEST ===");
  Serial.printf("Chip: %s rev %d, cores %d\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("CPU MHz: %d\n", getCpuFrequencyMhz());
  Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("PSRAM size: %u bytes\n", ESP.getPsramSize());
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.println("=== DONE ===");
}
void loop() { delay(1000); }