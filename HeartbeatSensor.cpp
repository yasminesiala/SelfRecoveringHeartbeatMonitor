#include <Arduino.h>
#include "HeartbeatSensor.hpp"
#include "SystemConfig.hpp"

HeartbeatSensor::HeartbeatSensor(uint8_t analogPin) : _pin(analogPin) {}

void HeartbeatSensor::begin() {
  _lastBeatMs = millis();
  _lastSignalChangeMs = millis();
  _rawMin = 1023;
  _rawMax = 0;
  _lastRaw = -1;
  _beatFlag = false;
}

static uint32_t lcgNext(uint32_t &s) {
  s = 1664525UL * s + 1013904223UL;
  return s;
}

int HeartbeatSensor::readRaw(bool faultInjection) {
  int x = analogRead(_pin);

  if (!faultInjection) return x;

  //simulate faults 
  const uint32_t r = lcgNext(_lcg);

  //occasional spike
  if ((r % 5000) == 0) x = (r & 1) ? 1023 : 0;

  //occasional dropout
  if ((r % 3000) == 0) x = 0;

  //occasional stuck window
  static uint16_t stuckCount = 0;
  if ((r % 7000) == 0) stuckCount = 400; // ~2 seconds at 200Hz

  if (stuckCount > 0) {
    stuckCount--;
    x = 20; //stuck low
  }

  return x;
}

void HeartbeatSensor::updateAdaptiveThreshold(int raw) {
  _base = (1.0f - BASE_ALPHA) * _base + BASE_ALPHA * raw;

  const float dev = fabs(raw - _base);
  _amp = (1.0f - AMP_ALPHA) * _amp + AMP_ALPHA * dev;

  _thresh = _base + THRESH_FRACTION * _amp;
}

void HeartbeatSensor::updateStats(int raw) {
  if (raw < _rawMin) _rawMin = raw;
  if (raw > _rawMax) _rawMax = raw;

  if (_lastRaw >= 0 && abs(raw - _lastRaw) > 2) {
    _lastSignalChangeMs = millis();
  }
  _lastRaw = raw;
}

void HeartbeatSensor::beatDetect(int raw) {
  static bool wasAbove = false;
  const uint32_t now = millis();
  const bool above = (raw > (int)_thresh);

  //rising edge
  if (!wasAbove && above) {
    const uint32_t dt = now - _lastBeatMs;
    if (dt >= REFRACTORY_MS) {
      _lastBeatMs = now;

      const uint16_t instBpm = (dt > 0) ? (uint16_t)(60000UL / dt) : 0;

      if (instBpm >= MIN_BPM && instBpm <= MAX_BPM) {
        _bpm = instBpm;
        _bpmSmooth = (_bpmSmooth == 0) ? _bpm : (uint16_t)((_bpmSmooth * 7 + _bpm) / 8);
        _beatFlag = true;
      }
    }
  }

  wasAbove = above;
}

void HeartbeatSensor::update(bool faultInjection) {
  const int raw = readRaw(faultInjection);
  updateStats(raw);
  updateAdaptiveThreshold(raw);
  beatDetect(raw);
}

bool HeartbeatSensor::consumeBeat() {
  const bool hadBeat = _beatFlag;
  _beatFlag = false;
  return hadBeat;
}

void HeartbeatSensor::recalibrateFromStats() {
  _base = (_rawMin + _rawMax) * 0.5f;
  float newAmp = (_rawMax - _rawMin) * 0.5f;
  if (newAmp < 10) newAmp = 10;
  _amp = newAmp;
  _thresh = _base + THRESH_FRACTION * _amp;

  _lastBeatMs = millis();
  _lastSignalChangeMs = millis();

  _rawMin = 1023;
  _rawMax = 0;
}
