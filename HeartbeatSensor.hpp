#include <Arduino.h>

class HeartbeatSensor {
public:
  explicit HeartbeatSensor(uint8_t analogPin);
  void begin();
  void update(bool faultInjection);
  bool consumeBeat();

  //metrics 
  uint16_t getBpm() const { return _bpm; }
  uint16_t getBpmSmooth() const { return _bpmSmooth; }
  float getThreshold() const { return _thresh; }
  int getSignalRange() const { return (_rawMax - _rawMin); }

  uint32_t lastBeatMs() const { return _lastBeatMs; }
  uint32_t lastSignalChangeMs() const { return _lastSignalChangeMs; }
  int rawMin() const { return _rawMin; }
  int rawMax() const { return _rawMax; }
  int lastRaw() const { return _lastRaw; }

  void recalibrateFromStats();

private:
  int readRaw(bool faultInjection);
  void updateAdaptiveThreshold(int raw);
  void updateStats(int raw);
  void beatDetect(int raw);

private:
  uint8_t _pin;

  float _base = 512.0f;
  float _amp = 40.0f;
  float _thresh = 540.0f;

//beat tracking
  uint32_t _lastBeatMs = 0;
  uint32_t _lastSignalChangeMs = 0;
  int _lastRaw = -1;

  uint16_t _bpm = 0;
  uint16_t _bpmSmooth = 0;

  //stats window 
  int _rawMin = 1023;
  int _rawMax = 0;

  //event flag
  bool _beatFlag = false;

  // RNG for fault injection
  uint32_t _lcg = 0x12345678;

  static const uint16_t REFRACTORY_MS = 250;
  static const uint16_t MIN_BPM = 35;
  static const uint16_t MAX_BPM = 220;
  static constexpr float BASE_ALPHA = 0.01f;
  static constexpr float AMP_ALPHA  = 0.02f;
  static constexpr float THRESH_FRACTION = 0.55f;
};
