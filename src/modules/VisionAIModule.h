#pragma once
#include "configuration.h"

#if MESHTASTIC_EV_MONITOR_GARAGE

#include "SinglePortModule.h"

/**
 * Polls the Grove Vision AI V2 (Himax + OV5647 camera) over I2C, maps
 * detection bounding boxes to 3 fixed ROIs (one per EV spot), and on a
 * per-spot occupancy state change emits a TEXT_MESSAGE_APP mesh packet
 * ("Spot N vacant" / "Spot N occupied").
 *
 * State changes are debounced: a spot only flips after N consecutive
 * consistent reads, to suppress flicker from inference noise.
 */
class VisionAIModule : public SinglePortModule, private concurrency::OSThread
{
  public:
    VisionAIModule()
        : SinglePortModule("visionai", meshtastic_PortNum_TEXT_MESSAGE_APP), OSThread("VisionAI")
    {
    }

  protected:
    int32_t runOnce() override;

  private:
    static constexpr uint8_t kNumSpots = 3;
    static constexpr uint8_t kHysteresisN = 2;   // consecutive reads required to flip
    static constexpr uint8_t kMinScore = 50;     // SSCMA score 0-100, ignore lower
    static constexpr uint32_t kDefaultPollMs = 5000;

    struct ROI {
        uint16_t x, y, w, h;     // pixel-space, tune for installed camera
    };

    // Spots laid out left-to-right in the camera frame. Tune for your install
    // (likely after seeing the first SenseCraft preview from the real mount).
    ROI rois[kNumSpots] = {
        {  0, 0, 100, 240},   // Spot 1
        {110, 0, 100, 240},   // Spot 2
        {220, 0, 100, 240},   // Spot 3
    };

    bool occupied[kNumSpots] = {false, false, false};
    uint8_t pendingCount[kNumSpots] = {0, 0, 0};   // consecutive opposite-state reads
    bool firstTime = true;

    bool pollAndDetect(bool currentlyOccupied[kNumSpots]);
    void emitSpotMessage(uint8_t spotIdx, bool nowOccupied);
};

extern VisionAIModule *visionAIModule;

#endif // MESHTASTIC_EV_MONITOR_GARAGE
