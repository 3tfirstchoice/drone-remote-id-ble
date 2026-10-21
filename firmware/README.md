# Firmware - YOLO UNO (ESP32-S3) phat Remote ID qua BLE

Firmware `rid_tx` phat goi Remote ID (ASTM F3411 / OpenDroneID) bang BLE Legacy Advertising, Service Data UUID 0xFFFA.
Giai doan 1 phat du lieu gia lap tu `rid_tx/waypoints.h`. Giai doan 2 (USE_GPS 1) phat du lieu tu GPS that (M5Stack GPS/BDS Unit v1.1, AT6668).

Moi truong build: `firmware-environment.md`. Chan cua board: `BOARD_PINS.md`. Cac sua doi thu vien: `PATCHES.md`.
Huong dan nap day du, loi thuong gap: se bo sung o ban phat hanh v1.0.

## Cau truc code

    firmware/
      rid_tx/                 firmware chinh (mo rid_tx.ino trong Arduino IDE)
        rid_tx.ino            noi cac module: setup(), loop(), lenh Serial, chon nguon du lieu
        config.h              MOI hang so (ID, home, chan, GPS, nhip) + kiem tra luc bien dich
        mock_source.h/.cpp    nguon gia lap: doc 3 kich ban trong waypoints.h
        gps_source.h/.cpp     nguon GPS that: doc NMEA qua UART1 bang TinyGPSPlus
        waypoints.h           du lieu kich ban, SINH TU DONG boi data/generate_mock_data.py
        telemetry.h/.cpp      log CSV 1 dong/giay ra Serial
        status_led.h/.cpp     den xanh GPIO48, bao trang thai bang so lan nhay
        ble_stats.h           khai bao bo dem goi BLE (dinh nghia trong thu vien, PATCH #2)
      libraries/              thu vien id_open + utm (da va, xem PATCHES.md)
      test/                   test tren may tinh, khong can board (bash firmware/test/run_tests.sh)
      tools/                  sketch chan doan va script ghi log (xem tools/README.md)
      evidence/               log va ghi chu ket qua kiem thu T02-T06

Luong du lieu (moi module chi lam 1 viec, chi `rid_tx.ino` biet ca he thong):

    waypoints.h --> mock_source --+                      +--> id_open transmit()  --> BLE, 1 goi / 75 ms
                    (USE_GPS 0)   |   1 lan / giay        |    (Location, System, Basic ID, Self ID, Operator ID)
                                  +--> UTM_data utm_data -+
    module GPS --> gps_source ----+   (phieu du lieu      +--> telemetry_line()   --> Serial, 1 dong CSV / giay
      (UART1)      (USE_GPS 1)        dung chung)

- `UTM_data` (khai bao trong `libraries/utm/utm.h`) la cau truc ma thu vien id_open doc de dong goi. Nguon du lieu dien vi tri, toc do, huong, so ve tinh; `rid_tx.ino` dien ngay gio (`fill_time`) va vi tri nguoi dieu khien (`base_*`).
- Lua chon gia lap / GPS chi nam o 1 cho: muc "nguon du lieu" trong `rid_tx.ino` (4 ham `source_begin`, `source_poll`, `source_step`, `source_command`). `setup()` va `loop()` khong co `#if`.
- Thu vien id_open khong doc `hdop_s` / `vdop_s` (do chinh xac ngang co dinh 10 m), nen firmware khong dien 2 truong nay.

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

- 115200 baud, USB CDC, moi dong ket thuc bang `\n` (rieng dong header `H,` ket thuc bang `\r\n`)
- Dong bat dau `#` la ghi chu; `H,` la header; `D,` la du lieu (1 dong/giay)
- Luc khoi dong: `# rid_tx boot ...`, `# identity uav=... operator=... self_id=...` (ID dang phat), roi header
- Dong `# first_tx ms=N` in 1 lan: thoi diem (ms ke tu luc chip khoi dong) goi BLE dau tien duoc phat. Neu sau 5 s van chua co goi nao thi in 1 dong `# CANH BAO ...`
- Header: `H,ms,utc,scn,idx,lat,lon,alt_msl,alt_agl,speed_kn,heading,sats,tx,err,heap`
- `scn`: 0 dung yen, 1 bay thang, 2 chu nhat, -1 GPS
- Cong COM co the bien mat vai giay moi lan board reset
- Doc log bang `tools/capture_log.ps1` (xem ben duoi)

### 3. Lenh Serial (1 ky tu)

| Lenh | Y nghia | Che do |
|---|---|---|
| 1 | kich ban dung yen | chi gia lap |
| 2 | kich ban bay thang | chi gia lap |
| 3 | kich ban hinh chu nhat | chi gia lap |
| r | chay lai kich ban hien tai | chi gia lap |
| h | in header | ca hai |
| v | in phien ban | ca hai |
| ? | tro giup (liet ke lenh cua che do dang chay) | ca hai |

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

Can g++, gcc, git (lan dau tai TinyGPSPlus 1.1.0 ve `test/.deps/`). Script build va chay:

| Chuong trinh | Kiem tra |
|---|---|
| `modules` | tung module: gps_source (voi TinyGPSPlus that), telemetry, status_led, mock_source + waypoints.h, byte Operator ID (voi opendroneid.c that) |
| `rid_tx_mock` | `rid_tx.ino` voi USE_GPS 0: setup, 1 dong log/giay, gio theo SCENARIO_EPOCH, lenh Serial, canh bao BLE, den |
| `rid_tx_gps` | `rid_tx.ino` voi USE_GPS 1: chua fix -> co fix (chot home, dong bo gio) -> mat fix |

Them 2 kiem tra tinh: 2 ban `waypoints.h` (data/ va firmware/rid_tx/) giong het nhau; `tools/gps_passthrough` dung cu phap.

Cau truc `test/`: `stubs/` (Arduino.h, id_open.h gia lap va dinh nghia bien gia lap trong stubs.cpp), `check.h` (macro CHECK), `nmea_fixture.h` (tao cau NMEA), moi file `test_*.cpp` ung voi 1 module.
Test khong thay the viec nap board that: chua kiem chung bien dich bang Arduino IDE, BLE that va fix ngoai troi.

## Cau hinh va luu y

- `config.h` co static_assert kiem tra ID (toi da 20 ky tu), toa do home, `MOCK_SATS` >= 7: sai thi bao loi luc bien dich.
- Operator ID `LAB-OPERATOR-01` duoc ma hoa dung (giu nguyen dau gach, da test bang opendroneid.c). Drone Scanner van bao "invalid" o truong Operator ID cho moi ID gia: da thu `VNM87labtest0001-abc` (dinh dang so dang ky EU) va vi du mau `NLD87astrdge12kc-xyz`, deu bi bao. Day la ket qua cua viec app kiem tra dinh danh nguoi dieu khien, khong phai loi firmware va khong anh huong vi tri.
