#include "configuration.h"

#if MESHTASTIC_EV_MONITOR_HOME

#include "TelegramModule.h"
#include "MeshService.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#ifndef EV_TELEGRAM_BOT_TOKEN
#error "EV_TELEGRAM_BOT_TOKEN must be defined via build_flags for ev-monitor-home env"
#endif
#ifndef EV_TELEGRAM_CHAT_ID
#error "EV_TELEGRAM_CHAT_ID must be defined via build_flags for ev-monitor-home env"
#endif

TelegramModule *telegramModule;

bool TelegramModule::wantPacket(const meshtastic_MeshPacket *p)
{
    return MeshService::isTextPayload(p);
}

ProcessMessage TelegramModule::handleReceived(const meshtastic_MeshPacket &mp)
{
    const auto &p = mp.decoded;

    // Build a null-terminated copy of the payload (it isn't guaranteed terminated).
    char text[meshtastic_Constants_DATA_PAYLOAD_LEN + 1];
    size_t n = p.payload.size;
    if (n > sizeof(text) - 1) n = sizeof(text) - 1;
    memcpy(text, p.payload.bytes, n);
    text[n] = '\0';

    LOG_INFO("Telegram: forwarding from=0x%0x msg=%s", mp.from, text);

    if (!postToTelegram(text)) {
        LOG_WARN("Telegram: POST failed or WiFi down; alert not delivered");
    }

    // Don't claim the packet - other modules (e.g. screen) should still see it.
    return ProcessMessage::CONTINUE;
}

bool TelegramModule::postToTelegram(const char *text)
{
    if (WiFi.status() != WL_CONNECTED) {
        LOG_DEBUG("Telegram: WiFi not connected, skipping");
        return false;
    }

    // setInsecure() = skip cert validation. Fine for a learning build; for a
    // hardened deployment, embed Telegram's root CA via WiFiClientSecure::setCACert().
    WiFiClientSecure tls;
    tls.setInsecure();

    HTTPClient http;
    String url = String("https://api.telegram.org/bot") + EV_TELEGRAM_BOT_TOKEN + "/sendMessage";
    if (!http.begin(tls, url)) {
        LOG_ERROR("Telegram: http.begin failed");
        return false;
    }
    http.addHeader("Content-Type", "application/json");

    // Hand-build the JSON to avoid pulling in a JSON lib. Telegram requires
    // the chat_id and text fields; text is JSON-string-escaped minimally below.
    String body = String("{\"chat_id\":\"") + EV_TELEGRAM_CHAT_ID + "\",\"text\":\"";
    for (const char *c = text; *c; c++) {
        if (*c == '"' || *c == '\\') body += '\\';
        if (*c == '\n') { body += "\\n"; continue; }
        body += *c;
    }
    body += "\"}";

    int code = http.POST(body);
    http.end();

    if (code >= 200 && code < 300) {
        LOG_DEBUG("Telegram: POST ok (%d)", code);
        return true;
    }
    LOG_WARN("Telegram: POST returned %d", code);
    return false;
}

#endif // MESHTASTIC_EV_MONITOR_HOME
