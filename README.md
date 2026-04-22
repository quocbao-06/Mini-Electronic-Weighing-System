📌 Mini Electronic Weighing System (ESP32)
📖 Giới thiệu

Đây là hệ thống cân điện tử mini sử dụng ESP32, có khả năng đo khối lượng, hiển thị trên LCD và gửi dữ liệu lên nền tảng IoT.

Hệ thống sử dụng loadcell + HX711 để đo chính xác khối lượng và WiFi (ESP32) để truyền dữ liệu lên ThingSpeak.

⚙️ Phần cứng sử dụng

  ESP32 DevKitC-32D
  
  Loadcell (cảm biến lực)
  
  Module HX711
  
  LCD 2004 I2C
  
  Nút nhấn (tare, gửi dữ liệu)
  
  Nguồn 5V

🔧 Chức năng chính

  ✅ Đo khối lượng vật
  
  ✅ Lọc nhiễu (EMA + deadband)
  
  ✅ Chức năng tare (trừ bì)
  
  ✅ Hiển thị lên LCD 20x4
  
  ✅ Gửi dữ liệu lên ThingSpeak qua WiFi
  
  ✅ Tự động reset tare khi bỏ vật

🌐 IoT - ThingSpeak

  Gửi dữ liệu qua giao thức HTTP
  
  Hiển thị dữ liệu dạng biểu đồ theo thời gian
  
  Giới hạn: ~15 giây/lần (tài khoản free)

🧠 Thuật toán xử lý

  Trung bình mẫu (average)
  
  Lọc EMA (Exponential Moving Average)
  
  Deadband (loại nhiễu nhỏ)
  
  Debounce nút nhấn

📊 Kết quả
  Hiển thị khối lượng realtime trên LCD
  
  Gửi dữ liệu lên ThingSpeak
  
  Theo dõi dữ liệu từ xa
  
⚠️ Lưu ý
  Cần hiệu chuẩn (calibration) để chính xác
  
  Tránh rung động khi đo
  
  Kiểm tra địa chỉ I2C (0x27 hoặc 0x3F)
  
📌 Hướng phát triển

  App mobile theo dõi dữ liệu
  
  Lưu dữ liệu SD card
  
  Thêm Bluetooth
  
  Tối ưu tiết kiệm năng lượng
