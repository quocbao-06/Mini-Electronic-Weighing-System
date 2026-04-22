#include <WiFi.h>
#include "ThingSpeak.h"
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

/* ================== KHAI BÁO PHẦN CỨNG ================== */
// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// HX711
#define HX711_SCK 5
#define HX711_DT  4
HX711 scale;

// Button bì
#define BTN_TARE     17
#define BTN_SendData 16

// Hệ số hiệu chuẩn 
float calibrationFactor = 100930.6931;  
long offset = 0;

float weight = 0;
float alpha = 0.5;
long rawValue = 0;

// ===== BIẾN CHỐNG DỘI NÚT =====
bool lastButtonTareState = HIGH;
bool buttonTareState = HIGH;

bool lastButtonSendState = HIGH;
bool buttonSendState = HIGH;

unsigned long lastDebounceTimeTare = 0;
unsigned long lastDebounceTimeSend = 0;
unsigned long debounceDelay = 50;

// ===== BIẾN RESET TARE ====
bool hadPositiveLoad = false;
unsigned long emptyStartTime = 0;

// Wifi config
const char* ssid = "Bboy Công Nghệ";
const char* password = "quocbao06";

// ThingSpeak config
const unsigned long channel_id = 3352024;
const char write_api_key[] = "1TRKGP8NLOLFUMRY";

WiFiClient client;


void setup() {
  Serial.begin(115200);

  // Khởi tạo nút nhấn
  pinMode(BTN_SendData, INPUT_PULLUP);
  pinMode(BTN_TARE, INPUT_PULLUP);

  // Khởi tạo LCD
  Wire.begin(21, 22);
  lcd.init();  
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);

  // Kết nối Wifi
  lcd.print("Dang ket noi WiFi...");
  WiFi.begin(ssid, password);

  lcd.setCursor(0, 1);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }

  lcd.setCursor(0, 2);
  lcd.print("Da ket noi WiFi!");
  lcd.setCursor(0, 3);
  lcd.print(WiFi.localIP());

  // Kết nối tới web ThingSpeak
  ThingSpeak.begin(client);

  delay(5000);
  
  // Hiện thị màn hình đo khối lượng vật
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Khoi luong vat");

  // Khởi tạo scale HX711
  scale.begin(HX711_DT, HX711_SCK);

  // Lấy offset ban đầu (bì)
  scale.tare();                 // Lấy offset
  offset = scale.get_offset();  // Lưu offset 

  lcd.setCursor(0, 1);
  lcd.print(weight, 3);   // 3 chữ số thập phân
  lcd.print(" Kg");
  delay(1500);
}

void loop() {
  // ====== NÚT BÌ ======
  CheckTare();

  // ====== ĐO LƯỜNG KHỐI LƯỢNG ======
  MeasureWeight();

  // ====== THÁO BÌ - RESET GIÁ TRỊ ======
  RemoveTareIfEmpty();

  // ====== GỬI DỮ LIỆU LÊN WEB ======
  SendDatatoWeb();

  // ====== HIỆN THỊ KHỐI LƯỢNG VẬT ======
  Display();
}

void SendDatatoWeb() {
  int reading = digitalRead(BTN_SendData);

  if (reading != lastButtonSendState) {
    lastDebounceTimeSend = millis();
  }

  if ((millis() - lastDebounceTimeSend) > debounceDelay) {

    if (reading != buttonSendState) {
      buttonSendState = reading;

      // Khi nút vừa được nhấn
      if (buttonSendState == LOW) {
        ThingSpeak.setField(1, weight);
        lcd.setCursor(0, 2);
        lcd.print("Posting ");
        lcd.print(weight, 3);
        lcd.print(" toWeb");
        int status = ThingSpeak.writeFields(channel_id, write_api_key);

        lcd.setCursor(0, 3);
        if (status == 200) {
          lcd.print("Sent successfully");
        } else {
          lcd.println("ThingSpeak Error");
        }
        delay(2000);
        lcd.setCursor(0, 2);
        lcd.print("                    "); 
        lcd.setCursor(0, 3);
        lcd.print("                    "); 
      }
    }
  }
  lastButtonSendState = reading;
}

void CheckTare() {
  int reading = digitalRead(BTN_TARE);

  if (reading != lastButtonTareState) {
    lastDebounceTimeTare = millis();
  }

  if ((millis() - lastDebounceTimeTare) > debounceDelay) {

    if (reading != buttonTareState) {
      buttonTareState = reading;

      // Khi nút vừa được nhấn
      if (buttonTareState == LOW) {

        // Kiểm tra độ ổn định trước khi tare
        long r1 = scale.read();
        delay(20);
        long r2 = scale.read();
        delay(20);
        long r3 = scale.read();

        if (abs(r1 - r2) < 50 && abs(r2 - r3) < 50) {

          lcd.setCursor(0, 1);
          lcd.print("Dang tare...   ");

          scale.tare();
          offset = scale.get_offset();

          weight = 0;

          lcd.setCursor(0, 1);
          lcd.print("0.000 Kg       ");
        }
        else {
          lcd.setCursor(0, 1);
          lcd.print("Khong on dinh  ");

          lcd.setCursor(0, 1);
          lcd.print(weight, 3);   // 3 chữ số thập phân
          lcd.print(" Kg       ");
        }
      }
    }
  }
  lastButtonTareState = reading;
}

void MeasureWeight() {
  // Đọc trung bình 3 mẫu (giảm chậm hệ thống)
  rawValue = scale.read_average(6);

  // Tính khối lượng thô   raw = calib * weight + offset
  float newWeight = (rawValue - offset) / calibrationFactor;

  // LỌC EMA (mượt dần) Exponential Moving Average (Lọc trung bình trượt mũ)
  weight = alpha * newWeight + (1 - alpha) * weight;

  // DEAD BAND (ngưỡng chết 5g)
  if (abs(weight) < 0.005) {
    weight = 0;
  }
}

void RemoveTareIfEmpty() {
  // Nếu từng có tải lớn hơn 5g
  if (weight > 0.005) {    // 0.05
    hadPositiveLoad = true;
  }

  // Nếu đã từng có tải và hiện tại âm đáng kể
  if (hadPositiveLoad && weight < -0.005) {

    if (emptyStartTime == 0) {
        emptyStartTime = millis();
    }

    if (millis() - emptyStartTime > 800) {
        scale.tare();
        offset = scale.get_offset();
        weight = 0;
        hadPositiveLoad = false;
        emptyStartTime = 0;
    }

  } else {
    emptyStartTime = 0;
  }
}

void Display() {
  lcd.setCursor(0, 1);
  lcd.print(weight, 3);   // 3 chữ số thập phân
  lcd.print(" Kg  ");

  Serial.print("Gia tri tho: ");
  Serial.print(rawValue);
  Serial.print("    Offset: ");
  Serial.println(offset);
}