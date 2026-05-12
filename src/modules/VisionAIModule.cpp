#include "configuration.h"
#if !MESHTASTIC_EXCLUDE_VISIONAI

#include "VisionAIModule.h"

VisionAIModule *visionAIModule;

int32_t VisionAIModule::runOnce()
{
    // TODO: poll Vision AI V2 over I2C, populate detections, check overlap
    // with rois[], emit sendSpotMessage(i, ...) on per-spot state change.
    LOG_DEBUG("VisionAI tick (skeleton — no I2C yet)\n");
    return 5000;
}

bool VisionAIModule::pollVisionAI()
{
    // TODO: SSCMA::AI.invoke() + AI.boxes() via Wire; fill a detection buffer.
    return false;
}

void VisionAIModule::sendSpotMessage(uint8_t spot, bool nowOccupied)
{
    // TODO: build meshtastic_MeshPacket with portnum=TEXT_MESSAGE_APP,
    // payload="Spot <N> vacant|occupied", route via service->sendToMesh().
    (void)spot;
    (void)nowOccupied;
}

#endif
