#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "LedRGB.h"
#include "Oled.h" 

// --- CẤU HÌNH ---
const char* ssid = "Huy tồ";           
const char* password = "123456789";    
const char* mqtt_server = "4137969922e04db986eb36cd1271ac95.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;            
const char* mqtt_user = "esp32_user";  
const char* mqtt_pass = "Esp123456";   
const char* mqtt_topic = "esp32/led/control"; 

WiFiClientSecure espClient; 
PubSubClient client(espClient);
LedRGB led(5, 8); 
OledDisplay oled; 

String currentMode = "static";
// ⭐ Biến displayBright lưu độ sáng hiện tại (0-255)
int displayR = 0, displayG = 0, displayB = 255, displayBright = 255; 

bool needUpdateScreen = false;
unsigned long lastScreenDraw = 0;
unsigned long lastHeartBeat = 0;

void callback(char* topic, byte* payload, unsigned int length) {
    // --- GIỮ NGUYÊN CODE DEBUG SERIAL ---
    Serial.print("\n>>> [MQTT] REC: ");
    String message = "";
    for (int i = 0; i < length; i++) message += (char)payload[i];
    Serial.println(message);
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print("[ERROR] JSON Malformed: ");
        Serial.println(error.c_str());
        return;
    }

    String newMode = doc["mode"] | currentMode; 
    int r = doc["color"]["r"];
    int g = doc["color"]["g"];
    int b = doc["color"]["b"];
    int bright = doc["brightness"] | -1; 

    // Cập nhật biến hiển thị
    displayR = r; displayG = g; displayB = b;
    // Nếu có độ sáng mới thì cập nhật biến hiển thị
    if (bright != -1) {
        // Web gửi 0-100, đổi sang 0-255 để lưu chuẩn LED
        displayBright = map(bright, 0, 100, 0, 255);
    }
    needUpdateScreen = true;

    // --- LOGIC ĐIỀU KHIỂN LED ---
    if (doc.containsKey("power")) {
        String power = doc["power"];
        Serial.print("[ACTION] Set Power: "); Serial.println(power);
        led.setPower(power != "OFF");
    }

    led.updateColorParams(r, g, b);
    if (bright != -1) {
        led.setBrightness(displayBright); 
    }

    if (newMode != "null" && newMode != "") {
        if (newMode == "static") {
            currentMode = "static";
            Serial.println("[MAIN] Goi lenh led.setColor...");
            led.setColor(r, g, b); 
        }
        else if (newMode != currentMode) {
            currentMode = newMode;
            Serial.print("[MAIN] Chuyen Mode sang: "); Serial.println(newMode);
            
            if (currentMode == "rainbow") led.setEffect("rainbow", 20, true); 
            else if (currentMode == "chase") led.setEffect("chase", 50, false);
            else if (currentMode == "breath") led.setEffect("fade", 30, false);
            else if (currentMode == "music") led.setEffect("music", 10); 
        }
    }

    if (currentMode == "music" && doc["beat"] == true) {
        led.triggerBeat();
    }
    
    Serial.println(">>> [MQTT] XU LY XONG.");
}

void reconnect() {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = millis();
        Serial.print("[MQTT] Dang ket noi...");
        String clientId = "ESP32-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            Serial.println(" OK!");
            client.subscribe(mqtt_topic);
            needUpdateScreen = true;
        } else {
            Serial.print(" Fail rc="); Serial.println(client.state());
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\n================ SYSTEM STARTING (FULL DEBUG) ================");

    // 1. Khởi động LED
    Serial.println("[SETUP] Khoi dong LED...");
    led.begin();
    led.setPower(true);
    led.setColor(0, 0, 255); 
    Serial.println("[SETUP] LED OK.");

    // 2. Khởi động OLED -> Hiện "Babyboy"
    Serial.println("[SETUP] Khoi dong OLED...");
    oled.begin(); 
    
    // ⭐ Delay 2 giây để bạn kịp nhìn thấy chữ "Babyboy"
    Serial.println("[SETUP] Hien thi man hinh chao...");
    delay(2000); 

    oled.showWifiConnecting(ssid);
    Serial.println("[SETUP] OLED OK.");

    // 3. WiFi
    Serial.print("[SETUP] Ket noi WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\n[SETUP] WiFi Connected! IP: " + WiFi.localIP().toString());

    espClient.setInsecure(); 
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
    Serial.println("================ SYSTEM READY ================");
}

void loop() {
    if (!client.connected()) reconnect();
    client.loop();
    led.loop(); 

    // Cập nhật OLED (Truyền thêm displayBright)
    if (needUpdateScreen && (millis() - lastScreenDraw > 200)) {
        oled.updateStatus(currentMode, displayR, displayG, displayB, displayBright, client.connected(), WiFi.localIP().toString());
        lastScreenDraw = millis();
        needUpdateScreen = false; 
    }
    
    // Heartbeat log
    if (millis() - lastHeartBeat > 5000) {
        Serial.println("[LOOP] System running...");
        lastHeartBeat = millis();
    }
}