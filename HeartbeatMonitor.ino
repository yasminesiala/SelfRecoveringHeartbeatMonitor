#include <avr/wdt.h>
#include <Wire.h>
#include "MAX30105.h" 
#include "RingBuffer.h"
#include "Filters.h"
#include "Logger.h"

// Globals
MAX30105 sensor;
RingBuffer rb;
DCFilter filter;
enum State { BOOT, IDLE, TRACKING, FAULT };
State systemState = BOOT;

void setup() {
    // 1. Check Reset Reason
    uint8_t reason = MCUSR;
    MCUSR = 0;
    wdt_disable(); 
    
    Serial.begin(115200);
    commitLog(0, reason); // Log the boot/reset reason

    // 2. Hardware Init
    if (!sensor.begin(Wire, I2C_SPEED_FAST)) systemState = FAULT;
    sensor.setup(); // Default 100Hz, 411us pulse width

    // 3. Timer1 Setup (100Hz Sampling)
    cli();
    TCCR1A = 0; TCCR1B = 0; TCNT1 = 0;
    OCR1A = 624;            // (16MHz / (256 * 100Hz)) - 1
    TCCR1B |= (1 << WGM12); // CTC Mode
    TCCR1B |= (1 << CS12);  // 256 Prescaler
    TIMSK1 |= (1 << OCIE1A); // Enable Interrupt
    sei();

    wdt_enable(WDTO_2S); // Enable Watchdog
    systemState = IDLE;
}

// ISR: High Priority. Just gets the data and leaves.
ISR(TIMER1_COMPA_vect) {
    uint32_t ir = sensor.getIR();
    rb.push(ir);
}

void loop() {
    wdt_reset(); // "Pet" the watchdog

    // Main FSM
    if (rb.available()) {
        int16_t cleanSignal = filter.process(rb.pop());
        processPulse(cleanSignal);
    }
}

void processPulse(int16_t val) {
    static int16_t lastVal = 0;
    // Simple Peak Detection Logic
    if (val > 500 && lastVal <= 500) { // Threshold crossed
        commitLog(1, 72); // Placeholder for calculated BPM
        Serial.println(F("Peak Detected"));
    }
    lastVal = val;
}