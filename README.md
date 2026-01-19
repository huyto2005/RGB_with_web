<p align="center">
  <a href="https://www.uit.edu.vn/" title="Trường Đại học Công nghệ Thông tin" style="border: none;">
    <img src="https://i.imgur.com/WmMnSRt.png" alt="Trường Đại học Công nghệ Thông tin | University of Information Technology">
  </a>
</p>

<h1 align="center"><b>Thiết kế hệ thống nhúng - CE224 </b></h1>

# Thành viên nhóm
| STT | MSSV | Họ và Tên | Chức Vụ | Email |
| :--- | :---: | :--- | :---: | :--- |
| 1 | 23520604 | Đỗ Quốc Huy | Nhóm trưởng | 23520604@gm.uit.edu.vn |
| 2 | 23520915 | Lê Quốc Mạnh | Thành viên | 23520915@gm.uit.edu.vn |
| 3 | 23520539 | Phùng Việt Hoàng | Thành viên | 23520539@gm.uit.edu.vn |
| 4 | 23520936 | Nguyễn Hoàng Minh | Thành viên | 23520936@gm.uit.edu.vn |                   |

# GIỚI THIỆU MÔN HỌC
* **Tên môn học:** Thiết kế hệ thống nhúng
* **Mã môn học:** CE224
* **Mã lớp:** CE224.Q11
* **Năm học:** HK1 (2025 - 2026)
* **Giảng viên**: ThS. Phạm Minh Quân

# ĐỒ ÁN CUỐI KÌ
* **Đề tài:** Thiết kế hệ thống điều khiển đèn LED RGB sử dụng giao tiếp I2C qua trình duyệt Web
* **Mô tả:** Hệ thống IoT điều khiển dải đèn LED WS2812B thông qua Wifi và giao thức MQTT. Tích hợp tính năng nháy theo nhạc (Music Visualizer), điều khiển bằng giọng nói tiếng Việt và hiển thị trạng thái trên màn hình OLED.

# Hướng dẫn sử dụng

**1. Yêu cầu phần cứng**

- **Vi điều khiển:** ESP32 NodeMCU (DevKit V1)
- **Thiết bị hiển thị:** Màn hình OLED SSD1306 (0.96 inch, giao tiếp I2C)
- **Thiết bị chấp hành:** LED RGB WS2812B (Dạng vòng hoặc dây)
- **Kết nối:**
    - LED RGB: Chân DIN nối vào **GPIO 5**
    - OLED: SDA nối **GPIO 21**, SCL nối **GPIO 22**

**2. Cài đặt môi trường phát triển**

- Cài đặt [Visual Studio Code](https://code.visualstudio.com/)
- Cài đặt Extension **PlatformIO IDE** trong VS Code.
- Các thư viện cần thiết (đã có trong `platformio.ini`):
    - `Adafruit NeoPixel`
    - `Adafruit SSD1306` & `Adafruit GFX`
    - `PubSubClient` (MQTT)
    - `ArduinoJson`

**3. Cấu hình & Nạp Code (Firmware)**

1. Clone repository này về máy hoặc tải file zip.
2. Mở thư mục dự án bằng VS Code.
3. Mở file `src/main.cpp`, chỉnh sửa thông tin Wifi và MQTT Broker:
   ```cpp
   const char* ssid = "TEN_WIFI_CUA_BAN";
   const char* password = "MAT_KHAU_WIFI";
   const char* mqtt_server = "URL_HIVEMQ_CLUSTER"; // Ví dụ: ...s1.eu.hivemq.cloud
   const char* mqtt_user = "user_mqtt";
   const char* mqtt_pass = "pass_mqtt";

4. Kết nối ESP32 với máy tính qua cáp USB - TypeC.

5. Bấm nút mũi tên (➡️) Upload trên thanh công cụ PlatformIO để nạp code.

**4. Triển khai Web App (Giao diện điều khiển)**
Truy cập thư mục Web_Interface (hoặc lấy file lib/UI.html).

Mở file index.html (hoặc UI.html), cập nhật địa chỉ MQTT Broker tại dòng:

    const host = "URL_HIVEMQ_CLUSTER"; // Phải khớp với trong main.cpp
Chạy file HTML trực tiếp trên trình duyệt hoặc Deploy lên Netlify để điều khiển qua Internet.

https://rgbcontrolonweb.netlify.app/

  Username: esp32_user
  
  Password: Esp123456

**5.Chức năng chính**

Dashboard: Điều khiển Bật/Tắt, chỉnh màu sắc, độ sáng (0-100%).

Effects: Các hiệu ứng: Rainbow (Cầu vồng), Chase (Đuổi bắt), Breath (Thở).

Music Mode 🎵: Đèn nháy theo nhịp điệu bài hát (phân tích phổ âm thanh FFT).

Voice Control 🎙️: Ra lệnh bằng giọng nói Tiếng Việt (Ví dụ: "Bật đèn", "Màu đỏ").

Monitor: OLED hiển thị logo "Babyboy", trạng thái Wifi, IP và Mode hiện tại.

# LICENSE

MIT

# Thông tin thêm

Dự án sử dụng cơ chế Session Locking để quản lý phiên làm việc, đảm bảo chỉ một người điều khiển tại một thời điểm. Nếu bạn gặp vấn đề hoặc cần thêm thông tin, vui lòng liên hệ qua email hỗ trợ của nhóm phát triển.