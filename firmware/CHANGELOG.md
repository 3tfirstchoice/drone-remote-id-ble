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
