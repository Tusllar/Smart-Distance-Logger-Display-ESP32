# Binary Data Configuration

## 📁 Cấu trúc file

```
Smart_Embed/
├── main/
│   ├── Smart_Embed.c
│   ├── CMakeLists.txt
│   └── index.html          ← File HTML cho web interface
├── components/
│   └── http_server_app/
│       ├── CMakeLists.txt  ← Cấu hình binary data
│       ├── http_server_app.c
│       └── include/
│           └── http_server_app.h
```

## 🔧 CMakeLists.txt Configuration

```cmake
idf_component_register(SRCS "http_server_app.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_wifi esp_netif esp_event esp_http_server esp_timer ultrasonic_sensor)

# Thêm file HTML vào binary data
target_add_binary_data(${COMPONENT_TARGET} "../../main/index.html" TEXT)
```

## 📋 Giải thích

### **target_add_binary_data()**
- **Mục đích**: Nhúng file HTML vào firmware ESP32
- **Cú pháp**: `target_add_binary_data(target "path" TYPE)`
- **Tham số**:
  - `target`: Component target (${COMPONENT_TARGET})
  - `path`: Đường dẫn đến file HTML
  - `TYPE`: TEXT (cho file text/HTML)

### **Đường dẫn file**
- **Từ**: `components/http_server_app/CMakeLists.txt`
- **Đến**: `main/index.html`
- **Đường dẫn tương đối**: `../../main/index.html`

## 🔄 Cách hoạt động

1. **Build time**: CMake đọc file `index.html`
2. **Compile**: Chuyển đổi thành binary data
3. **Link**: Nhúng vào firmware ESP32
4. **Runtime**: HTTP server truy cập qua symbol:
   ```c
   extern const uint8_t index_html_start[] asm("_binary_index_html_start");
   extern const uint8_t index_html_end[] asm("_binary_index_html_end");
   ```

## ✅ Kết quả

- **File HTML**: Được nhúng vào firmware
- **Web server**: Có thể serve file HTML
- **Không cần**: File system riêng biệt
- **Tự động**: Cập nhật khi rebuild

## 🎯 Lợi ích

1. **Đơn giản**: Không cần SPIFFS/LittleFS
2. **Tích hợp**: File HTML trong firmware
3. **Hiệu quả**: Truy cập nhanh từ RAM
4. **Bảo mật**: File không thể thay đổi từ bên ngoài
