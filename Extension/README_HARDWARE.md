# Smart Distance Logger & Display - Hướng dẫn kết nối phần cứng

## 🔌 Kết nối phần cứng

### **OLED Display (SSD1306)**
| OLED Pin | ESP32-C3 Pin | Mô tả |
|----------|--------------|-------|
| VCC      | 3.3V         | Nguồn 3.3V |
| GND      | GND          | Mass |
| SCL      | GPIO 4       | Clock I2C |
| SDA      | GPIO 3       | Data I2C |

### **Cảm biến siêu âm HC-SR04**
| HC-SR04 Pin | ESP32-C3 Pin | Mô tả |
|-------------|--------------|-------|
| VCC         | 5V hoặc 3.3V | Nguồn |
| GND         | GND          | Mass |
| TRIG        | GPIO 6       | Tín hiệu kích hoạt |
| ECHO        | GPIO 7       | Tín hiệu phản hồi |

### **LED Warning**
| LED Pin | ESP32-C3 Pin | Mô tả |
|---------|--------------|-------|
| Anode    | GPIO 1       | Điều khiển LED |
| Cathode  | GND          | Mass (qua điện trở 220Ω) |

## ⚠️ Lưu ý quan trọng

1. **GPIO đã được sửa**: Không còn xung đột GPIO
   - OLED: GPIO 3, 4
   - HC-SR04: GPIO 6, 7  
   - LED: GPIO 1

2. **Nguồn điện**: 
   - OLED: 3.3V
   - HC-SR04: 5V (hoặc 3.3V nếu module hỗ trợ)
   - LED: 3.3V (qua điện trở 220Ω)

3. **Điện trở kéo lên**: Có thể cần điện trở 4.7kΩ cho SDA và SCL của I2C

4. **LED**: Cần điện trở 220Ω để bảo vệ LED khỏi quá dòng

## 🔧 Sửa lỗi xung đột GPIO

Nếu bạn gặp xung đột GPIO, hãy sửa file `ultrasonic_sensor.h`:

```c
// Thay đổi từ:
#define ECHO_PIN GPIO_NUM_5  // GPIO 5 cho ECHO

// Thành:
#define ECHO_PIN GPIO_NUM_6  // GPIO 6 cho ECHO
```

## 📋 Chức năng hệ thống

- **Hiển thị**: Khoảng cách đo được từ cảm biến siêu âm
- **Cảnh báo**: LED sáng khi khoảng cách < 10cm
- **Web Interface**: Truy cập qua WiFi AP
- **Cập nhật**: Cảm biến 500ms, Hiển thị 200ms, LED 100ms
- **Phạm vi**: 2cm - 4m
- **Giao diện**: OLED 128x64 pixels + Web browser

## 🎯 Kết quả mong đợi

Màn hình OLED sẽ hiển thị:
```
    Distance Logger
      Smart Embed
    ────────────────
   Distance: 25.3 cm
    Status: OK
```

**LED sẽ sáng khi khoảng cách < 10cm và tắt khi >= 10cm**

## 🌐 Web Interface

1. **Kết nối WiFi**: 
   - SSID: `Smart_Embed_AP`
   - Password: `12345678`

2. **Truy cập web**: 
   - URL: `http://192.168.4.1`
   - Xem dữ liệu khoảng cách real-time
   - Điều khiển LED (nếu có)

3. **Tính năng web**:
   - Hiển thị khoảng cách hiện tại
   - Trạng thái LED
   - Log dữ liệu
   - Giao diện responsive
