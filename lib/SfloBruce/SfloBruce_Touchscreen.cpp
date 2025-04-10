#include <SfloBruce_Touchscreen.h>
#include <SPI.h>

#define ISR_PREFIX IRAM_ATTR
#define MSEC_THRESHOLD 3
#define SPI_SETTING SPISettings(2000000, MSBFIRST, SPI_MODE0)

static SFLOBRUCE_Touch *isrPinptr;

void isrPin(void);

bool SFLOBRUCE_Touch::begin()
{
    pinMode(SFLOBRUCE_TOUCH_MOSI, OUTPUT);
    pinMode(SFLOBRUCE_TOUCH_MISO, INPUT);
    pinMode(SFLOBRUCE_TOUCH_CLK, OUTPUT);
    pinMode(SFLOBRUCE_TOUCH_CS, OUTPUT);
    digitalWrite(SFLOBRUCE_TOUCH_CLK, LOW);
    digitalWrite(SFLOBRUCE_TOUCH_CS, HIGH);
    pinMode(SFLOBRUCE_TOUCH_IRQ, INPUT);
    attachInterrupt(digitalPinToInterrupt(SFLOBRUCE_TOUCH_IRQ), isrPin, FALLING);
    isrPinptr = this;

    return true;
}

bool SFLOBRUCE_Touch::begin(SPIClass *tspi)
{
    _pspi = tspi;
    //_pspi->begin();
    pinMode(SFLOBRUCE_TOUCH_CS, OUTPUT);
    digitalWrite(SFLOBRUCE_TOUCH_CS, HIGH);
    pinMode(SFLOBRUCE_TOUCH_IRQ, INPUT);
    attachInterrupt(digitalPinToInterrupt(SFLOBRUCE_TOUCH_IRQ), isrPin, FALLING);
    isrPinptr = this;

    return true;
}

ISR_PREFIX
void isrPin(void)
{
    SFLOBRUCE_Touch *o = isrPinptr;
    o->isrWake = true;
}

uint8_t SFLOBRUCE_Touch::transfer(uint8_t val)
{
    if (_pspi == nullptr)
    {
        uint8_t out = 0;
        uint8_t del = _delay >> 1;
        uint8_t bval = 0;
        int sck = LOW;

        int8_t bit = 8;
        while (bit)
        {
            bit--;
            digitalWrite(SFLOBRUCE_TOUCH_MOSI, ((val & (1 << bit)) ? HIGH : LOW)); // Write bit
            wait(del);
            sck ^= 1u;
            digitalWrite(SFLOBRUCE_TOUCH_CLK, sck);
            /* ... Read bit */
            bval = digitalRead(SFLOBRUCE_TOUCH_MISO);
            out <<= 1;
            out |= bval;
            wait(del);
            sck ^= 1u;
            digitalWrite(SFLOBRUCE_TOUCH_CLK, sck);
        }

        return out;
    }
    else
    {
        uint8_t out = _pspi->transfer(val);

        return out;
    }
}

uint16_t SFLOBRUCE_Touch::transfer16(uint16_t data)
{
    union
    {
        uint16_t val;
        struct
        {
            uint8_t lsb;
            uint8_t msb;
        };
    } in, out;
    in.val = data;

    if (_pspi == nullptr)
    {
        out.msb = transfer(in.msb);
        out.lsb = transfer(in.lsb);
        return out.val;
    }
    else
    {
        out.msb = _pspi->transfer(in.msb);
        out.lsb = _pspi->transfer(in.lsb);
        return out.val;
    }
}

void SFLOBRUCE_Touch::wait(uint_fast8_t del)
{
    for (uint_fast8_t i = 0; i < del; i++)
    {
        asm volatile("nop");
    }
}

SFLOBRUCE_TS_Point SFLOBRUCE_Touch::getPointScaled()
{
    update();
    int16_t x = xraw, y = yraw;
    convertRawXY(&x, &y);

    return SFLOBRUCE_TS_Point(x, y, zraw);
}

SFLOBRUCE_TS_Point SFLOBRUCE_Touch::getPointRaw()
{
    update();
    return SFLOBRUCE_TS_Point(xraw, yraw, zraw);
}

bool SFLOBRUCE_Touch::touched()
{
    update();
    return ((zraw >= threshold) && isrWake);
}

void SFLOBRUCE_Touch::readData(uint16_t *x, uint16_t *y, uint8_t *z)
{
    update();
    *x = xraw;
    *y = yraw;
    *z = zraw;
}

static int16_t besttwoavg(int16_t x, int16_t y, int16_t z)
{
    int16_t da, db, dc;
    int16_t reta = 0;
    if (x > y)
        da = x - y;
    else
        da = y - x;
    if (x > z)
        db = x - z;
    else
        db = z - x;
    if (z > y)
        dc = z - y;
    else
        dc = y - z;

    if (da <= db && da <= dc)
        reta = (x + y) >> 1;
    else if (db <= da && db <= dc)
        reta = (x + z) >> 1;
    else
        reta = (y + z) >> 1;

    return (reta);
}

void SFLOBRUCE_Touch::update()
{
    int16_t data[6];
    int z;
    if (!isrWake)
        return;
    uint32_t now = millis();
    if (now - msraw < MSEC_THRESHOLD)
        return;

    digitalWrite(SFLOBRUCE_TOUCH_CS, LOW);

    if (_pspi != nullptr)
        _pspi->beginTransaction(SPI_SETTING);

    transfer(0xB1 /* Z1 */);
    int16_t z1 = transfer16(0xC1 /* Z2 */) >> 3;
    z = z1 + 4095;
    int16_t z2 = transfer16(0x91 /* X */) >> 3;
    z -= z2;
    if (z >= threshold)
    {
        transfer16(0x91 /* X */); // dummy X measure, 1st is always noisy
        data[0] = transfer16(0xD1 /* Y */) >> 3;
        data[1] = transfer16(0x91 /* X */) >> 3; // make 3 x-y measurements
        data[2] = transfer16(0xD1 /* Y */) >> 3;
        data[3] = transfer16(0x91 /* X */) >> 3;
    }
    else
        data[0] = data[1] = data[2] = data[3] = 0;
    data[4] = transfer16(0xD0 /* Y */) >> 3;
    data[5] = transfer16(0) >> 3;

    if (_pspi != nullptr)
        _pspi->endTransaction();

    digitalWrite(SFLOBRUCE_TOUCH_CS, HIGH);

    if (z < 0)
        z = 0;
    if (z < threshold)
    {
        zraw = 0;
        if (z < SFLOBRUCE_TOUCH_Z_THRESHOLD_INT)
        {
            isrWake = false;
        }
        return;
    }
    zraw = z;

    int16_t x = besttwoavg(data[0], data[2], data[4]);
    int16_t y = besttwoavg(data[1], data[3], data[5]);

    if (z >= threshold)
    {
        msraw = now; // good read completed, set wait
        xraw = x;
        yraw = y;
        // log_i("xraw= %d, yraw= %d", xraw, yraw);
    }
}

void SFLOBRUCE_Touch::convertRawXY(int16_t *x, int16_t *y)
{
    int16_t x_tmp = *x, y_tmp = *y, xx, yy;

    switch (rotation)
    {
    case 0: // PORT0
        xx = ((y_tmp - SFLOBRUCE_TOUCH_YMIN) * sizeY_px) / (SFLOBRUCE_TOUCH_YMAX - SFLOBRUCE_TOUCH_YMIN);
        yy = ((x_tmp - SFLOBRUCE_TOUCH_XMIN) * sizeX_px) / (SFLOBRUCE_TOUCH_XMAX - SFLOBRUCE_TOUCH_XMIN);
        xx = sizeY_px - xx;
        break;
    case 1: // LANDSC0
        xx = ((x_tmp - SFLOBRUCE_TOUCH_XMIN) * sizeX_px) / (SFLOBRUCE_TOUCH_XMAX - SFLOBRUCE_TOUCH_XMIN);
        yy = ((y_tmp - SFLOBRUCE_TOUCH_YMIN) * sizeY_px) / (SFLOBRUCE_TOUCH_YMAX - SFLOBRUCE_TOUCH_YMIN);
        break;
    case 2: // PORT1
        xx = ((y_tmp - SFLOBRUCE_TOUCH_YMIN) * sizeY_px) / (SFLOBRUCE_TOUCH_YMAX - SFLOBRUCE_TOUCH_YMIN);
        yy = ((x_tmp - SFLOBRUCE_TOUCH_XMIN) * sizeX_px) / (SFLOBRUCE_TOUCH_XMAX - SFLOBRUCE_TOUCH_XMIN);
        yy = sizeX_px - yy;
        break;
    default: // 3 LANDSC1
        xx = ((x_tmp - SFLOBRUCE_TOUCH_XMIN) * sizeX_px) / (SFLOBRUCE_TOUCH_XMAX - SFLOBRUCE_TOUCH_XMIN);
        yy = ((y_tmp - SFLOBRUCE_TOUCH_YMIN) * sizeY_px) / (SFLOBRUCE_TOUCH_YMAX - SFLOBRUCE_TOUCH_YMIN);
        xx = sizeX_px - xx;
        yy = sizeY_px - yy;
        break;
    }
    *x = xx;
    *y = yy;
}