# firmware-environment

- OS: Windows
- Arduino IDE: 2.3.10
- Core: esp32 by Espressif Systems 2.0.17 (build chinh)
- Goi OhStem 3.1.3: chi dung de doc ten chan bang pin_probe, khong dung de build firmware chinh
- Board: ESP32S3 Dev Module
- Tools: USB CDC On Boot = Enabled, USB Mode = Hardware CDC and JTAG, Flash Size = 16MB, PSRAM = OPI PSRAM, Upload Speed = 921600 (115200 neu loi)
- Nap code: neu khong ket noi duoc, giu BOOT, bam RESET, tha RESET, tha BOOT, roi Upload
- Sketchbook location: <repo>\firmware

## lib_test (Phan B)
- Board: ESP32S3 Dev Module, core esp32 2.0.17, Partition Scheme Default 4MB with spiffs
- FQBN: esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi
- Thu vien: id_open 1.2, utm 1.1, ESP32 BLE Arduino 2.0.0
- Verify: Sketch uses 890033 bytes (67%) of 1310720; global variables 45916 bytes (14%)
- Serial: "# lib_test start", "[BLE] init ok, tx power level enum=11", "# lib_test init done"
- Luu y: build bang board OhStem (ESP-IDF 5.3) bi loi wifi.c:16 (clock_gettime), nen firmware chinh phai build bang ESP32S3 Dev Module + core 2.0.17