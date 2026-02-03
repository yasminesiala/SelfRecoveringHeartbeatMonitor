#include <Arduino.h>
#include "EventLog.hpp"
#include "HeartbeatSensor.hpp"

class FaultManager {
public:
  enum class State : uint8_t {
    NORMAL = 0,
    RECOVERING,
    SAFE_MODE
  };

  enum class Action : uint8_t {
    NONE = 0,
    FORCE_RESET
  };

  struct Inputs {
    uint32_t nowMs;
    uint32_t lastBeatMs;
    uint32_t lastSignalChangeMs;
    int rawMin;
    int rawMax;
    int lastRaw;
    uint16_t bpmSmooth;
  };

  void begin();
  Action update(const Inputs &in, EventLog &log, HeartbeatSensor &sensor);

  void noteTimingOverrun() { _timingOverruns++; }
  State state() const { return _state; }
  uint8_t recoveryAttempts() const { return _recoveryAttempts; }

private:
  void requestRecovery(EventLog &log, int16_t code, int16_t detail);
  void performRecovery(EventLog &log, HeartbeatSensor &sensor);

private:
  State _state = State::NORMAL;
  uint8_t _recoveryAttempts = 0;
  uint8_t _timingOverruns = 0;

  static const uint16_t NO_BEAT_TIMEOUT_MS = 4000;
  static const uint16_t SENSOR_STUCK_MS    = 1500;

  static const uint8_t MAX_RECOVERY_BEFORE_RESET = 6;
  static const uint8_t MAX_SAFE_BEFORE_RESET = 8;
};
