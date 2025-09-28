# Smart_Embed - ESP32-C3 IoT Distance Monitoring System

[![ESP32-C3](https://img.shields.io/badge/ESP32--C3-RISC--V-blue)](https://www.espressif.com/en/products/socs/esp32-c3)
[![FreeRTOS](https://img.shields.io/badge/FreeRTOS-Real--time-green)](https://www.freertos.org/)
[![WiFi](https://img.shields.io/badge/WiFi-Station--Mode-orange)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-guides/wifi.html)
[![License](https://img.shields.io/badge/License-CC0--1.0-lightgrey)](https://creativecommons.org/publicdomain/zero/1.0/)

## 📋 Tổng quan

**Smart_Embed** là hệ thống giám sát khoảng cách thông minh sử dụng ESP32-C3, tích hợp cảm biến siêu âm HC-SR04, màn hình OLED, và giao diện web để theo dõi và điều khiển thiết bị từ xa. Hệ thống được thiết kế với kiến trúc FreeRTOS đa tác vụ, đảm bảo hiệu suất cao và phản hồi real-time.

### ✨ Tính năng chính

- 🔍 **Giám sát khoảng cách real-time** với cảm biến siêu âm HC-SR04
- 📱 **Giao diện web responsive** với biểu đồ trực quan
- 💡 **Cảnh báo LED tự động** khi phát hiện vật thể gần
- 💾 **Lưu trữ dữ liệu** trên thẻ SD với định dạng CSV
- 🌐 **Kết nối WiFi** với chế độ Station
- 📊 **Theo dõi hiệu suất** và tối ưu hóa hệ thống
- 🔄 **Đồng bộ hóa real-time** giữa thiết bị và web interface

## 🏗️ Kiến trúc hệ thống

### Cấu trúc dự án

```
Smart_Embed/
├── main/                          # Source code chính
│   ├── Smart_Embed.c             # File chính với FreeRTOS tasks
│   ├── index.html                # Web interface (embedded)
│   ├── CMakeLists.txt            # Build configuration
│   └── Kconfig.projbuild         # Configuration options
├── components/                    # Custom components
│   ├── ultrasonic_sensor/        # Driver cảm biến siêu âm
│   ├── oled_driver/              # Driver màn hình OLED
│   ├── http_server_app/          # HTTP server và API
│   ├── sd_card_spi/              # Driver thẻ SD
│   └── esp32c3_wifi/             # WiFi configuration
├── build/                        # Build output
├── sdkconfig                     # ESP-IDF configuration
└── README.md                     # Documentation này
```

### FreeRTOS Tasks

| Task | Priority | Stack | Period | Chức năng |
|------|----------|-------|--------|-----------|
| `sensor_task` | 3 | 4KB | 500ms | Đọc cảm biến siêu âm |
| `led_task` | 2 | 2KB | 100ms | Điều khiển LED cảnh báo |
| `display_task` | 2 | 4KB | 200ms | Cập nhật màn hình OLED |
| `sdcard_task` | 2 | 4KB | 100ms | Lưu dữ liệu vào thẻ SD |
| `http_server_task` | 1 | 8KB | 30s | Quản lý HTTP server |
| `app_main` | 1 | 8KB | 10s | Monitor hệ thống |

## 🔧 Kết nối phần cứng

### ESP32-C3 Pin Mapping

| Chức năng | GPIO | Mô tả |
|-----------|------|-------|
| **OLED Display (SSD1306)** |
| SDA | 3 | I2C Data |
| SCL | 4 | I2C Clock |
| **Ultrasonic Sensor (HC-SR04)** |
| TRIG | 6 | Trigger Signal |
| ECHO | 7 | Echo Signal |
| **LED Warning** |
| LED | 1 | Warning LED (với điện trở 220Ω) |
| **SD Card (SPI)** |
| MISO | 19 | SPI Data In |
| MOSI | 23 | SPI Data Out |
| CLK | 18 | SPI Clock |
| CS | 5 | SPI Chip Select |

### Sơ đồ kết nối

```
ESP32-C3                    OLED Display
┌─────────┐                 ┌─────────┐
│ GPIO 3  │ ─────────────── │   SDA   │
│ GPIO 4  │ ─────────────── │   SCL   │
│   3.3V  │ ─────────────── │   VCC   │
│   GND   │ ─────────────── │   GND   │
└─────────┘                 └─────────┘

ESP32-C3                  HC-SR04 Sensor
┌─────────┐                 ┌─────────┐
│ GPIO 6  │ ─────────────── │  TRIG   │
│ GPIO 7  │ ─────────────── │  ECHO   │
│   5V    │ ─────────────── │   VCC   │
│   GND   │ ─────────────── │   GND   │
└─────────┘                 └─────────┘

ESP32-C3                    LED Warning
┌─────────┐                 ┌─────────┐
│ GPIO 1  │ ────[220Ω]───── │ Anode   │
│   GND   │ ─────────────── │ Cathode │
└─────────┘                 └─────────┘

ESP32-C3                      SD_CARD
┌─────────┐                 ┌─────────┐
│ GPIO 5  │ ─────────────── │   CS    │
│ GPIO 18 │ ─────────────── │   SCL   │
│ GPIO 23 │ ─────────────── │   MOSI  │
│ GPIO 19 │ ─────────────── │   MISO  │
└─────────┘                 └─────────┘
```

## 🚀 Cài đặt và Build

### Yêu cầu hệ thống

- **ESP-IDF**: v5.1 hoặc mới hơn
- **Python**: 3.8+
- **CMake**: 3.16+
- **Git**: Để clone repository

### Cài đặt ESP-IDF

```bash
# Clone ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.1

# Cài đặt ESP-IDF
./install.sh esp32c3

# Setup environment
. ./export.sh
```

### Build và Flash

```bash
# Clone project
git clone <repository-url>
cd Smart_Embed

# Configure project
idf.py set-target esp32c3
idf.py menuconfig

# Build project
idf.py build

# Flash firmware
idf.py -p /dev/ttyUSB0 flash monitor
```

### Cấu hình WiFi

Sửa file `components/esp32c3_wifi/esp32c3_wifi.c`:

```c
// WiFi Station configuration
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
```

## 🌐 Sử dụng hệ thống

### 1. Kết nối WiFi

Thiết bị sẽ tự động kết nối đến WiFi network được cấu hình:

```
I (12345) wifi: connected with Your_WiFi_SSID, channel 6
I (12346) wifi: ip: 192.168.1.100, mask: 255.255.255.0, gw: 192.168.1.1
```

### 2. Truy cập Web Interface

Mở trình duyệt và truy cập: `http://192.168.1.100`

**Tính năng Web Interface:**
- 📊 **Real-time Distance Chart**: Biểu đồ khoảng cách theo thời gian
- 💡 **LED Control**: Điều khiển LED từ xa
- 📈 **Performance Monitor**: Theo dõi hiệu suất hệ thống
- ⚙️ **Settings**: Tùy chỉnh tốc độ cập nhật và số điểm dữ liệu
- 📱 **Responsive Design**: Tương thích với mobile và desktop

### 3. API Endpoints

| Endpoint | Method | Mô tả |
|----------|--------|-------|
| `/hello` | GET | Trang web interface |
| `/ultrasonic` | GET | Lấy dữ liệu khoảng cách |
| `/led?state=on` | GET | Bật LED |
| `/led?state=off` | GET | Tắt LED |
| `/led/status` | GET | Kiểm tra trạng thái LED |
| `/sensor/history` | GET | Lấy dữ liệu lịch sử từ SD card |

**Ví dụ sử dụng API:**

```bash
# Lấy dữ liệu khoảng cách
curl http://192.168.1.100/ultrasonic

# Điều khiển LED
curl http://192.168.1.100/led?state=on
curl http://192.168.1.100/led?state=off

# Kiểm tra trạng thái LED
curl http://192.168.1.100/led/status
```

### 4. Dữ liệu SD Card

Dữ liệu cảm biến được lưu vào file `/sdcard/sensor.csv`:

```csv
25.30,1640995200000
24.85,1640995200500
26.12,1640995201000
```

**Format**: `distance,timestamp`

## 📊 Monitoring và Logging

### System Logs

```bash
# Xem logs real-time
idf.py monitor

# Lọc logs theo component
idf.py monitor --print-filter="smart_embed:*,ultrasonic:*,oled_driver:*"
```

### Performance Metrics

Hệ thống tự động log các thông tin sau:

```
I (5000) smart_embed: Distance: 25.3 cm
I (5200) smart_embed: LED ON - Distance: 8.5 cm < 10.0 cm
I (5400) smart_embed: System running - Distance: 25.3 cm, Valid: Yes, LED: OFF
I (30000) smart_embed: HTTP Server running - Distance: 25.3 cm, LED: OFF
```

### Web Performance Monitor

Web interface hiển thị:
- **FPS Counter**: Tốc độ cập nhật giao diện
- **Connection Status**: Trạng thái kết nối ESP32
- **Update Rate**: Tốc độ cập nhật dữ liệu (có thể tùy chỉnh)
- **Data Points**: Số điểm dữ liệu hiển thị trên biểu đồ

## 🔧 Tùy chỉnh và Mở rộng

### Thay đổi ngưỡng cảnh báo

Sửa trong `main/Smart_Embed.c`:

```c
#define DISTANCE_THRESHOLD 15.0f  // Thay đổi từ 10.0f thành 15.0f
```

### Tùy chỉnh tốc độ cập nhật

Sửa period của các tasks:

```c
// Cập nhật nhanh hơn (250ms thay vì 500ms)
vTaskDelay(pdMS_TO_TICKS(250));
```

### Thêm cảm biến mới

1. Tạo component mới trong `components/`
2. Thêm task trong `main/Smart_Embed.c`
3. Cập nhật web interface nếu cần

### Tùy chỉnh giao diện web

Chỉnh sửa file `main/index.html`:
- Thay đổi màu sắc trong CSS
- Thêm tính năng mới trong JavaScript
- Tùy chỉnh layout responsive

## 🐛 Troubleshooting

### Lỗi thường gặp

#### 1. ESP32 không kết nối WiFi

```bash
# Kiểm tra cấu hình WiFi
idf.py menuconfig
# Component config → ESP32C3 WiFi → Kiểm tra SSID và password
```

#### 2. OLED không hiển thị

- Kiểm tra kết nối I2C (GPIO 3, 4)
- Kiểm tra địa chỉ I2C (mặc định 0x3C)
- Thêm điện trở kéo lên 4.7kΩ cho SDA và SCL

#### 3. Cảm biến siêu âm không hoạt động

- Kiểm tra kết nối GPIO 6 (TRIG) và GPIO 7 (ECHO)
- Kiểm tra nguồn 5V cho HC-SR04
- Kiểm tra khoảng cách trong phạm vi 2cm - 4m

#### 4. SD Card không được nhận diện

- Kiểm tra kết nối SPI (GPIO 5, 18, 19, 23)
- Kiểm tra format thẻ SD (FAT32)
- Kiểm tra quyền ghi/đọc

#### 5. Web interface không load

- Kiểm tra kết nối WiFi
- Kiểm tra IP address của ESP32
- Kiểm tra HTTP server đã start chưa

### Debug Commands

```bash
# Xem thông tin chi tiết
idf.py monitor --print-filter="*:V"

# Kiểm tra heap memory
idf.py monitor | grep "heap"

# Kiểm tra task status
idf.py monitor | grep "task"
```

## 📈 Performance Optimization

### Memory Optimization

- Sử dụng `CONFIG_ESP32C3_SPIRAM_ENABLE=y` để bật external RAM
- Tối ưu stack size của các tasks
- Sử dụng `CONFIG_FREERTOS_HZ=1000` để tăng độ chính xác

### Network Optimization

- Sử dụng `CONFIG_LWIP_TCP_MSS=1460` để tối ưu TCP
- Bật `CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM=32` cho WiFi
- Sử dụng HTTP keep-alive để giảm overhead

### Power Optimization

- Sử dụng `esp_deep_sleep_start()` khi không cần thiết
- Tắt WiFi khi không sử dụng: `esp_wifi_stop()`
- Sử dụng `CONFIG_PM_ENABLE=y` để bật power management

## 🤝 Đóng góp

Chúng tôi hoan nghênh mọi đóng góp! Để đóng góp:

1. Fork repository
2. Tạo feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push branch: `git push origin feature/amazing-feature`
5. Tạo Pull Request

### Coding Standards

- Sử dụng ESP-IDF coding style
- Comment code bằng tiếng Việt hoặc tiếng Anh
- Test thoroughly trước khi commit
- Update documentation khi cần

## 📄 License

Dự án này được phân phối dưới giấy phép CC0-1.0. Xem file `LICENSE` để biết thêm chi tiết.

## 🙏 Acknowledgments

- **Espressif Systems** - ESP32-C3 và ESP-IDF framework
- **FreeRTOS** - Real-time operating system
- **Chart.js** - Web charting library
- **Font Awesome** - Icons cho web interface

## 📞 Liên hệ

- **Issues**: [GitHub Issues](https://github.com/your-repo/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-repo/discussions)
- **Email**: tutran56324@gmail.com

---

**Smart_Embed** - Hệ thống IoT giám sát khoảng cách thông minh với ESP32-C3 🚀

*Được phát triển với ❤️ bởi Tu_Linux_Embed*
