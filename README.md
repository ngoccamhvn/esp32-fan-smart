# 🍃 Dự án Quạt Thông Minh ESP32 (Smart Fan Control)

Dự án phần cứng và mã nguồn mở biến quạt cơ thông thường thành quạt thông minh tích hợp vào hệ thống nhà thông minh **Home Assistant**. Hệ thống sử dụng bo mạch **uPesy ESP32 Wroom DevKit**, được lập trình bằng nền tảng **ESPHome** và tích hợp quy trình **Tự động hóa CI/CD cập nhật OTA qua GitHub Actions**.

---

## 🌟 Tính năng nổi bật

* **Điều khiển đa chế độ**: Quản lý bật/tắt, điều chỉnh 3 cấp độ gió tuần tự và đảo gió (Swing).
* **Mã hóa hồng ngoại vạn năng**: Sử dụng thuật toán băm toàn chuỗi xung thông minh (**MurmurHash3**) kết hợp mắt thu phát hồng ngoại để học và lưu trữ mã remote vĩnh viễn vào bộ nhớ Flash của ESP32.
* **Tự động hóa CI/CD**: Mỗi khi chỉnh sửa và `git push` file cấu hình lên GitHub, hệ thống sẽ tự động biên dịch tạo ra bản phát hành (Release) mới.
* **Cập nhật OTA chỉ với 1 chạm**: Truy cập giao diện IP cục bộ của ESP32 để kiểm tra phiên bản mới nhất trên GitHub và kích hoạt nạp Firmware từ xa qua mạng (Over-The-Air) an toàn bằng nhân ESP-IDF gốc.

---

## 📂 Cấu trúc thư mục kho lưu trữ

```text
├── .github/workflows/
│   └── deploy.yml          # Kịch bản tự động biên dịch và tạo Release trên GitHub
├── esp32-fan-smart.yaml    # File cấu hình cấu trúc phần cứng và logic ESPHome
├── github_ota.h            # Thư viện C++ Custom Component nạp OTA trực tiếp bằng ESP-IDF
├── secrets.yaml            # Nơi lưu trữ thông tin WiFi bảo mật (Không công khai công cộng)
└── README.md               # Hướng dẫn sử dụng dự án này
```

---

## 🛠️ Sơ đồ chân kết nối phần cứng (Pinout)

| Linh kiện / Chức năng | Chân GPIO trên ESP32 | Chế độ cấu hình | Ghi chú |
| :--- | :---: | :---: | :--- |
| **Mắt thu hồng ngoại (IR)** | `GPIO12` | INPUT_PULLUP | Strapping PIN (Sử dụng cẩn thận) |
| **Nút bấm học lệnh vật lý** | `GPIO33` | INPUT_PULLUP | Nhấn giữ để học lệnh / Nhấn nhả để chuyển số |
| **Công tắc gạt chế độ** | `GPIO13` | INPUT_PULLUP | Phân quyền và khóa logic điều khiển |
| **Rơ-le số 1 / 2 / 3 & Đảo gió**| Các chân Relay | OUTPUT | Phụ thuộc vào đấu nối mạch rơ-le thực tế |

---

## 🚀 Hướng dẫn vận hành quy trình cập nhật OTA

### 1. Quản lý phiên bản khi sửa code
Mỗi khi anh thực hiện thay đổi tính năng quạt trong file `esp32-fan-smart.yaml`, hãy tìm đến dòng cấu hình biến môi trường ở đầu file và tăng số phiên bản lên:
```yaml
substitutions:
  fw_version: "1.0.1" # Tăng số này lên bản mới (Ví dụ: 1.0.0 -> 1.0.1)
```

### 2. Kích hoạt quy trình biên dịch tự động
Tiến hành `git commit` và `git push` mã nguồn lên GitHub. Hệ thống **GitHub Actions** sẽ tự chạy trong khoảng 2 phút để biên dịch mã nguồn C++, trích xuất số phiên bản để tạo ra bản phát hành định dạng **Release v1.0.1** kèm file đóng gói `firmware.bin`.

### 3. Cập nhật trên thiết bị bằng 1 chạm
1. Mở trình duyệt, gõ **địa chỉ IP cục bộ của ESP32** Quạt Thông Minh.
2. Đăng nhập với tài khoản quản trị (Mặc định: `admin` / `12345678`).
3. Nhấn nút **Check Firmware Update**: ESP32 sẽ âm thầm gọi GitHub API để kiểm tra. Nếu phát hiện có bản mới, đèn nhật ký hệ thống sẽ báo: *"Phát hiện bản cập nhật mới: X.X.X"*.
4. Nhấn nút **Click to Update Firmware**: Mạch sẽ tự động tải file nhị phân từ internet về, thực hiện flashing an toàn vào bộ nhớ và tự reboot lại để chạy tính năng mới trong 15 giây.

---

## 🔒 Bảo mật dữ liệu cá nhân
Thông tin định danh mạng WiFi nội bộ được mã hóa tách biệt hoàn toàn qua file `secrets.yaml`. Vui lòng không chia sẻ file này để đảm bảo an toàn an ninh mạng cho gia đình anh.

---
✍️ *Dự án được phát triển và duy trì bởi độc quyền bởi **@ngoccamhvn** (Trương Ngọc Cam).*
