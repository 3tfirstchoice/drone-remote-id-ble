# Data contract: dữ liệu giả lập Remote ID

Tài liệu giao ước dữ liệu của Task 2 (Data & Protocol Lead), theo mục 3.2 của đặc tả `AeroDrone.pdf`. Tài liệu quy định tên biến, kiểu dữ liệu, hệ quy chiếu, đơn vị đo và dải giá trị hợp lệ của dữ liệu giả lập dùng chung giữa các task:

- Task 1 nạp `waypoints.h` vào firmware.
- Task 3 và Task 4 dùng các file CSV làm số liệu chuẩn để đối chiếu với log firmware và với giá trị hiển thị trên ứng dụng.

Tiêu chuẩn tham chiếu: ASTM F3411 / OpenDroneID, mã nguồn `opendroneid.h` và `opendroneid.c` trong `firmware/libraries/id_open/`.

## 1. Nguồn dữ liệu và file bàn giao

Mọi file đều sinh từ một script duy nhất, không sửa tay:

| File | Dùng cho | Nội dung |
|---|---|---|
| `data/generate_mock_data.py` | Task 2 | Script sinh dữ liệu, chỉ dùng thư viện chuẩn Python (`csv`, `math`, `struct`...) |
| `data/stationary.csv` | Task 3, Task 4 | Kịch bản 1: đứng yên, 60 dòng |
| `data/straight_line.csv` | Task 3, Task 4 | Kịch bản 2: bay thẳng hướng Bắc, 30 dòng |
| `data/rectangle.csv` | Task 3, Task 4 | Kịch bản 3: bay hình chữ nhật, 41 dòng |
| `data/waypoints.h` | Task 1 | Mảng waypoint dạng C (bản bàn giao) |
| `firmware/rid_tx/waypoints.h` | Firmware | Bản giống hệt `data/waypoints.h`, nơi firmware đọc |

Sinh lại dữ liệu: chạy `python data/generate_mock_data.py` ở thư mục gốc repo, sau đó nạp lại firmware.

## 2. Hệ quy chiếu

| Đại lượng | Hệ quy chiếu |
|---|---|
| Vĩ độ, kinh độ | WGS84 ellipsoid (hệ tọa độ của GPS), đơn vị độ thập phân. Bắc và Đông mang dấu dương. |
| Tâm mô phỏng (home) | Khuôn viên ĐH Bách Khoa TP.HCM: vĩ độ 10.7725°, kinh độ 106.6578°. Đây cũng là vị trí người điều khiển trong System message. |
| `alt_msl` | Độ cao so với mực nước biển (MSL). Home ở 10.0 m, drone bay ở 60.0 m. |
| `alt_agl` | Độ cao so với điểm cất cánh (home), bằng `alt_msl − 10.0 m`. Firmware gửi giá trị này vào trường `Height` của Location message, với loại độ cao `ODID_HEIGHT_REF_OVER_TAKEOFF`. |
| Hướng bay `heading` | Góc theo chiều kim đồng hồ tính từ hướng Bắc thật: 0° là Bắc, 90° là Đông, 180° là Nam, 270° là Tây. |
| Thời gian | UTC. Mốc bắt đầu là thời điểm chạy script, ghi trong `SCENARIO_EPOCH`. |

Lưu ý về độ cao: trường `AltitudeGeo` của Remote ID được chuẩn định nghĩa là độ cao WGS84-HAE (so với ellipsoid). Firmware đưa thẳng `alt_msl` vào trường này mà không đổi từ MSL sang HAE. Với dữ liệu giả lập, `alt_msl = 60.0 m` được dùng nguyên như vậy.

## 3. Đơn vị đo

| Đại lượng | Đơn vị trong dữ liệu giả lập | Đơn vị trong gói Remote ID | Quy đổi |
|---|---|---|---|
| Vị trí | độ (CSV), độ × 10⁷ (`waypoints.h`) | độ × 10⁷ (số nguyên int32) | chia hoặc nhân 10⁷ |
| Độ cao | mét (CSV), 0.1 m (`waypoints.h`) | 0.5 m, gốc −1000 m | `(m + 1000) / 0.5` |
| Tốc độ ngang | knots (hải lý/giờ) | m/s, bước 0.25 m/s | 1 kn = 0.514444 m/s |
| Hướng | độ | độ | không đổi |
| Thời gian | giây UTC | 0.1 s tính từ đầu giờ | — |

- Mọi độ cao và khoảng cách dùng **mét**, không dùng feet. Nguồn nào cho số liệu bằng feet phải đổi sang mét (1 ft = 0.3048 m) trước khi đưa vào dữ liệu.
- Tốc độ lưu bằng **knots** vì struct `UTM_data` của firmware dùng `speed_kn`. Firmware đổi sang m/s khi đóng gói Remote ID, với hệ số 0.514444 (`id_open.cpp`).

## 4. Tên biến và kiểu dữ liệu

### 4.1 `waypoints.h`

```c
#define SCENARIO_EPOCH   <unix>UL      // giờ bắt đầu giả lập (UTC)

typedef struct {
  int32_t  lat_e7, lon_e7;
  int16_t  alt_msl_dm, alt_agl_dm;
  uint8_t  speed_kn;
  uint16_t heading_deg;
} waypoint_t;

static const waypoint_t WP_STATIONARY[] = { ... };   // 60 phần tử
static const waypoint_t WP_STRAIGHT[]   = { ... };   // 30 phần tử
static const waypoint_t WP_RECTANGLE[]  = { ... };   // 40 phần tử
#define <TÊN>_LEN (sizeof(<TÊN>) / sizeof(<TÊN>[0]))
```

| Biến | Kiểu C | Đơn vị | Dải hợp lệ | Ý nghĩa |
|---|---|---|---|---|
| `SCENARIO_EPOCH` | `unsigned long` (macro) | giây Unix, UTC | thời điểm chạy script | Firmware đặt đồng hồ hệ thống bằng giá trị này lúc khởi động (`settimeofday` trong `rid_tx.ino`) |
| `lat_e7` | `int32_t` | 10⁻⁷ độ | −900 000 000 … 900 000 000 | Vĩ độ × 10⁷ |
| `lon_e7` | `int32_t` | 10⁻⁷ độ | −1 800 000 000 … 1 800 000 000 | Kinh độ × 10⁷ |
| `alt_msl_dm` | `int16_t` | 0.1 m | −10 000 … 32 767 (−1000.0 … 3276.7 m), chia hết cho 5 | Độ cao MSL |
| `alt_agl_dm` | `int16_t` | 0.1 m | như trên | Độ cao so với điểm cất cánh |
| `speed_kn` | `uint8_t` | knots, số nguyên | 0 … 255 | Tốc độ ngang |
| `heading_deg` | `uint16_t` | độ | 0 … 359 | Hướng bay |

Giải thích các giới hạn:
- Cận dưới −1000 m của độ cao là giá trị nhỏ nhất OpenDroneID biểu diễn được (`MIN_ALT`). Cận trên 3276.7 m là giới hạn của `int16_t` khi lưu theo đơn vị 0.1 m.
- Độ cao phải chia hết cho 5 (tức bội số của 0.5 m) để khớp đúng bước mã hóa 0.5 m.
- Hướng bay giới hạn ở 0 … 359 vì OpenDroneID dùng 360 trở lên làm giá trị "không hợp lệ" (`INV_DIR = 361`).

### 4.2 File CSV

Định dạng: ASCII, dấu phẩy phân cách, xuống dòng `\n`, dòng đầu là header. Tên cột trùng tên cột trong log Serial của firmware (`firmware/README.md`), để Task 4 so sánh trực tiếp theo cột.

| Cột | Kiểu | Định dạng / đơn vị | Dải hợp lệ | Ý nghĩa |
|---|---|---|---|---|
| `utc` | chuỗi | ISO 8601 `YYYY-MM-DDThh:mm:ssZ` | từ `SCENARIO_EPOCH`, tăng 1 s mỗi dòng | Thời điểm của mẫu |
| `scn` | số nguyên | — | 0, 1, 2 | Mã kịch bản (0 đứng yên, 1 bay thẳng, 2 hình chữ nhật), trùng cột `scn` của firmware |
| `idx` | số nguyên | — | 0 … (số waypoint − 1) | Chỉ số waypoint trong mảng `waypoints.h` |
| `lat` | số thực | độ, 7 chữ số thập phân | −90 … 90 | `lat_e7 / 10⁷` |
| `lon` | số thực | độ, 7 chữ số thập phân | −180 … 180 | `lon_e7 / 10⁷` |
| `alt_msl` | số thực | mét, 1 chữ số thập phân | −1000.0 … 3276.7 | `alt_msl_dm / 10` |
| `alt_agl` | số thực | mét, 1 chữ số thập phân | −1000.0 … 3276.7 | `alt_agl_dm / 10` |
| `speed_kn` | số nguyên | knots | 0 … 255 | Tốc độ ngang |
| `heading` | số nguyên | độ | 0 … 359 | Hướng bay |
| `sats` | số nguyên | — | ≥ 7 | Số vệ tinh giả lập, bằng `MOCK_SATS` trong `config.h` |

CSV chỉ chứa các cột do kịch bản quyết định. Các cột chỉ đo được khi chip chạy (`ms`, `tx`, `err`, `heap` trong log firmware) không có trong CSV.

### 4.3 Đường đi của dữ liệu trong firmware

| `waypoints.h` | `UTM_data` (`mock_source.cpp`) | Trường Location message (`id_open.cpp`) |
|---|---|---|
| `lat_e7`, `lon_e7` | `latitude_d`, `longitude_d` = giá trị / 10⁷ (`double`) | `Latitude`, `Longitude` |
| `alt_msl_dm` | `alt_msl_m` = giá trị / 10 (`float`) | `AltitudeGeo` |
| `alt_agl_dm` | `alt_agl_m` = giá trị / 10 (`float`) | `Height` (loại `OVER_TAKEOFF`) |
| `speed_kn` | `speed_kn` (`int`) | `SpeedHorizontal` = 0.514444 × `speed_kn` (m/s) |
| `heading_deg` | `heading` (`int`) | `Direction` |
| `MOCK_SATS` (`config.h`) | `satellites` | Nếu ≥ 7 (`SATS_LEVEL_2`) thì gửi tọa độ với Status `UNDECLARED`; nếu nhỏ hơn thì Status `REMOTE_ID_SYSTEM_FAILURE` |

## 5. Ba kịch bản bay

Tham số chung: tần số 1 Hz (mỗi waypoint, mỗi dòng CSV là 1 giây), `alt_msl = 60.0 m`, `alt_agl = 50.0 m`, `sats = 10`. Mỗi file CSV bắt đầu tại `SCENARIO_EPOCH`.

| Kịch bản | Mảng (số phần tử) | File CSV (số dòng dữ liệu) | Mô tả |
|---|---|---|---|
| 1. Tĩnh | `WP_STATIONARY` (60) | `stationary.csv` (60) | Đứng yên tại home, tốc độ 0, hướng 0° |
| 2. Đường thẳng | `WP_STRAIGHT` (30) | `straight_line.csv` (30) | Bay hướng Bắc (0°), 10 kn ≈ 5.144 m/s. Vĩ độ tăng tuyến tính 4.62 × 10⁻⁵ độ mỗi giây (5.144 m / 111 320 m/độ), kinh độ không đổi |
| 3. Hình chữ nhật | `WP_RECTANGLE` (40) | `rectangle.csv` (41) | 4 cạnh, mỗi cạnh 10 giây, 10 kn: Đông (90°) → Bắc (0°) → Tây (270°) → Nam (180°). Mỗi cạnh dài ≈ 51.4 m |

Kịch bản 3 khép kín như sau:
- `waypoints.h` lưu 40 điểm, không lưu điểm cuối trùng điểm đầu. Sau điểm `idx = 39`, firmware quay về `idx = 0`, cũng là điểm xuất phát, ở giây thứ 40. Vòng lặp dài 40 giây.
- `rectangle.csv` có thêm dòng thứ 41 (`idx = 0`, giây thứ 40) để thể hiện điểm kết thúc trùng điểm xuất phát, theo tiêu chí T04. Dòng này giống đúng dòng mà firmware ghi ra ở giây thứ 40.

Hành vi firmware ngoài phạm vi CSV:
- Kịch bản 1 và 3 lặp lại liên tục.
- Kịch bản 2 không lặp. Sau điểm `idx = 29`, firmware giữ nguyên vị trí cuối và báo tốc độ 0 (`mock_source.cpp`).

Hệ số 111 320 m/độ là giá trị gần đúng cho một độ vĩ. Ở vĩ độ 10.77°, một độ vĩ thực tế dài khoảng 110 600 m, nên quãng đường thật mỗi giây ngắn hơn 5.144 m khoảng 0.6%.

## 6. Kiểm tra dữ liệu trong script

Script dừng và báo lỗi, không ghi file nào, nếu một trong các điều kiện sau sai:

1. `HOME_LAT`, `HOME_LON`, `HOME_ALT_MSL`, `MOCK_SATS` trong script trùng với `firmware/rid_tx/config.h`.
2. Số điểm đúng là 60 / 30 / 40.
3. Mỗi waypoint nằm trong dải hợp lệ ở mục 4.1 (tọa độ WGS84, độ cao bội 0.5 m và vừa `int16_t`, tốc độ vừa `uint8_t`, hướng 0 … 359).
4. `MOCK_SATS ≥ 7`.
5. Kịch bản hình chữ nhật khép kín: bước từ điểm 39 về điểm 0 bằng bước giữa hai điểm liên tiếp trên cạnh Nam.
6. Sai số lượng tử hóa nằm trong ngưỡng ở mục 7.

## 7. Sai số lượng tử hóa

Script mô phỏng đúng chuỗi xử lý của firmware: đọc waypoint → đổi sang `double`/`float` như `mock_source.cpp` → mã hóa như `opendroneid.c` → giải mã lại. Sau đó so với giá trị gốc do script tính trước khi làm tròn. Kết quả mô phỏng đã được đối chiếu với thư viện `opendroneid.c` thật (biên dịch bằng gcc) và trùng khớp cho cả 130 waypoint.

| Đại lượng | Độ phân giải gốc của dữ liệu | Độ phân giải mã hóa OpenDroneID | Sai số lớn nhất đo được | Ngưỡng đạt | Kết quả |
|---|---|---|---|---|---|
| Vĩ độ, kinh độ | 10⁻⁷ độ (`int32` trong `waypoints.h`) | 10⁻⁷ độ (≈ 1.1 cm) | 1.49 × 10⁻⁷ độ (≈ 1.7 cm) | < 10⁻⁶ độ (tiêu chí T01) | Đạt |
| Độ cao MSL, AGL | 0.1 m | 0.5 m | 0 m | < 0.5 m (1 bước mã hóa) | Đạt |
| Tốc độ ngang | 1 kn (≈ 0.514 m/s) | 0.25 m/s | 0.144 m/s | < 0.25 m/s (1 bước mã hóa) | Đạt |

Sai số lớn nhất theo từng kịch bản:

| Kịch bản | Vĩ/kinh độ: sai số làm tròn khi lưu waypoint | Vĩ/kinh độ: sai số sau mã hóa | Số giá trị bị mất 10⁻⁷ độ khi mã hóa | Độ cao | Tốc độ |
|---|---|---|---|---|---|
| Tĩnh | 0 | 0 | 0 / 120 | 0 | 0 |
| Đường thẳng | 4.85 × 10⁻⁸ độ | 1.49 × 10⁻⁷ độ | 2 / 60 | 0 | 0.144 m/s |
| Hình chữ nhật | 4.77 × 10⁻⁸ độ | 1.37 × 10⁻⁷ độ | 6 / 80 | 0 | 0.144 m/s |

Giải thích kết quả:

1. **Vị trí.** Sai số gồm hai phần:
   - Script làm tròn tọa độ về 10⁻⁷ độ khi lưu waypoint, sai tối đa 0.5 × 10⁻⁷ độ.
   - `encodeLatLon` của thư viện cắt bỏ phần lẻ (`(int64)(lat × 10⁷)`) thay vì làm tròn. Với một số giá trị, phép chia rồi nhân 10⁷ trên số thực cho kết quả nhỏ hơn số nguyên một chút, ví dụ `107733780` thành `107733779`, nên mất 1 đơn vị 10⁻⁷ độ.

   Tổng sai số vẫn nhỏ hơn ngưỡng T01 khoảng 7 lần.
2. **Độ cao.** 60.0 m và 50.0 m là bội số của 0.5 m nên mã hóa không mất gì.
3. **Tốc độ.** 10 kn = 5.144 m/s, nhưng thư viện cắt về bội số 0.25 m/s nên gửi đi 5.00 m/s. Ứng dụng thu sẽ hiển thị 5.0 m/s (khoảng 9.7 kn), không phải 5.14 m/s. Khi đối chiếu T03 cần dùng giá trị 5.00 m/s này.
