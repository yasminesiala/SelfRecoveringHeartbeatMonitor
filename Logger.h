#include <EEPROM.h>

struct LogEvent {
    uint16_t timestamp_sec; // Seconds since boot
    uint8_t type;           // 0=Reset, 1=BPM, 2=Fault
    uint8_t value;          // The BPM or Error Code
};

void commitLog(uint8_t type, uint8_t val) {
    static int eepromAddr = 0;
    LogEvent ev = {(uint16_t)(millis()/1000), type, val};
    
    // Append-only logic
    if(eepromAddr < (EEPROM.length() - sizeof(LogEvent))) {
        EEPROM.put(eepromAddr, ev);
        eepromAddr += sizeof(LogEvent);
    }
}