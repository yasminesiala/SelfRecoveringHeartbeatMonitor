#pragma once

class Watchdog {
public:
  enum class Timeout : uint8_t {
    WDTO_15MS,
    WDTO_30MS,
    WDTO_60MS,
    WDTO_120MS,
    WDTO_250MS,
    WDTO_500MS,
    WDTO_1S,
    WDTO_2S,
    WDTO_4S,
    WDTO_8S
  };

  static void begin(Timeout t);
  static void kick();
  static void forceReset();

  //reset cause 
  static void captureResetCauseEarly();     
  static uint8_t resetCauseBits();
  static void printResetCause(Stream &out);
};
