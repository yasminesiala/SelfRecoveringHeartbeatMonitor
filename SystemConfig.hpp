#pragma once
#include <Arduino.h>

static const uint16_t SAMPLE_HZ = 200;
static const uint32_t SAMPLE_PERIOD_US = 1000000UL / SAMPLE_HZ;

static const bool FAULT_INJECTION = true;

static const uint16_t NO_BEAT_TIMEOUT_MS = 4000;
static const uint16_t SENSOR_STUCK_MS    = 1500;
