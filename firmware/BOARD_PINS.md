# BOARD_PINS - OhStem YOLO UNO (ESP32-S3-WROOM-1 N16R8)

Nguon: output firmware/tools/pin_probe (goi board OhStem 3.1.3) va firmware/tools/led_test, ngay 13/10/2026.

| Nhan | GPIO | Ghi chu |
|---|---|---|
| D2, D3, D4 | 5, 6, 7 | Tu do |
| D5, D6, D7 | 8, 9, 10 | Tu do |
| D8, D9 | 17, 18 | Tu do |
| D10 | 21 | Tu do |
| D11, D12 | 38, 47 | Tu do |
| D13 | 48 | LED xanh la tren board, sang khi HIGH (da xac nhan bang led_test) |
| A0, A1, A2, A3 | 1, 2, 3, 4 | GPIO3 la strapping, tranh dung |
| A4 (SDA), A5 (SCL) | 11, 12 | I2C |
| RX, TX | 44, 43 | UART0 |

Khong co NeoPixel (neopixelWrite tren GPIO48 khong lam den doi mau). Den do tren board la den nguon.
Goi OhStem khong dinh nghia LED_BUILTIN / RGB_BUILTIN / PIN_NEOPIXEL.

## Quyet dinh cho firmware
- STATUS_LED_PIN = 48 (digitalWrite, 1 mau, bao trang thai bang mau nhay)
- GPS_RX_PIN = 6 (D3): ESP32 nhan, noi vao chan TX cua module GPS
- GPS_TX_PIN = 7 (D4): ESP32 gui, noi vao chan RX cua module GPS
- GPS_BAUD = 115200, 8N1 (M5Stack GPS/BDS Unit v1.1 AT6668; module cu NEO-6M dung 9600). Dung tools/gps_passthrough de xac nhan
- Day Grove cua module: den = GND, do = 5V, vang va trang = 2 day UART. Tai lieu M5Stack khong ghi ro day nao la TX nhin tu phia module, nen tools/gps_passthrough tu thu ca 2 cach noi va bao neu phai dao

## Phan cung da xac nhan (board_test)
- Chip: ESP32-S3 (v0.2), 2 nhan, 240 MHz
- Flash: 16 MB (16777216 byte)
- PSRAM: 8 MB OPI (8386295 byte)
- Cong COM khi cam USB: COM8 (co the doi may)
## Dia chi BLE (do bang nRF Connect, 13/10/2026)
- F0:F5:BD:45:1B:0D