# Firmware - YOLO UNO (ESP32-S3) phat Remote ID qua BLE

Firmware `rid_tx` phat goi Remote ID (ASTM F3411 / OpenDroneID) bang BLE Legacy Advertising, Service Data UUID 0xFFFA.
Giai doan 1 phat du lieu gia lap tu `rid_tx/waypoints.h`. Giai doan 2 (USE_GPS 1) phat du lieu tu GPS NEO-6M.

Huong dan nap day du, loi thuong gap: se bo sung o ban phat hanh v1.0.

## Giao dien

### 1. waypoints.h

Moi phan tu la 1 diem, 1 giay mot diem. File duoc sinh boi `tools/gen_waypoints_tmp.py`, khong sua tay.

    typedef struct {
      int32_t  lat_e7, lon_e7;        // do * 1e7
      int16_t  alt_msl_dm, alt_agl_dm; // 0.1 m
      uint8_t  speed_kn;               // hai ly/gio, so nguyen
      uint16_t heading_deg;            // 0 = Bac, 90 = Dong
    } waypoint_t;

Ba mang: WP_STATIONARY (60 diem), WP_STRAIGHT (30 diem, huong Bac, 10 kn), WP_RECTANGLE (41 diem, 4 canh x 10 giay, dong-bac-tay-nam, diem cuoi = diem dau). Moi mang co macro `<TEN>_LEN`.

### 2. Log Serial

- 115200 baud, USB CDC, moi dong ket thuc bang `\n`
- Dong bat dau `#` la ghi chu; `H,` la header; `D,` la du lieu (1 dong/giay)
- Header: `H,ms,utc,scn,idx,lat,lon,alt_msl,alt_agl,speed_kn,heading,sats,tx,err,heap`
- `scn`: 0 dung yen, 1 bay thang, 2 chu nhat, -1 GPS
- Cong COM co the bien mat vai giay moi lan board reset
- Doc log bang `tools/capture_log.ps1` (xem ben duoi)

### 3. Lenh Serial (1 ky tu)

| Lenh | Y nghia |
|---|---|
| 1 | kich ban dung yen |
| 2 | kich ban bay thang |
| 3 | kich ban hinh chu nhat |
| r | chay lai kich ban hien tai |
| h | in header |
| v | in phien ban |
| ? | tro giup |

### 4. Den xanh (GPIO48)

| Mau nhay | Y nghia |
|---|---|
| 1 lan moi giay | binh thuong |
| 2 lan moi giay | chua co GPS fix (giai doan 2) |
| nhay nhanh lien tuc | BLE bao loi |

## Ghi log ra file

Dong Serial Monitor cua Arduino IDE truoc (cong COM chi mo duoc boi 1 chuong trinh), roi chay:

    powershell -NoProfile -ExecutionPolicy Bypass -File firmware\tools\capture_log.ps1 -Cmd 1 -Count 60 -Out firmware\evidence\T02.csv

`-Cmd` la lenh chon kich ban (1, 2 hoac 3), `-Count` la so dong D can ghi, `-Port` mac dinh COM8.
