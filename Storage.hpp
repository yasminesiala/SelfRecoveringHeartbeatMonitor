#include <Arduino.h>
#pragma once
#include "EventLog.hpp"

class Storage {
public:
  void begin(bool enableSd, uint8_t csPin);
  void writeEventLog(const EventLog &log);

private:
  bool _enabled = false;
  uint8_t _csPin = 10;
};
