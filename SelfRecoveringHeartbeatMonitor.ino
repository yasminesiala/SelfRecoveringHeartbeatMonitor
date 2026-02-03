#include <Arduino.h>
#include "EventLog.hpp"
#include "Watchdog.hpp"
#include "HeartbeatSensor.hpp"
#include "FaultManager.hpp"
#include "Storage.hpp"
#include "Pins.hpp"
#include "SystemConfig.hpp"

EventLog gLog;
HeartbeatSensor gSensor(PIN_SENSOR);
FaultManager gFaults;
Storage gStorage;

static uint32_t nextSampleUs = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(PIN_LED, OUTPUT);
  if (PIN_BUZZER != 255) pinMode(PIN_BUZZER, OUTPUT);

  Watchdog::begin(Watchdog::Timeout::WDTO_2S);

  Watchdog::captureResetCauseEarly(); 
  Watchdog::printResetCause(Serial);

  gLog.log(EVT_BOOT, (int16_t)Watchdog::resetCauseBits(), 0);

  gStorage.begin(USE_SD_LOG, PIN_SD_CS);
  // gStorage.writeEventLog(gLog);

  gSensor.begin();
  gFaults.begin();

  nextSampleUs = micros();
  Serial.println("SelfRecoveringHeartbeatMonitor booted. Sampling started.");
}

static void ledPulse(uint16_t ms = 20) {
  digitalWrite(PIN_LED, HIGH);
  delay(ms);
  digitalWrite(PIN_LED, LOW);
}

static void buzzerBeep(uint16_t ms = 15) {
  if (PIN_BUZZER == 255) return;
  digitalWrite(PIN_BUZZER, HIGH);
  delay(ms);
  digitalWrite(PIN_BUZZER, LOW);
}

void loop() {
  Watchdog::kick();

  const uint32_t nowUs = micros();
  if ((int32_t)(nowUs - nextSampleUs) < 0) return;

  nextSampleUs += SAMPLE_PERIOD_US;

  //timing overrun detection 
  const int32_t latenessUs = (int32_t)(nowUs - nextSampleUs);
  if (latenessUs > (int32_t)SAMPLE_PERIOD_US) {
    gLog.log(EVT_FAULT_DETECTED, 1005, (int16_t)(latenessUs / 1000)); // ms late
    gFaults.noteTimingOverrun();
  }

  //sample & update sensor processing
  gSensor.update(FAULT_INJECTION);

  if (gSensor.consumeBeat()) {
    gLog.log(EVT_BEAT, (int16_t)gSensor.getBpm(), (int16_t)gSensor.getBpmSmooth());
    if (gFaults.state() == FaultManager::State::NORMAL) {
      ledPulse();
      buzzerBeep();
    }
  }

  //1Hz stats output & log
  static uint16_t sampleCounter = 0;
  sampleCounter++;
  if (sampleCounter >= SAMPLE_HZ) {
    sampleCounter = 0;

    gLog.log(EVT_SAMPLE_STATS,
             (int16_t)gSensor.getBpmSmooth(),
             (int16_t)gSensor.getSignalRange());

    Serial.print("BPM="); Serial.print(gSensor.getBpmSmooth());
    Serial.print("  thresh="); Serial.print((int)gSensor.getThreshold());
    Serial.print("  range="); Serial.print(gSensor.getSignalRange());
    Serial.print("  state="); Serial.println((int)gFaults.state());
  }

  //fault recovery
  FaultManager::Inputs in;
  in.nowMs = millis();
  in.lastBeatMs = gSensor.lastBeatMs();
  in.lastSignalChangeMs = gSensor.lastSignalChangeMs();
  in.rawMin = gSensor.rawMin();
  in.rawMax = gSensor.rawMax();
  in.lastRaw = gSensor.lastRaw();
  in.bpmSmooth = gSensor.getBpmSmooth();

  const FaultManager::Action act = gFaults.update(in, gLog, gSensor);

  if (act == FaultManager::Action::FORCE_RESET) {
    gLog.log(EVT_FORCED_RESET, (int16_t)gFaults.recoveryAttempts(), 0);
    gLog.printReport(Serial);
    gStorage.writeEventLog(gLog);
    Watchdog::forceReset();
  }
}
