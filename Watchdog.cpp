#include <Arduino.h>
#include "Watchdog.hpp"
#include <avr/wdt.h>
#include <avr/interrupt.h>

static uint8_t g_mcusr_mirror __attribute__((section(".noinit")));
static bool g_captured = false;

void Watchdog::captureResetCauseEarly() {
  if (g_captured) return;
  g_captured = true;
  g_mcusr_mirror = MCUSR;
  MCUSR = 0;
}

uint8_t Watchdog::resetCauseBits() {
  captureResetCauseEarly();
  return g_mcusr_mirror;
}

void Watchdog::printResetCause(Stream &out) {
  captureResetCauseEarly();

  out.print("Reset cause: ");
  bool any = false;
  if (g_mcusr_mirror & _BV(WDRF)) { out.print("WDT "); any = true; }
  if (g_mcusr_mirror & _BV(BORF)) { out.print("BROWNOUT "); any = true; }
  if (g_mcusr_mirror & _BV(EXTRF)){ out.print("EXTERNAL "); any = true; }
  if (g_mcusr_mirror & _BV(PORF)) { out.print("POWERON "); any = true; }
  if (!any) out.print("UNKNOWN ");
  out.println();
}

static uint8_t toWdtEnum(Watchdog::Timeout t) {
  switch (t) {
    case Watchdog::Timeout::WDTO_15MS: return WDTO_15MS;
    case Watchdog::Timeout::WDTO_30MS: return WDTO_30MS;
    case Watchdog::Timeout::WDTO_60MS: return WDTO_60MS;
    case Watchdog::Timeout::WDTO_120MS: return WDTO_120MS;
    case Watchdog::Timeout::WDTO_250MS: return WDTO_250MS;
    case Watchdog::Timeout::WDTO_500MS: return WDTO_500MS;
    case Watchdog::Timeout::WDTO_1S: return WDTO_1S;
    case Watchdog::Timeout::WDTO_2S: return WDTO_2S;
    case Watchdog::Timeout::WDTO_4S: return WDTO_4S;
    case Watchdog::Timeout::WDTO_8S: return WDTO_8S;
    default: return WDTO_2S;
  }
}

void Watchdog::begin(Timeout t) {
  captureResetCauseEarly();
  wdt_enable(toWdtEnum(t));
}

void Watchdog::kick() {
  wdt_reset();
}

void Watchdog::forceReset() {
  wdt_enable(WDTO_15MS);
  while (1) { }
}
