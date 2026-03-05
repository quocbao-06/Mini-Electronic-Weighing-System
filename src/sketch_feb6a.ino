#include <LiquidCrystal.h>
#include "HX711.h"

/* ================== KHAI BÁO PHẦN CỨNG ================== */
// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 2, 3, 4, 5);

// HX711
#define HX711_SCK A0
#define HX711_DT  A1
HX711 scale;

// Button bì
#define BTN_TARE 13

// Hệ số hiệu chuẩn 
float calibrationFactor = 99537.5;  
long offset = 0;

float weight = 0;
float alpha = 0.5;
long rawValue = 0;

// ===== BIẾN CHỐNG DỘI NÚT =====
bool lastButtonState = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// ===== BIẾN RESET TARE ====
bool hadPositiveLoad = false;
unsigned long emptyStartTime = 0;

/* ======================================================= */

void setup() {
  Serial.begin(9600);
  pinMode(BTN_TARE, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Khoi luong vat");

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
  /* ================== NÚT BÌ (CHỐNG NHIỄU) ================== */
  CheckTare();

  /* ======== ĐO ĐẠC - LỌC NHIỄU CHUẨN ======== */
  MeasureWeight();
  
  /* ===== BỎ BÌ RA KHỎI CÂN - AUTO RESET AN TOÀN ===== */
  RemoveTareIfEmpty();


  /* ================== HIỂN THỊ ================== */
  Display();
 
}

void CheckTare() {
  int reading = digitalRead(BTN_TARE);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;

      // Khi nút vừa được nhấn
      if (buttonState == LOW) {

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
        }
      }
    }
  }

  lastButtonState = reading;
}

void MeasureWeight() {
  // Đọc trung bình 3 mẫu (giảm chậm hệ thống)
  rawValue = scale.read_average(3);

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