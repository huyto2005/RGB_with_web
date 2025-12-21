#ifndef COREIOTMQTT_H
#define COREIOTMQTT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class CoreIoTMQTT {
public:
    WiFiClient espClient;
    PubSubClient mqtt;

    String token;
    String server;
    int port;

    std::function<void(JsonDocument&)> onRpcReceived;

    CoreIoTMQTT(const char* server, int port, const char* token)
        : mqtt(espClient), server(server), port(port), token(token) {}

    void begin() {
        mqtt.setServer(server.c_str(), port);
        mqtt.setCallback([this](char* topic, byte* payload, unsigned int len) {
            this->callback(topic, payload, len);
        });
    }

    void connect() {
        while (!mqtt.connected()) {
            Serial.print("[MQTT] Connecting to CoreIoT... ");
            if (mqtt.connect("DEVICE", token.c_str(), NULL)) {
                Serial.println("OK");
                mqtt.subscribe("v1/devices/me/rpc/request/+");
                Serial.println("[MQTT] Subscribed RPC topic");
            } else {
                Serial.println("Fail → retrying");
                delay(2000);
            }
        }
    }

    void loop() {
        if (!mqtt.connected()) connect();
        mqtt.loop();
    }

    void sendTelemetry(JsonDocument& doc) {
        String data;
        serializeJson(doc, data);
        mqtt.publish("v1/devices/me/telemetry", data.c_str());
        Serial.println("[MQTT] Sent telemetry: " + data);
    }

private:

    void callback(char* topic, byte* payload, unsigned int len) {
        String msg;
        for (int i = 0; i < len; i++) msg += (char)payload[i];

        Serial.println("[MQTT] RPC received: " + msg);

        DynamicJsonDocument doc(256);
        if (deserializeJson(doc, msg)) return;

        if (onRpcReceived)
            onRpcReceived(doc);

        int rpcId = doc["id"] | 0;

        DynamicJsonDocument resp(64);
        resp["success"] = true;

        String out;
        serializeJson(resp, out);
        mqtt.publish(("v1/devices/me/rpc/response/" + String(rpcId)).c_str(), out.c_str());
    }

    // TODO: Thêm publishTopic(), subscribeTopic() tùy mục đích
};

#endif
