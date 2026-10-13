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