#ifndef LEDRGB_H
#define LEDRGB_H

#include <Adafruit_NeoPixel.h>

enum EffectState { STATIC, RAINBOW, FADE, CHASE, MUSIC };

class LedRGB {
private:
    int pin;
    int count;
    Adafruit_NeoPixel* pixels;
    
    // Thay Ticker bằng biến đếm thời gian (An toàn tuyệt đối)
    unsigned long lastUpdate = 0;
    int effectSpeed = 0; // Tốc độ hiệu ứng
    
    long step = 0;
    int fadeDir = 1;
    int fadeVal = 0;
    
    bool useRainbowColor = false;
    uint16_t currentHue = 0;
    int musicBrightness = 0;

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
            // Vẽ lại trạng thái hiện tại ngay khi bật
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
        
        // Nếu không phải Music (Music tự quản lý độ sáng), thì update ngay
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

    // Hàm set màu tĩnh (Sẽ dừng mọi hiệu ứng)
    void setColor(int r, int g, int b) {
        if (!powerState) return;
        
        currentEffect = STATIC; // Chuyển về tĩnh
        useRainbowColor = false;
        
        this->r = r; this->g = g; this->b = b;
        pixels->setBrightness(globalBrightness);
        for(int i=0; i<count; i++) pixels->setPixelColor(i, pixels->Color(r, g, b));
        pixels->show();
    }

    // ⭐ HÀM QUAN TRỌNG: Thiết lập hiệu ứng (Reset trạng thái sạch sẽ)
    void setEffect(String effectName, int speedMs, bool isRainbow = false) {
        if (!powerState) return;
        
        // 1. Reset các biến đếm
        step = 0;
        effectSpeed = speedMs;
        lastUpdate = millis(); // Đặt lại đồng hồ
        useRainbowColor = isRainbow;

        // 2. Chuyển chế độ
        if (effectName == "rainbow") currentEffect = RAINBOW;
        else if (effectName == "fade") { currentEffect = FADE; fadeVal = 0; }
        else if (effectName == "chase") currentEffect = CHASE;
        else if (effectName == "music") { 
            currentEffect = MUSIC; 
            musicBrightness = 0; 
            pixels->setBrightness(0); // Music bắt đầu từ tối
            pixels->show();
        }
        else currentEffect = STATIC;

        // 3. Khôi phục độ sáng nếu không phải Music
        if (currentEffect != MUSIC) {
            pixels->setBrightness(globalBrightness);
        }
    }

    void triggerBeat() {
        if (!powerState || currentEffect != MUSIC) return;
        musicBrightness = 255; 
        pixels->setBrightness(musicBrightness);
        uint32_t c = pixels->Color(r, g, b);
        for(int i=0; i<count; i++) pixels->setPixelColor(i, c);
        pixels->show();
    }

    // ⭐ HÀM LOOP MỚI: Gọi trong main loop
    void loop() {
        if (!powerState || currentEffect == STATIC) return;

        // Kiểm tra thời gian (Thay thế Ticker)
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
        if (musicBrightness > 0) {
            musicBrightness -= 15; 
            if (musicBrightness < 0) musicBrightness = 0;
            pixels->setBrightness(musicBrightness);
            uint32_t c = pixels->Color(r, g, b);
            for(int i=0; i<count; i++) pixels->setPixelColor(i, c);
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
        int actualBrightness = map(fadeVal, 0, 255, 10, globalBrightness); // Min 10 để không tắt hẳn
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


