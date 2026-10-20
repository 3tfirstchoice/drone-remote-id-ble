# firmware/tools - sketch va script phu tro

Moi sketch la 1 chuong trinh Arduino doc lap (mo thu muc cua no trong Arduino IDE roi nap), khong dung code trong `rid_tx/`.
Firmware chinh nam o `firmware/rid_tx/`.

| Ten | Loai | Dung de | Khi nao dung |
|---|---|---|---|
| `gps_passthrough/` | sketch | Do baud, dinh dang UART va dao RX/TX cua module GPS; in tom tat fix / ve tinh / hdop | Truoc khi bat `USE_GPS 1`, hoac khi GPS khong co du lieu |
| `board_test/` | sketch | In thong tin chip, flash, PSRAM | Kiem tra board moi |
| `pin_probe/` | sketch | In so GPIO cua nhan D2..D13, A0..A5 (build bang goi board OhStem) | Da dung de lap `BOARD_PINS.md` |
| `led_test/` | sketch | Thu den GPIO48 la NeoPixel hay LED thuong | Da dung de chon `STATUS_LED_PIN` |
| `lib_test/` | sketch | Thu vien id_open bien dich va khoi tao BLE duoc tren S3 | Kiem tra moi truong build |
| `first_packet/` | sketch | Ban phat Remote ID tinh dau tien (fw-v0.1, T01), giu nguyen lam bang chung | Chi de doi chieu |
| `capture_log.ps1` | PowerShell | Doc log Serial cua `rid_tx`, ghi cac dong `D,` ra CSV | Ghi bang chung T02-T04, T06 |
| `t05_measure.ps1` | PowerShell | Do thoi gian tu luc cap nguon lai den goi BLE dau tien (dong `# first_tx`) | Do T05 |

Test tren may tinh (khong can board) nam o `firmware/test/`, chay `bash firmware/test/run_tests.sh`.
