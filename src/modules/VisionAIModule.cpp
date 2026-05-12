#include "configuration.h"

#if MESHTASTIC_EV_MONITOR_GARAGE

#include "VisionAIModule.h"
#include "MeshService.h"
#include "ThermalManager.h"
#include <Seeed_Arduino_SSCMA.h>
#include <Wire.h>

VisionAIModule *visionAIModule;

static SSCMA sscma;

static bool rectsOverlap(uint16_t ax, uint16_t ay, uint16_t aw, uint16_t ah,
                         uint16_t bx, uint16_t by, uint16_t bw, uint16_t bh)
{
    return !(ax + aw <= bx || bx + bw <= ax || ay + ah <= by || by + bh <= ay);
}

// COCO classes considered "a vehicle in a spot" - tune if you train a custom model.
static bool isVehicleClass(uint8_t target)
{
    return target == 2 /*car*/ || target == 5 /*bus*/ || target == 7 /*truck*/;
}

int32_t VisionAIModule::runOnce()
{
    if (firstTime) {
        firstTime = false;
        Wire.begin();                 // I2C_SDA=5, I2C_SCL=6 from variant.h
        sscma.begin();
        LOG_INFO("VisionAI: SSCMA initialized, polling every %u ms",
                 (unsigned)ThermalManager::getPollIntervalMs());
        return ThermalManager::getPollIntervalMs();
    }

    bool currentlyOccupied[kNumSpots] = {false, false, false};
    if (!pollAndDetect(currentlyOccupied)) {
        // I2C read failed - try again next tick, don't change state.
        return ThermalManager::getPollIntervalMs();
    }

    for (uint8_t i = 0; i < kNumSpots; i++) {
        if (currentlyOccupied[i] == occupied[i]) {
            pendingCount[i] = 0;     // matches current state, reset hysteresis
            continue;
        }
        pendingCount[i]++;
        if (pendingCount[i] >= kHysteresisN) {
            occupied[i] = currentlyOccupied[i];
            pendingCount[i] = 0;
            emitSpotMessage(i, occupied[i]);
        }
    }

    return ThermalManager::getPollIntervalMs();
}

bool VisionAIModule::pollAndDetect(bool currentlyOccupied[kNumSpots])
{
    // SSCMA::invoke() runs inference on the latest captured frame.
    // Returns 0 on success in current library; older versions returned bool.
    if (sscma.invoke() != 0) {
        LOG_DEBUG("VisionAI: invoke failed");
        return false;
    }

    // For each detection, mark any ROI it overlaps as occupied.
    for (const auto &b : sscma.boxes()) {
        if (b.score < kMinScore) continue;
        if (!isVehicleClass(b.target)) continue;

        for (uint8_t i = 0; i < kNumSpots; i++) {
            if (rectsOverlap(b.x, b.y, b.w, b.h,
                             rois[i].x, rois[i].y, rois[i].w, rois[i].h)) {
                currentlyOccupied[i] = true;
            }
        }
    }
    return true;
}

void VisionAIModule::emitSpotMessage(uint8_t spotIdx, bool nowOccupied)
{
    char msg[40];
    snprintf(msg, sizeof(msg), "Spot %u %s",
             (unsigned)(spotIdx + 1), nowOccupied ? "occupied" : "vacant");

    meshtastic_MeshPacket *p = allocDataPacket();
    p->want_ack = false;
    p->decoded.payload.size = strlen(msg);
    memcpy(p->decoded.payload.bytes, msg, p->decoded.payload.size);

    LOG_INFO("VisionAI: %s (id=%u)", msg, p->id);
    service->sendToMesh(p);
}

#endif // MESHTASTIC_EV_MONITOR_GARAGE
