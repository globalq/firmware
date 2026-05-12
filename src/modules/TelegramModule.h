#pragma once
#include "configuration.h"

#if MESHTASTIC_EV_MONITOR_HOME

#include "SinglePortModule.h"

/**
 * Home-gateway-only module. Listens for incoming TEXT_MESSAGE_APP packets
 * from the mesh and POSTs them to a Telegram bot over WiFi/HTTPS.
 *
 * Bot token and chat ID are baked in at build time from build_flags:
 *   -DEV_TELEGRAM_BOT_TOKEN="..."
 *   -DEV_TELEGRAM_CHAT_ID="..."
 *
 * Skips the POST silently if WiFi isn't connected, so a momentary outage
 * just means one missed alert (mesh receipt itself is unaffected).
 */
class TelegramModule : public SinglePortModule
{
  public:
    TelegramModule() : SinglePortModule("telegram", meshtastic_PortNum_TEXT_MESSAGE_APP) {}

  protected:
    ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;
    bool wantPacket(const meshtastic_MeshPacket *p) override;

  private:
    bool postToTelegram(const char *text);
};

extern TelegramModule *telegramModule;

#endif // MESHTASTIC_EV_MONITOR_HOME
