# ESP32 Smart Fan Controller

> Bộ điều khiển quạt thông minh sử dụng **ESP32 + ESPHome + Home Assistant** với khả năng **học lệnh hồng ngoại**, **điều khiển từ xa**, **OTA qua GitHub** và **giữ nguyên cách sử dụng quạt gốc**.

---

# Giới thiệu

Đây là firmware dành cho ESP32 giúp biến quạt điện thông thường thành quạt thông minh mà **không cần thay đổi kết cấu của quạt**.

Thiết bị hỗ trợ điều khiển trực tiếp bằng nút bấm, remote hồng ngoại hoặc Home Assistant, đồng thời vẫn đảm bảo người dùng có thể sử dụng quạt như ban đầu.

---

# Tính năng nổi bật

✅ Điều khiển quạt qua Home Assistant

✅ Điều khiển trực tiếp bằng nút bấm vật lý

✅ Học lệnh từ mọi loại remote hồng ngoại

✅ Hỗ trợ 3 cấp tốc độ

✅ Điều khiển đảo gió

✅ Chế độ quạt cơ và quạt điện tử

✅ Đồng bộ trạng thái giữa thiết bị và Home Assistant

✅ Web Server tích hợp

✅ OTA qua trình duyệt Web

✅ OTA tự động từ GitHub Releases

✅ Lưu dữ liệu vào Flash, không mất khi mất điện

---

# Hai chế độ hoạt động

Firmware hỗ trợ hai chế độ điều khiển độc lập.

## 1. Chế độ Quạt Cơ

Ở chế độ này:

- Các nút bấm trên quạt hoạt động như nguyên bản.
- Home Assistant chỉ hiển thị trạng thái.
- Không làm thay đổi cách sử dụng ban đầu.

---

## 2. Chế độ Quạt Điện Tử

ESP32 sẽ điều khiển toàn bộ hoạt động của quạt.

Có thể thực hiện:

- Bật / Tắt
- Tốc độ 1
- Tốc độ 2
- Tốc độ 3
- Đảo gió
- Chuyển tốc độ tuần tự
- Điều khiển từ Home Assistant
- Điều khiển bằng Remote

---

# Học lệnh hồng ngoại

Một trong những tính năng nổi bật nhất của dự án là khả năng **tự học lệnh hồng ngoại**.

Firmware không phụ thuộc vào giao thức NEC, Sony, Samsung...

Thay vào đó sử dụng thuật toán băm (Hash) để ghi nhớ toàn bộ chuỗi xung của remote.

Điều này giúp thiết bị tương thích với gần như mọi loại điều khiển từ xa.

Các phím được học gồm:

- Bật / Tắt
- Chuyển tốc độ
- Tốc độ 1
- Tốc độ 2
- Tốc độ 3
- Đảo gió

Toàn bộ dữ liệu được lưu trong bộ nhớ Flash nên không bị mất khi mất nguồn.

---

# Thuật toán nhận dạng Remote

Firmware sử dụng thuật toán **Murmur Hash** để mã hóa chuỗi xung hồng ngoại.

Ưu điểm:

- Không cần thư viện IR riêng
- Không phụ thuộc giao thức
- Hỗ trợ hầu hết các loại remote
- Tốc độ xử lý nhanh
- Tiết kiệm bộ nhớ RAM
- Hoạt động ổn định ngay cả khi tín hiệu hồng ngoại bị sai lệch nhỏ

---

# Điều khiển Relay an toàn

Ba relay tốc độ được cấu hình **Interlock**.

Điều này đảm bảo:

- Không thể bật đồng thời nhiều cấp tốc độ.
- Tránh chập relay.
- Bảo vệ động cơ quạt.

Relay đảo gió sẽ tự động tắt khi quạt ngừng hoạt động.

---

# Đồng bộ với Home Assistant

Firmware tự động tạo các thực thể (Entity) trong Home Assistant.

Bao gồm:

### Button

- Học lệnh IR
- Bật/Tắt
- Chuyển tốc độ
- Kiểm tra Firmware mới
- Cập nhật Firmware

### Switch

- Tốc độ 1
- Tốc độ 2
- Tốc độ 3
- Đảo gió

### Binary Sensor

- Trạng thái nút bấm
- Trạng thái Relay
- Chế độ hoạt động

### Text Sensor

- Trạng thái học lệnh
- Phiên bản Firmware mới nhất

---

# Cập nhật Firmware OTA qua GitHub

Firmware hỗ trợ cập nhật trực tiếp từ GitHub Releases.

Quy trình hoạt động:

ESP32

↓

Kiểm tra phiên bản mới trên GitHub

↓

So sánh phiên bản hiện tại

↓

Thông báo có Firmware mới

↓

Tải file firmware.bin

↓

Nạp Firmware

↓

Khởi động lại thiết bị

Toàn bộ quá trình không cần sử dụng ESPHome Dashboard.

---

# Web Server tích hợp

Thiết bị có Web Server sẵn.

Có thể truy cập bằng địa chỉ IP của ESP32 để:

- Xem thông tin thiết bị
- Kiểm tra trạng thái
- OTA bằng trình duyệt
- Cập nhật Firmware

Mặc định:

Port:

80

Tên đăng nhập:

admin

Mật khẩu:

12345678

---

# Phần cứng sử dụng

## Vi điều khiển

ESP32

## Sơ đồ chân

| GPIO | Chức năng |
|------|-----------|
| GPIO12 | Thu tín hiệu hồng ngoại |
| GPIO13 | Công tắc chọn chế độ |
| GPIO14 | Nút điều khiển số 1 |
| GPIO25 | Nút đảo gió |
| GPIO26 | Nút tốc độ 3 |
| GPIO27 | Nút điều khiển số 2 |
| GPIO33 | Nút học lệnh |
| GPIO16 | Relay tốc độ 1 |
| GPIO17 | Relay tốc độ 2 |
| GPIO5 | Relay tốc độ 3 |
| GPIO18 | Relay đảo gió |

---

# Công nghệ sử dụng

- ESPHome
- Home Assistant API
- GitHub OTA
- HTTP Request
- Web Server
- Murmur Hash
- Flash Storage
- Relay Interlock
- OTA qua Web
- OTA qua GitHub

---

# Cấu trúc dự án

```
ESP32_FAN_SMART

├── firmware.yaml
├── github_ota.h
├── README.md
├── build/
└── releases/
    └── firmware.bin
```

---

# Hướng phát triển

Trong các phiên bản tiếp theo sẽ bổ sung:

- Hẹn giờ tắt quạt
- Điều khiển bằng MQTT
- Tự động theo nhiệt độ phòng
- Điều khiển bằng giọng nói
- Giao diện Web cấu hình WiFi
- Điều chỉnh thời gian đảo gió
- Điều khiển nhiều quạt cùng lúc
- Theo dõi điện năng tiêu thụ

---

# Giấy phép

Dự án được phát hành theo giấy phép **MIT License**.

---

# Tác giả

**Ngọc Cam**

Nếu dự án hữu ích, hãy nhấn ⭐ Star để ủng hộ và theo dõi các bản cập nhật mới.

Mọi đóng góp, báo lỗi hoặc đề xuất tính năng đều được chào đón thông qua mục **Issues** hoặc **Pull Requests** trên GitHub.
