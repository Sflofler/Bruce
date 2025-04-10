#pragma once

#include <Arduino.h>
#include <SPI.h>

#define SFLOBRUCE_TOUCH_Z_THRESHOLD 500
#define SFLOBRUCE_TOUCH_Z_THRESHOLD_INT 100

#define SFLOBRUCE_TOUCH_IRQ     TOUCH_IRQ
#define SFLOBRUCE_TOUCH_MOSI    18
#define SFLOBRUCE_TOUCH_MISO    8
#define SFLOBRUCE_TOUCH_CLK     9
#define SFLOBRUCE_TOUCH_CS      TOUCH_CS

#define SFLOBRUCE_TOUCH_XMIN 185
#define SFLOBRUCE_TOUCH_XMAX 3700
#define SFLOBRUCE_TOUCH_YMIN 280
#define SFLOBRUCE_TOUCH_YMAX 3850

class SFLOBRUCE_TS_Point
{
public:
    SFLOBRUCE_TS_Point(void) : x(0), y(0), z(0) { }
    SFLOBRUCE_TS_Point(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) { }
    bool operator==(SFLOBRUCE_TS_Point p) { return ((p.x == x) && (p.y == y) && (p.z == z)); }
    bool operator!=(SFLOBRUCE_TS_Point p) { return ((p.x != x) || (p.y != y) || (p.z != z)); }

    int16_t x, y, z;
};

class SFLOBRUCE_Touch
{
public:
    constexpr SFLOBRUCE_Touch(int32_t w, int32_t h) : _delay(2), sizeX_px(w), sizeY_px(h) { }
    bool begin();
    bool begin(SPIClass *tspi);

    SFLOBRUCE_TS_Point getPointScaled();
    SFLOBRUCE_TS_Point getPointRaw();
    bool touched();
    void readData(uint16_t *x, uint16_t *y, uint8_t *z);
    void setRotation(uint8_t n) { rotation = n % 4; }
    void setThreshold(uint16_t th) { threshold = th;}
  
    volatile bool isrWake=true;

private:
    void update();
    uint8_t transfer(uint8_t);
    uint16_t transfer16(uint16_t data);
    void wait(uint_fast8_t del);
    void convertRawXY(int16_t *x, int16_t *y);
    uint8_t rotation=1;
    int16_t xraw=0, yraw=0, zraw=0;
    uint16_t threshold = SFLOBRUCE_TOUCH_Z_THRESHOLD;
    uint32_t msraw=0x80000000;
    uint8_t _delay;
    const  int32_t sizeX_px;
    const int32_t sizeY_px;
    SPIClass *_pspi = nullptr;
};