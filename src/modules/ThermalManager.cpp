#include "configuration.h"

#if MESHTASTIC_EV_MONITOR_GARAGE

#include "ThermalManager.h"
#include "sleep.h"
#include <Arduino.h>

ThermalManager *thermalManager;

// Start at the normal rate; runOnce() will adjust based on first reading.
uint32_t ThermalManager::currentPollIntervalMs = ThermalManager::kNormalPollMs;

int32_t ThermalManager::runOnce()
{
    // temperatureRead() is the Arduino-ESP32 wrapper around the SoC's
    // internal sensor. Accuracy is loose (designed for relative monitoring),
    // which is fine for throttle/sleep gating.
    float tempC = temperatureRead();

    if (tempC >= kCriticalTempC) {
        LOG_WARN("Thermal: %.1f°C >= critical %.1f°C - deep sleeping %u ms",
                 tempC, kCriticalTempC, (unsigned)kCooldownMs);
        doDeepSleep(kCooldownMs, false, true);
        return kCheckIntervalMs;  // unreachable, but appeases the scheduler
    }

    uint32_t prev = currentPollIntervalMs;
    if (tempC >= kThrottleTempC) {
        currentPollIntervalMs = kSlowPollMs;
    } else {
        currentPollIntervalMs = kNormalPollMs;
    }

    if (currentPollIntervalMs != prev) {
        LOG_INFO("Thermal: %.1f°C - poll interval -> %u ms",
                 tempC, (unsigned)currentPollIntervalMs);
    } else {
        LOG_DEBUG("Thermal: %.1f°C (interval %u ms)",
                  tempC, (unsigned)currentPollIntervalMs);
    }

    return kCheckIntervalMs;
}

#endif // MESHTASTIC_EV_MONITOR_GARAGE
