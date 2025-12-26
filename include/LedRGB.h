#ifndef LEDRGB_H
#define LEDRGB_H

#include <Adafruit_NeoPixel.h>

enum EffectState { STATIC, RAINBOW, FADE, CHASE, MUSIC };

class LedRGB {
private:
    int pin;
    int count;
    Adafruit_NeoPixel* pixels;
    
    unsigned long lastUpdate = 0;
    int effectSpeed = 0;
    
    long step = 0;
    int fadeDir = 1;
    int fadeVal = 0;
    
    bool useRainbowColor = false;
    uint16_t currentHue = 0;
    
    int musicBrightness = 0;
    uint32_t musicColor = 0; 
    const int baseBrightness = 60; 

public:
    int r = 0, g = 0, b = 0;
    int globalBrightness = 255;
    EffectState currentEffect = STATIC;
    bool powerState = true; 

    LedRGB(int pin, int count = 8) : pin(   pin), count(count) {
        pixels = new Adafruit_NeoPixel(count, pin, NEO_GRB + NEO_KHZ800);
    }

    void begin() {
        pixels->begin();
        pixels->setBrightness(globalBrightness);
        pixels->show();
        Serial.println("[LED_DRIVER] Da khoi tao NeoPixel tai chan GPIO " + String(pin));
    }

    void setPower(bool state) {
        powerState = state;
        Serial.print("[LED_DRIVER] Set Power: "); Serial.println(state ? "ON" : "OFF");
        if (powerState) {
            pixels->setBrightness(globalBrightness);
            if (currentEffect == STATIC) {
                // Vẽ lại màu cũ
                uint32_t c = pixels->Color(r, g, b);
                for(int i=0; i<count; i++) pixels->setPixelColor(i, c);
                pixels->show();
            }
        } else {
            pixels->clear();
            pixels->show();
        }
    }

    void setBrightness(int b) {
        if (!powerState) return;
        if (b < 0) b = 0; if (b > 255) b = 255;
        globalBrightness = b;
        
        Serial.printf("[LED_DRIVER] Set Brightness: %d\n", b);

        if (currentEffect != MUSIC) {
            pixels->setBrightness(globalBrightness);
            if (currentEffect == STATIC) {
                for(int i=0; i<count; i++) pixels->setPixelColor(i, pixels->Color(r, g, b));
                pixels->show();
            }
        }
    }

    void updateColorParams(int r, int g, int b) {
        this->r = r; this->g = g; this->b = b;
        useRainbowColor = false;
    }

    void setColor(int r, int g, int b) {
        if (!powerState) {
            Serial.println("[LED_DRIVER] Bo qua lenh SetColor vi Power dang OFF");
            return;
        }
        
        Serial.printf("[LED_DRIVER] Set Color STATIC: %d, %d, %d\n", r, g, b);

        currentEffect = STATIC;
        useRainbowColor = false;
        this->r = r; this->g = g; this->b = b;
        
        pixels->setBrightness(globalBrightness);
        for(int i=0; i<count; i++) pixels->setPixelColor(i, pixels->Color(r, g, b));
        pixels->show(); // <--- Lệnh quan trọng nhất để đèn sáng
    }

    void setEffect(String effectName, int speedMs, bool isRainbow = false) {
        if (!powerState) return;
        
        Serial.println("[LED_DRIVER] Chuyen Effect: " + effectName);

        step = 0;
        effectSpeed = speedMs;
        lastUpdate = millis();
        useRainbowColor = isRainbow;

        if (effectName == "rainbow") currentEffect = RAINBOW;
        else if (effectName == "fade") { currentEffect = FADE; fadeVal = 0; }
        else if (effectName == "chase") currentEffect = CHASE;
        else if (effectName == "music") { 
            currentEffect = MUSIC; 
            musicBrightness = baseBrightness; 
            pixels->setBrightness(musicBrightness);
            musicColor = pixels->ColorHSV(random(0, 65535), 255, 255);
            for(int i=0; i<count; i++) pixels->setPixelColor(i, musicColor);
            pixels->show();
        }
        else currentEffect = STATIC;

        if (currentEffect != MUSIC) {
            pixels->setBrightness(globalBrightness);
        }
    }

    void triggerBeat() {
        if (!powerState || currentEffect != MUSIC) return;
        
        musicColor = pixels->ColorHSV(random(0, 65535), 255, 255);
        musicBrightness = 255; 
        pixels->setBrightness(musicBrightness);
        for(int i=0; i<count; i++) pixels->setPixelColor(i, musicColor);
        pixels->show();
    }

    void loop() {
        if (!powerState || currentEffect == STATIC) return;

        if (millis() - lastUpdate >= effectSpeed) {
            lastUpdate = millis();
            
            if (useRainbowColor) currentHue += 256;

            switch (currentEffect) {
                case RAINBOW: runRainbow(); break;
                case FADE:    runFade();    break;
                case CHASE:   runChase();   break;
                case MUSIC:   runMusicDecay(); break;
                default:      break;
            }
        }
    }

private:
    void runMusicDecay() {
        bool changed = false;
        if (musicBrightness > baseBrightness) {
            musicBrightness -= 15; 
            if (musicBrightness < baseBrightness) musicBrightness = baseBrightness;
            changed = true;
        } 
        else if (musicBrightness < baseBrightness) {
            musicBrightness = baseBrightness;
            changed = true;
        }
        if (changed) {
            pixels->setBrightness(musicBrightness);
            for(int i=0; i<count; i++) pixels->setPixelColor(i, musicColor);
            pixels->show();
        }
    }

    void runChase() {
        pixels->clear();
        uint32_t color = useRainbowColor ? pixels->ColorHSV(currentHue, 255, 255) : pixels->Color(r, g, b);
        pixels->setPixelColor(step, color);
        pixels->show();
        step++;
        if (step >= count) step = 0;
    }

    void runFade() {
        int actualBrightness = map(fadeVal, 0, 255, 10, globalBrightness);
        pixels->setBrightness(actualBrightness);
        uint32_t color = useRainbowColor ? pixels->ColorHSV(currentHue, 255, 255) : pixels->Color(r, g, b);
        for(int i=0; i<count; i++) pixels->setPixelColor(i, color);
        pixels->show();
        fadeVal += fadeDir * 5;
        if (fadeVal <= 0) { fadeVal = 0; fadeDir = 1; }
        else if (fadeVal >= 255) { fadeVal = 255; fadeDir = -1; }
    }

    void runRainbow() {
        pixels->setBrightness(globalBrightness);
        for(int i=0; i<count; i++) {
            int pixelHue = step + (i * 65536L / count);
            pixels->setPixelColor(i, pixels->gamma32(pixels->ColorHSV(pixelHue)));
        }
        pixels->show();
        step += 256;
        if (step >= 65536) step = 0;
    }
};

#endif