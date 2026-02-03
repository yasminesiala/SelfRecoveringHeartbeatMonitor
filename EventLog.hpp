#include <Arduino.h>

enum EventType : uint8_t {
  EVT_BOOT = 1,
  EVT_SAMPLE_STATS,
  EVT_BEAT,
  EVT_FAULT_DETECTED,
  EVT_RECOVERY_ACTION,
  EVT_FORCED_RESET,
};

struct Event {
  uint32_t t_ms;
  uint8_t  type;
  int16_t  a;
  int16_t  b;
};

class EventLog {
public:
  void log(uint8_t type, int16_t a = 0, int16_t b = 0);
  void printReport(Stream &out) const;

  uint16_t capacity() const { return CAP; }
  bool wrapped() const { return _wrapped; }
  uint16_t head() const { return _head; }
  const Event* buffer() const { return _buf; }

private:
  static const uint16_t CAP = 256;
  Event _buf[CAP];
  uint16_t _head = 0;
  bool _wrapped = false;
};
