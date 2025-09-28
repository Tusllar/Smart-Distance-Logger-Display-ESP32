# Smart Distance Logger & Display - FreeRTOS Tasks

## 🏗️ Cấu trúc Task

### **Task 1: Sensor Task (sensor_task)**
- **Priority**: 2 (cao hơn)
- **Stack Size**: 4096 bytes
- **Chức năng**: 
  - Đọc dữ liệu từ cảm biến siêu âm mỗi 500ms
  - Cập nhật biến global `g_distance` và `g_distance_valid`
  - Log dữ liệu ra console

### **Task 2: Display Task (display_task)**
- **Priority**: 1 (thấp hơn)
- **Stack Size**: 4096 bytes
- **Chức năng**:
  - Hiển thị dữ liệu lên OLED mỗi 200ms
  - Đọc dữ liệu từ biến global
  - Cập nhật giao diện người dùng

### **Task 3: LED Task (led_task)**
- **Priority**: 2 (trung bình)
- **Stack Size**: 2048 bytes
- **Chức năng**:
  - Kiểm tra khoảng cách mỗi 100ms
  - Bật LED khi distance < 10cm
  - Tắt LED khi distance >= 10cm
  - GPIO: GPIO 1

### **Task 4: HTTP Server Task (http_server_task)**
- **Priority**: 1 (thấp)
- **Stack Size**: 8192 bytes
- **Chức năng**:
  - Khởi động HTTP server sau 5 giây
  - Cung cấp web interface để xem dữ liệu
  - Monitor HTTP server mỗi 30 giây
  - WiFi AP: Smart_Embed_AP (Password: 12345678)

### **Task 5: Main Task (app_main)**
- **Priority**: 1 (mặc định)
- **Chức năng**:
  - Khởi tạo hệ thống (WiFi, OLED, Sensor)
  - Tạo các task khác
  - Monitor hệ thống mỗi 10 giây

## 🔄 Luồng hoạt động

```
app_main()
    ├── Khởi tạo WiFi AP
    ├── Khởi tạo OLED
    ├── Khởi tạo cảm biến siêu âm
    ├── Tạo sensor_task (Priority 2)
    ├── Tạo display_task (Priority 1)
    ├── Tạo led_task (Priority 2)
    ├── Tạo http_server_task (Priority 1)
    └── Monitor hệ thống (mỗi 10s)

sensor_task()                    display_task()                    led_task()                    http_server_task()
    ├── Đọc cảm biến (500ms)         ├── Đọc g_distance              ├── Kiểm tra distance (100ms)   ├── Chờ 5s
    ├── Cập nhật g_distance          ├── Hiển thị lên OLED (200ms)   ├── Bật LED nếu < 10cm          ├── Khởi động HTTP server
    └── Log dữ liệu                  └── Cập nhật giao diện          └── Tắt LED nếu >= 10cm         └── Monitor server (30s)
```

## 📊 Biến Global

```c
static oled_driver_t *g_oled = NULL;        // Driver OLED
static float g_distance = 0.0f;             // Khoảng cách đo được
static bool g_distance_valid = false;       // Trạng thái dữ liệu
```

## ⚡ Ưu điểm của FreeRTOS Tasks

1. **Song song hóa**: Cảm biến và hiển thị chạy độc lập
2. **Ưu tiên**: Sensor task có priority cao hơn
3. **Hiệu suất**: Display cập nhật nhanh hơn (200ms vs 500ms)
4. **Mở rộng**: Dễ dàng thêm task mới
5. **Quản lý**: Dễ debug và monitor

## 🔧 Cấu hình Task

| Task | Priority | Stack | Period | Function |
|------|----------|-------|--------|----------|
| sensor_task | 2 | 4KB | 500ms | Đọc cảm biến |
| led_task | 2 | 2KB | 100ms | Điều khiển LED |
| display_task | 1 | 4KB | 200ms | Hiển thị OLED |
| http_server_task | 1 | 8KB | 30s | HTTP Server |
| app_main | 1 | 8KB | 10s | Monitor |

## 🎯 Kết quả

- **Cảm biến**: Đọc mỗi 500ms, không bị block
- **LED**: Phản hồi nhanh mỗi 100ms khi distance < 10cm
- **Hiển thị**: Cập nhật mỗi 200ms, mượt mà
- **Web Interface**: Truy cập qua http://192.168.4.1
- **WiFi AP**: Smart_Embed_AP (Password: 12345678)
- **Hệ thống**: Ổn định, có thể mở rộng
- **Log**: Thông tin chi tiết về hoạt động và trạng thái LED
