#pragma once
#include "configuration.h"

#if MESHTASTIC_EV_MONITOR_GARAGE

#include "concurrency/OSThread.h"

/**
 * Reads the ESP32-S3 internal temperature sensor and adjusts the
 * effective poll interval used by VisionAIModule. Triggers deep sleep
 * if temperature exceeds a critical threshold.
 *
 * - Below kThrottleTempC : kNormalPollMs (5 s)
 * - Between kThrottleTempC and kCriticalTempC : kSlowPollMs (60 s)
 * - Above kCriticalTempC : doDeepSleep(kCooldownMs)
 *
 * VisionAIModule reads getPollIntervalMs() each tick, so throttling
 * affects inference cadence automatically.
 */
class ThermalManager : public concurrency::OSThread
{
  public:
    ThermalManager() : OSThread("Thermal") {}

    // Polled by VisionAIModule. Returns the current recommended poll interval.
    static uint32_t getPollIntervalMs() { return currentPollIntervalMs; }

  protected:
    int32_t runOnce() override;

  private:
    static constexpr float kThrottleTempC = 70.0f;
    static constexpr float kCriticalTempC = 85.0f;

    static constexpr uint32_t kNormalPollMs = 5000;     // 5 s
    static constexpr uint32_t kSlowPollMs = 60000;      // 60 s
    static constexpr uint32_t kCooldownMs = 5 * 60 * 1000;  // deep sleep duration on critical
    static constexpr uint32_t kCheckIntervalMs = 30000; // 30 s between checks

    static uint32_t currentPollIntervalMs;
};

extern ThermalManager *thermalManager;

#endif // MESHTASTIC_EV_MONITOR_GARAGE
