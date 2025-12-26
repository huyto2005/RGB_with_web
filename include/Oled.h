#ifndef OLED_H
#define OLED_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CẤU HÌNH ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C 
#define OLED_SDA 21
#define OLED_SCL 22

class OledDisplay {
private:
    Adafruit_SSD1306 display;

public:
    OledDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

    void begin() {
        Serial.print(F("[OLED] Khoi dong I2C... "));
        Wire.begin(OLED_SDA, OLED_SCL);
        
        if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
            Serial.println(F("THAT BAI! (Check day SDA/SCL)"));
            return; 
        }
        Serial.println(F("OK!"));
        
        // --- MÀN HÌNH CHÀO "Babyboy" ---
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        
        // Chữ Babyboy to
        display.setTextSize(2); 
        display.setCursor(20, 20); // Căn giữa
        display.println(F("Babyboy"));
        
        // Dòng chữ nhỏ bên dưới
        display.setTextSize(1);
        display.setCursor(35, 45);
        display.println(F("System Init..."));
        
        display.display();
    }

    void showWifiConnecting(String ssid) {
        display.clearDisplay();
        
        // Vẫn giữ Header Babyboy
        display.setTextSize(2); 
        display.setCursor(20, 10);
        display.println(F("Babyboy"));

        display.setTextSize(1);
        display.setCursor(0, 40);
        display.print("Connecting: ");
        display.println(ssid);
        display.display();
    }

    // ⭐ CẬP NHẬT: Thêm tham số bright
    void updateStatus(String mode, int r, int g, int b, int bright, bool mqttConnected, String ip) {
        display.clearDisplay();
        
        // Header
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(mqttConnected ? "MQTT: OK" : "NO MQTT");
        display.setCursor(70, 0);
        display.print(ip.substring(ip.lastIndexOf('.')+1)); 
        display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

        // Body: Mode
        display.setCursor(0, 15);
        display.print("Mode:");
        display.setCursor(35, 15);
        String displayMode = mode;
        displayMode.toUpperCase();
        display.println(displayMode);

        // Body: Brightness (MỚI)
        display.setCursor(0, 30);
        display.print("Bri: ");
        int percent = map(bright, 0, 255, 0, 100); // Đổi 0-255 về 0-100%
        display.setTextSize(2); 
        display.print(percent + 1);
        display.setTextSize(1);
        display.print("%");

        // Footer: RGB
        display.setCursor(0, 52);
        display.printf("R: %d G: %d B: %d", r, g, b); 

        display.display();
    }
};

#endif