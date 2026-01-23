#ifndef FILTERS_H
#define FILTERS_H

class DCFilter {
private:
    int32_t w = 0; 
    const int16_t alpha = 31130; // 0.95 in 16-bit fixed point

public:
    // Removes the 50,000+ DC offset from MAX30102
    int16_t process(uint32_t raw) {
        int32_t prev_w = w;
        // Fixed-point multiplication: (alpha * w) / 32768
        w = (int32_t)raw + ((int32_t)((int16_t)alpha * w) >> 15);
        return (int16_t)(w - prev_w);
    }
};
#endif