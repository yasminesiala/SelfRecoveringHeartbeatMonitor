#include "Storage.hpp"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

void Storage::begin(bool enableSd, uint8_t csPin) {
  _enabled = enableSd;
  _csPin = csPin;
  if (!_enabled) return;
  SD.begin(_csPin);
}

void Storage::writeEventLog(const EventLog &log) {
  if (!_enabled) return;

  File f = SD.open("event_log.txt", FILE_WRITE);
  if (!f) return;

  f.println("---- EVENT LOG REPORT ----");

  const uint16_t cap = log.capacity();
  const bool wrapped = log.wrapped();
  const uint16_t head = log.head();
  const Event *buf = log.buffer();
  const uint16_t start = wrapped ? head : 0;
  const uint16_t count = wrapped ? cap : head;
  for (uint16_t i = 0; i < count; i++) {
    const uint16_t idx = (start + i) % cap;
    const Event &e = buf[idx];

    f.print(e.t_ms); f.print(" ms  ");
    f.print("T="); f.print(e.type);
    f.print("  A="); f.print(e.a);
    f.print("  B="); f.println(e.b);
  }
  f.println("--------------------------");
  f.close();
}
