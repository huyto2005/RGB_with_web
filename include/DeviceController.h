#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include "LedRGB.h"
#include "CoreIoTMQTT.h"

class DeviceController {
public:
    LedRGB led;
    CoreIoTMQTT mqtt;

    DeviceController()
        : led(5, 8), // GPIO 5, 8 bóng LED
          mqtt("10.0.224.107", 1883, "") // Token server trống vì không dùng
    {}

    void begin() {
        Serial.println("=== Device Controller Started (Final Version) ===");
        led.begin();
        mqtt.begin();

        // Xử lý lệnh từ Dashboard gửi xuống
        mqtt.onRpcReceived = [this](JsonDocument& doc) {
            // --- DEBUG ---
            String jsonString;
            serializeJson(doc, jsonString);
            Serial.println("[DEBUG] Received JSON: " + jsonString);
            // -----------------------------
            if (!doc.containsKey("method")) return;
            String method = doc["method"].as<String>();

            // ⭐ 1. LỆNH ON/OFF (Công tắc tổng)
            if (method == "setPower") {
                // Nhận giá trị true/false từ switch
                bool state = doc["params"]; 
                led.setPower(state);
                
                // Gửi lại trạng thái để đồng bộ switch trên web (nếu cần)
                sendTelemetry(); 
            }

            // ⭐ 2. Các lệnh khác -> Chỉ chạy khi Power ON (Đã chặn bên LedRGB, nhưng chặn ở đây cũng tốt)
            else if (method == "setBrightness") {
                if (led.powerState) { // Check nhẹ
                    int val = doc["params"];
                    led.setBrightness(val);
                }
            }
            else if (method == "setColor") {
                if (led.powerState) {
                    int r = doc["params"]["r"];
                    int g = doc["params"]["g"];
                    int b = doc["params"]["b"];
                    
                    if (led.currentEffect == CHASE || led.currentEffect == FADE) {
                        led.updateColorParams(r, g, b);
                    } else {
                        led.setColor(r, g, b);
                    }
                    sendTelemetry();
                }
            }
            else if (method == "setEffect") {
                if (led.powerState) {
                    String name = doc["params"]["name"] | "static";
                    int speed = doc["params"]["speed"] | 50;
                    bool isRainbow = doc["params"]["rainbow"] | false;
                    
                    if (doc["params"].containsKey("r")) {
                        led.r = doc["params"]["r"];
                        led.g = doc["params"]["g"];
                        led.b = doc["params"]["b"];
                    }
                    led.setEffect(name, speed, isRainbow);
                }
            }
        };
    }

    void loop() {
        mqtt.loop();
        // Không cần gọi led.loop() vì đã dùng Ticker
    }

private:
    void sendTelemetry() {
        DynamicJsonDocument doc(128);
        doc["r"] = led.r;
        doc["g"] = led.g;
        doc["b"] = led.b;
        mqtt.sendTelemetry(doc);
    }
};

#endif

// run --target upload --upload-port COM8 