# drone-remote-id-ble

Nghiên cứu và triển khai hệ thống giám sát drone tích hợp chuẩn định danh từ xa (Remote ID, ASTM F3411 / OpenDroneID) qua sóng BLE.
Bài tập lớn Kỹ thuật Máy tính, HCMUT, tháng 10/2026.

ESP32-S3 phát gói Remote ID bằng BLE Legacy Advertising (Service Data UUID 0xFFFA) mỗi 1 Hz, ứng dụng Android quét, giải mã và vẽ quỹ đạo trên bản đồ.

## Cấu trúc

| Thư mục | Nội dung | Phụ trách |
|---|---|---|
| `firmware/` | Code ESP32-S3, thư viện đã vá, bằng chứng kiểm thử | Task 1 |
| `data/`, `docs/` | Dữ liệu giả lập, data-contract, báo cáo | Task 2 |
| `mobile/` | Ứng dụng Android | Task 3 |
| `tools/` | Công cụ ghi log, script kiểm thử | Task 4 |

## Quy ước làm việc

- Mỗi người làm trên nhánh riêng, không commit thẳng vào `main`.
- Đặt tên nhánh theo phần việc, ví dụ `fw/A-env`.
- Commit message: `fw(<phần>): <mô tả>`. Dùng tiền tố tương ứng cho phần việc của mình.
- Mở Pull Request vào `main`, có ít nhất 1 người review trước khi merge.