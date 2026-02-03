#pragma once
#include <Arduino.h>  

class DCFilter {
private:
    int32_t w = 0;
    static constexpr int16_t alpha = 31130; // ~0.95 in Q15 fixed-point (31130/32768)

public:
    //removes DC offset 
    int16_t process(uint32_t raw) {
        int32_t prev_w = w;
        //w = raw + alpha * w (fixed point)
        w = (int32_t)raw + (((int32_t)alpha * w) >> 15);
        //return raw - lowpass(raw) approx
        return (int16_t)(w - prev_w);
    }
};
