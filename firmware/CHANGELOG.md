# CHANGELOG - firmware

## fw-v0.1-first-packet (2026-10-13)
- Phat Remote ID tinh qua BLE Legacy Advertising tren YOLO UNO (ESP32-S3), sketch firmware/tools/first_packet
- Ban tin: Basic ID, Location/Vector, Self ID, System, Operator ID
- T01 dat:
  - nRF Connect: Service Data 0xFFFA, ma ung dung 0x0D, ban tin Location giai ma dung lat 10.7725000, lon 106.6578000, alt MSL 10.0 m, AGL 0, toc do 0, huong 0
  - Drone Scanner (Android, BT 4 Legacy, -52 dBm o 1 m): ID LAB-ESP32-0001, Multirotor, vi tri pilot tai diem cat canh, marker tai ĐH Bach Khoa, Self ID LAB SIMULATION
  - Serial: err=0, ble_tx_cnt tang khoang 6 moi giay
- Luu y: vi tri la du lieu gia (hang so HOME_LAT/HOME_LON trong code), khong phai vi tri that cua board
- Van de da biet:
  - Drone Scanner bao Operator ID khong hop le (dinh dang LAB-OPERATOR-01 khong theo chuan dang ky EU), khong anh huong T01
  - Thu vien quang ba o che do connectable (ADV_TYPE_IND), de bai yeu cau non-connectable; du kien ban va #4

## fw-v0.2-scenarios (2026-10-13)
- Firmware day du `firmware/rid_tx`: 3 kich ban (dung yen, bay thang huong Bac, hinh chu nhat) doc tu `waypoints.h`
- Log CSV 1 Hz qua Serial, lenh Serial 1/2/3/r/h/v/?, den xanh GPIO48 bao trang thai bang so lan nhay
- `waypoints.h` sinh boi `tools/gen_waypoints_tmp.py`; `tools/capture_log.ps1` ghi log ra file
- Ban va #4: quang ba non-connectable (ADV_TYPE_NONCONN_IND), thay ADV_TYPE_IND
- T02 dat: 60 dong D, ms cach nhau dung 1000, tx tang 6 goi/giay, err=0, heap on dinh 265600 byte
- T03 dat: lat tang deu 0.0000462 do/giay, lon khong doi, speed_kn=10, heading=0
- T04 dat: heading 90, 0, 270, 180 lap lai 2 vong, diem cuoi vong trung diem dau
- Luu y: toc do 10 kn = 5.144 m/s, ODID ma hoa buoc 0.25 m/s nen ung dung co the hien 5.0 hoac 5.25 m/s


## fw-v0.3-tested (2026-10-13)
- Patch #5 (id_open.cpp): last_msecs 2000 -> 300; BOOT_DELAY_MS 2000 -> 500
- Thoi gian tu luc chip khoi dong den goi BLE dau tien (first_tx): 2444 ms -> 943 ms
- Firmware in them dong `# first_tx ms=N`; them tools/t05_measure.ps1
- T05 dat (khoi phuc sau khi mat nguon), T06 dat (phat lien tuc khi dong/mo app, err=0)
- T07 (chay 30 phut) va T08 (cu ly) khong thuc hien

## fw-v0.4-gps (2026-10-20)
- Giai doan 2: `rid_tx/gps_source.cpp` doc GPS that (M5Stack GPS/BDS Unit v1.1, UART1 GPIO6/7, 115200 8N1) bang TinyGPSPlus; bat bang `USE_GPS 1`
- Home = fix dat chuan dau tien, AGL tinh tu home, giu huong khi cham, mat fix -> Location bao loi, dong bo dong ho he thong theo gio GPS
- `tools/gps_passthrough`: do baud/dinh dang va RX/TX tu dong, tom tat fix/ve_tinh/nhin_thay/hdop
- config.h: static_assert, `# identity` luc khoi dong, canh bao khi 5 s chua phat BLE; `ble_stats.h` thay cho khai bao extern lap lai; `telemetry_format()` tach rieng de test
- `firmware/test/run_tests.sh`: 1181 kiem tra tren may tinh (GPS, log, den bao, waypoints, byte Operator ID); da thu pha code co chu y de xac nhan test bat duoc loi
- Ghi chu T04: luc do T04 mang hinh chu nhat co 41 diem (diem cuoi trung diem dau); `waypoints.h` hien tai co 40 diem, vong lap 40 giay
- Operator ID: byte phat dung; canh bao cua Drone Scanner den tu kiem tra dinh dang o phia app (xem README)
- SCENARIO_EPOCH co dinh theo lan sinh waypoints (co chu y); che do GPS dung gio GPS
- Chua kiem chung tren board that: bien dich Arduino IDE, fix ngoai troi

## fw-v0.4.1-cleanup (2026-10-20)
- Khong doi hanh vi phat BLE va dinh dang dong log `H,` / `D,` (test xac nhan). Doi chu: dong canh bao BLE ghi "5000 ms", lenh `?` theo che do
- `rid_tx.ino`: lua chon gia lap / GPS gom vao 1 muc (`source_begin/poll/step/command`), `setup()`/`loop()` khong con `#if`; tach `report_first_tx()`, `ble_healthy()`; lenh `?` chi liet ke lenh cua che do dang chay; nguong canh bao BLE dua vao config.h (`FIRST_TX_WARN_MS`)
- `gps_source.cpp`: ham `is_fresh()` thay 6 cho kiem tra lap; bo xu ly HDOP/VDOP vi thu vien id_open khong doc 2 truong nay
- `mock_source`: include `utm.h` thay vi ca thu vien id_open; bo ham `mock_name()` khong dung; `status_led`: dung enum thay so 0/1/2
- `tools/gps_passthrough`: gom doan ghep cau NMEA lap 2 lan thanh `LineReader`; lenh `r` xoa ca so lieu GSV cu
- `firmware/test`: them test cho chinh `rid_tx.ino` o ca 2 che do (build 2 lan); gom bien gia lap vao `stubs/stubs.cpp`, cau NMEA vao `nmea_fixture.h`; kiem tra 2 ban waypoints.h giong nhau va cu phap gps_passthrough; da thu pha code co chu y de xac nhan test bat duoc loi
- Them `tools/README.md`, muc "Cau truc code" trong README; bo cac file .gitkeep thua
