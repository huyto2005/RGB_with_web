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
    
    // --- BIẾN CHO MUSIC MODE ---
    int musicBrightness = 0;
    uint32_t musicColor = 0; 
    const int baseBrightness = 60; // Độ sáng nền (Sáng nhẹ khi không có nhạc)

public:
    int r = 0, g = 0, b = 0;
    int globalBrightness = 255;
    EffectState currentEffect = STATIC;
    bool powerState = true; 

    LedRGB(int pin, int count = 8) : pin(pin), count(count) {
        pixels = new Adafruit_NeoPixel(count, pin, NEO_GRB + NEO_KHZ800);
    }

    void begin() {
        pixels->begin();
        pixels->setBrightness(globalBrightness);
        pixels->show();
    }

    void setPower(bool state) {
        powerState = state;
        if (powerState) {
            pixels->setBrightness(globalBrightness);
            if (currentEffect == STATIC) {
                uint32_t c = useRainbowColor ? pixels->ColorHSV(currentHue) : pixels->Color(r, g, b);
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
        
        if (currentEffect != MUSIC) {
            pixels->setBrightness(globalBrightness);
            if (currentEffect == STATIC) {
                uint32_t c = useRainbowColor ? pixels->ColorHSV(currentHue) : pixels->Color(r, g, b);
                for(int i=0; i<count; i++) pixels->setPixelColor(i, c);
                pixels->show();
            }
        }
    }

    void updateColorParams(int r, int g, int b) {
        this->r = r; this->g = g; this->b = b;
        useRainbowColor = false;
    }

    void setColor(int r, int g, int b) {
        if (!powerState) return;
        currentEffect = STATIC;
        useRainbowColor = false;
        this->r = r; this->g = g; this->b = b;
        pixels->setBrightness(globalBrightness);
        for(int i=0; i<count; i++) pixels->setPixelColor(i, pixels->Color(r, g, b));
        pixels->show();
    }

    void setEffect(String effectName, int speedMs, bool isRainbow = false) {
        if (!powerState) return;
        
        step = 0;
        effectSpeed = speedMs;
        lastUpdate = millis();
        useRainbowColor = isRainbow;

        currentEffect = STATIC;

        if (effectName == "rainbow") currentEffect = RAINBOW;
        else if (effectName == "fade") { currentEffect = FADE; fadeVal = 0; }
        else if (effectName == "chase") currentEffect = CHASE;
        else if (effectName == "music") { 
            currentEffect = MUSIC; 
            // Khởi tạo: Sáng nền + Màu ngẫu nhiên
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

    // ⭐ SỬA LỖI: LUÔN RANDOM MÀU KHI CÓ BEAT
    void triggerBeat() {
        if (!powerState || currentEffect != MUSIC) return;
        
        // 1. Tạo màu mới (Bắt buộc Random)
        musicColor = pixels->ColorHSV(random(0, 65535), 255, 255);

        // 2. Bùng sáng Max (255)
        musicBrightness = 255; 
        pixels->setBrightness(musicBrightness);
        
        // 3. Hiển thị ngay lập tức
        for(int i=0; i<count; i++) pixels->setPixelColor(i, musicColor);
        pixels->show();
    }

    // ⭐ HÀM LOOP ĐỂ CHẠY HIỆU ỨNG (Cần được gọi trong main.cpp)
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
    // Logic làm dịu đèn (Pulse) sau khi Beat đánh
    void runMusicDecay() {
        bool changed = false;
        
        if (musicBrightness > baseBrightness) {
            musicBrightness -= 15; // Giảm độ sáng
            if (musicBrightness < baseBrightness) musicBrightness = baseBrightness;
            changed = true;
        } 
        else if (musicBrightness < baseBrightness) {
            musicBrightness = baseBrightness;
            changed = true;
        }

        if (changed) {
            pixels->setBrightness(musicBrightness);
            // Vẽ lại đúng màu đang lưu (musicColor)
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

