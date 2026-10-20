# Firmware - YOLO UNO (ESP32-S3) phat Remote ID qua BLE

Firmware `rid_tx` phat goi Remote ID (ASTM F3411 / OpenDroneID) bang BLE Legacy Advertising, Service Data UUID 0xFFFA.
Giai doan 1 phat du lieu gia lap tu `rid_tx/waypoints.h`. Giai doan 2 (USE_GPS 1) phat du lieu tu GPS that (M5Stack GPS/BDS Unit v1.1, AT6668).

Huong dan nap day du, loi thuong gap: se bo sung o ban phat hanh v1.0.

## Giao dien

### 1. waypoints.h

Moi phan tu la 1 diem, 1 giay mot diem. File duoc sinh boi `data/generate_mock_data.py` (Task 2), khong sua tay. Sinh lai: `python data/generate_mock_data.py` o thu muc goc repo, roi nap lai firmware.
File con chua `#define SCENARIO_EPOCH` = gio bat dau gia lap (thoi diem chay script, trung moc thoi gian trong CSV cua Task 2).

    typedef struct {
      int32_t  lat_e7, lon_e7;        // do * 1e7
      int16_t  alt_msl_dm, alt_agl_dm; // 0.1 m
      uint8_t  speed_kn;               // hai ly/gio, so nguyen
      uint16_t heading_deg;            // 0 = Bac, 90 = Dong
    } waypoint_t;

Ba mang: WP_STATIONARY (60 diem), WP_STRAIGHT (30 diem, huong Bac, 10 kn), WP_RECTANGLE (40 diem, 4 canh x 10 giay, dong-bac-tay-nam; sau diem 39 firmware quay ve diem 0 = diem xuat phat, vong lap 40 giay). Moi mang co macro `<TEN>_LEN`.

### 2. Log Serial

- 115200 baud, USB CDC, moi dong ket thuc bang `\n`
- Dong bat dau `#` la ghi chu; `H,` la header; `D,` la du lieu (1 dong/giay)
- Dong `# first_tx ms=N` in 1 lan sau khi khoi dong: thoi diem (ms ke tu luc chip khoi dong) goi BLE dau tien duoc phat
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

## Giai doan 2: GPS that

1. Cai thu vien **TinyGPSPlus** (Arduino IDE > Library Manager).
2. Noi day (xem `BOARD_PINS.md`): GND-GND, 5V-5V, TX cua GPS -> D3 (GPIO6), RX cua GPS -> D4 (GPIO7). Neu khong co du lieu thi dao 2 day TX/RX.
3. Nap `tools/gps_passthrough` truoc: sketch tu thu cac to hop baud/dinh dang, in dong `# => CHON: ...` va tom tat moi 2 giay (`fix`, `ve_tinh` dung de fix, `nhin_thay` = ve tinh anten dang thay, `hdop`). Phai de gan cua so/ngoai troi; trong nha thuong khong fix. Lenh: `v` bat/tat in cau tho, `r` do lai.
4. Neu baud khac 115200 hoac module chay 7N1, sua `GPS_BAUD` / `GPS_UART_7N1` trong `rid_tx/config.h`.
5. Dat `USE_GPS 1` trong `rid_tx/config.h`, nap `rid_tx`. Den xanh nhay 2 lan/giay cho den khi du >= 7 ve tinh.

Hanh vi chinh (`rid_tx/gps_source.cpp`):
- Moi giay dien `UTM_data` tu cau GGA/RMC moi nhat; mat fix (qua `GPS_FIX_MAX_AGE_MS` = 2 s) hoac < 7 ve tinh thi goi Location bao loi, khong phat toa do cu nhu toa do hop le.
- Home (vi tri nguoi dieu khien) = diem fix dat chuan dau tien; do cao AGL = do cao MSL hien tai tru do cao home.
- Huong (heading) giu gia tri cu khi toc do < 1 kn, vi huong GPS khong tin cay luc dung yen.
- Dong ho he thong duoc dong bo theo gio GPS (chi khi co fix, moi 60 s); truoc do chay theo `SCENARIO_EPOCH`.
- Cac dong log co `scn=-1`, `idx=0` o che do GPS.

## Kiem thu tren may tinh

    bash firmware/test/run_tests.sh

Chay logic GPS (voi TinyGPSPlus that), dinh dang log, den bao, waypoints va byte Operator ID tren may tinh bang stub Arduino. Khong thay the viec nap board that: chua kiem chung bien dich Arduino IDE, BLE va fix ngoai troi.

## Cau hinh va luu y

- `config.h` co static_assert kiem tra ID (toi da 20 ky tu), toa do home, `MOCK_SATS` >= 7: sai thi bao loi luc bien dich.
- Luc khoi dong firmware in `# identity ...` (ID dang phat); sau 5 s ma chua co goi BLE nao se in `# CANH BAO`.
- Operator ID `LAB-OPERATOR-01` duoc ma hoa dung (giu nguyen dau gach, da test). App Drone Scanner hien "LABOPERATOR01" kem canh bao vi no kiem tra dinh dang dang ky EU, khong phai loi firmware. Muon het canh bao thi doi `OPERATOR_ID` sang dinh dang ma app chap nhan (chua kiem chung).
