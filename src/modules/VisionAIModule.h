#pragma once
#include "SinglePortModule.h"

/**
 * Polls the Grove Vision AI Module V2 over I2C for object detections,
 * maps detection bounding boxes to 3 fixed ROIs (one per EV charging
 * spot), and on a per-spot state transition emits a mesh text message
 * ("Spot N vacant" / "Spot N occupied").
 *
 * Status: skeleton. I2C read via Seeed_Arduino_SSCMA, ROI overlap
 * test, and mesh send are stubbed and need implementation before this
 * module is registered in Modules.cpp.
 */
class VisionAIModule : public SinglePortModule, private concurrency::OSThread
{
  public:
    VisionAIModule()
        : SinglePortModule("visionai", meshtastic_PortNum_TEXT_MESSAGE_APP), OSThread("VisionAI")
    {
    }

  protected:
    virtual int32_t runOnce() override;

  private:
    static constexpr uint8_t kNumSpots = 3;

    struct ROI {
        uint16_t x, y, w, h;
    };

    bool occupied[kNumSpots] = {false, false, false};
    ROI rois[kNumSpots] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    bool pollVisionAI();
    void sendSpotMessage(uint8_t spot, bool nowOccupied);
};

extern VisionAIModule *visionAIModule;
