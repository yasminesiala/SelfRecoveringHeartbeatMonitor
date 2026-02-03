#include <Arduino.h>
#include "EventLog.hpp"

void EventLog::log(uint8_t type, int16_t a, int16_t b) {
  _buf[_head] = { millis(), type, a, b };
  _head++;
  if (_head >= CAP) { _head = 0; _wrapped = true; }
}

void EventLog::printReport(Stream &out) const {
  out.println("---- EVENT LOG REPORT (most recent last) ----");
  const uint16_t start = _wrapped ? _head : 0;
  const uint16_t count = _wrapped ? CAP : _head;

  for (uint16_t i = 0; i < count; i++) {
    const uint16_t idx = (start + i) % CAP;
    const Event &e = _buf[idx];

    out.print(e.t_ms); out.print(" ms  ");
    out.print("T="); out.print(e.type);
    out.print("  A="); out.print(e.a);
    out.print("  B="); out.println(e.b);
  }
  out.println("--------------------------------------------");
}
