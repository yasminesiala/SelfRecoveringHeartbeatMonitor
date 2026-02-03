#include <Arduino.h>
#include "SystemConfig.hpp"
#include "FaultManager.hpp"


void FaultManager::begin() {
  _state = State::NORMAL;
  _recoveryAttempts = 0;
  _timingOverruns = 0;
}

void FaultManager::requestRecovery(EventLog &log, int16_t code, int16_t detail) {
  log.log(EVT_FAULT_DETECTED, code, detail);
  _state = State::RECOVERING;
  _recoveryAttempts++;
}

void FaultManager::performRecovery(EventLog &log, HeartbeatSensor &sensor) {
  sensor.recalibrateFromStats();
  log.log(EVT_RECOVERY_ACTION, (int16_t)_recoveryAttempts, (int16_t)sensor.getThreshold());

  if (_recoveryAttempts >= 3) _state = State::SAFE_MODE;
  else _state = State::NORMAL;
}

FaultManager::Action FaultManager::update(const Inputs &in, EventLog &log, HeartbeatSensor &sensor) {
  // Fault code conventions:
  // 1001: no beat timeout
  // 1002: sensor stuck (no meaningful change)
  // 1003: disconnected low
  // 1004: disconnected high
  // 1005: timing overrun (logged in main loop)

  if (_state == State::NORMAL || _state == State::SAFE_MODE) {
    if ((in.nowMs - in.lastBeatMs) > NO_BEAT_TIMEOUT_MS) {
      requestRecovery(log, 1001, (int16_t)(in.nowMs - in.lastBeatMs));
    } else if ((in.nowMs - in.lastSignalChangeMs) > SENSOR_STUCK_MS) {
      requestRecovery(log, 1002, (int16_t)(in.nowMs - in.lastSignalChangeMs));
    } else if (in.rawMin < 5 && in.rawMax < 50) {
      requestRecovery(log, 1003, (int16_t)in.rawMax);
    } else if (in.rawMax > 1018 && in.rawMin > 980) {
      requestRecovery(log, 1004, (int16_t)in.rawMin);
    }
  }

  if (_state == State::RECOVERING) {
    performRecovery(log, sensor);

    if (_recoveryAttempts >= MAX_RECOVERY_BEFORE_RESET) {
      return Action::FORCE_RESET;
    }
  } else if (_state == State::SAFE_MODE) {
    if (_recoveryAttempts >= MAX_SAFE_BEFORE_RESET) {
      return Action::FORCE_RESET;
    }
  }

  return Action::NONE;
}
