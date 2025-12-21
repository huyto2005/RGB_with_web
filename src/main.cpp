#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "LedRGB.h" 

// ================= CẤU HÌNH HIVEMQ CLOUD =================
const char* ssid = "Huy tồ";           
const char* password = "123456789";    

const char* mqtt_server = "53beae9b0e4b41f3b1b2dc76a5b641eb.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;            
const char* mqtt_user = "esp32_user";  
const char* mqtt_pass = "Esp123456";   
const char* mqtt_topic = "esp32/led/control"; 

WiFiClientSecure espClient; 
PubSubClient client(espClient);
LedRGB led(5, 8); 

String currentMode = "static";

void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) message += (char)payload[i];
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    if (error) return;

    // 1. Cập nhật Nguồn
    if (doc.containsKey("power")) {
        String power = doc["power"];
        if (power == "OFF") led.setPower(false);
        else if (!led.powerState) led.setPower(true);
    }
    
    // 2. Lấy dữ liệu
    String newMode = doc["mode"];
    int r = doc["color"]["r"];
    int g = doc["color"]["g"];
    int b = doc["color"]["b"];
    int bright = doc["brightness"];
    
    // Cập nhật thông số màu (luôn cập nhật)
    led.updateColorParams(r, g, b);
    
    // Cập nhật độ sáng
    int pwmBright = map(bright, 0, 100, 0, 255);
    led.setBrightness(pwmBright); 

    // 3. Xử lý BEAT
    if (currentMode == "music" && doc["beat"] == true) {
        led.triggerBeat();
        return; 
    }

    // 4. Xử lý Đổi Mode
    if (newMode != "null" && newMode != "") {
        if (newMode != currentMode) {
            // Chuyển Mode: Gọi setEffect để reset trạng thái cũ và chạy cái mới
            currentMode = newMode;
            
            if (currentMode == "static") led.setColor(r, g, b);
            else if (currentMode == "rainbow") led.setEffect("rainbow", 20, true); 
            else if (currentMode == "chase") led.setEffect("chase", 50, false);
            else if (currentMode == "breath") led.setEffect("fade", 30, false);
            else if (currentMode == "music") led.setEffect("music", 10); 
        } 
        else {
            // Mode không đổi: Chỉ cập nhật màu nếu đang ở mode cho phép chỉnh màu
            if (currentMode == "static") {
                led.setColor(r, g, b);
            }
            // Chase/Breath tự lấy màu mới từ updateColorParams trong vòng lặp tiếp theo
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        String clientId = "ESP32Client-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            Serial.println("Connected Cloud");
            client.subscribe(mqtt_topic); 
        } else {
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    led.begin();
    led.setPower(true);
    led.setColor(0, 0, 255); 

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    
    espClient.setInsecure(); 
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) reconnect();
    client.loop();
    
    // ⭐ QUAN TRỌNG: Gọi hàm loop của LED để chạy hiệu ứng
    led.loop(); 
}


