#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "logger.h"
#include "network_config.h"
#include "settings_store.h"
#include "web_interface.h"
#include "serial_page.h"
#include "web_page.h"

static String clockText(uint16_t minutes)
{
    char text[6];
    snprintf(text, sizeof(text), "%02u:%02u", minutes / 60, minutes % 60);
    return String(text);
}

static String clockText(uint8_t hour, uint8_t minute)
{
    return clockText((uint16_t)(hour * 60 + minute));
}

// A missing field leaves the current value alone; a present one must be a
// whole number from 0 to max.
static bool readNumber(JsonVariantConst value, uint32_t max, uint32_t& out)
{
    if (value.isNull())
    {
        return true;
    }

    if (!value.is<long>())
    {
        return false;
    }

    long number = value.as<long>();
    if (number < 0 || (uint32_t)number > max)
    {
        return false;
    }

    out = (uint32_t)number;
    return true;
}

// A missing field leaves the address alone, an empty string clears it, and
// anything else must be a dotted IPv4 address.
static bool readAddress(JsonVariantConst value, uint32_t& out)
{
    if (value.isNull())
    {
        return true;
    }

    const char* text = value.as<const char*>();
    if (text == nullptr)
    {
        return false;
    }

    if (text[0] == '\0')
    {
        out = 0;
        return true;
    }

    IPAddress address;
    if (!address.fromString(text))
    {
        return false;
    }

    out = (uint32_t)address;
    return true;
}

static String addressText(uint32_t address)
{
    return (address != 0) ? IPAddress(address).toString() : String();
}

static bool readClock(JsonVariantConst value, uint8_t& hour, uint8_t& minute)
{
    if (value.isNull())
    {
        return true;
    }

    unsigned h = 0;
    unsigned m = 0;
    const char* text = value.as<const char*>();

    if (text == nullptr || sscanf(text, "%u:%u", &h, &m) != 2 || h > 23 || m > 59)
    {
        return false;
    }

    hour   = (uint8_t)h;
    minute = (uint8_t)m;
    return true;
}

void WebInterface::init(const DeviceInfo* deviceInfo,
                        const CurrentSensor* currentSensor,
                        const NetworkManager* networkManager)
{
    m_deviceInfo     = deviceInfo;
    m_currentSensor  = currentSensor;
    m_networkManager = networkManager;
}

void WebInterface::update()
{
    if (!m_started)
    {
        if (m_networkManager->isConnected())
        {
            start();
        }
        return;
    }

    m_server.handleClient();
}

void WebInterface::start()
{
    m_server.on("/",                      HTTP_GET,  [this]() { handleRoot(); });
    m_server.on("/serial",                HTTP_GET,  [this]() { handleSerialPage(); });
    m_server.on("/api/status",            HTTP_GET,  [this]() { handleStatus(); });
    m_server.on("/api/settings",          HTTP_GET,  [this]() { handleGetSettings(); });
    m_server.on("/api/settings",          HTTP_POST, [this]() { handleSaveSettings(); });
    m_server.on("/api/settings/defaults", HTTP_POST, [this]() { handleRestoreDefaults(); });
    m_server.on("/api/logs",              HTTP_GET,  [this]() { handleLogs(); });
    m_server.onNotFound([this]() { handleNotFound(); });

    m_server.begin();
    MDNS.addService("http", "tcp", WEB_SERVER_PORT);

    m_started = true;
    Log.printf("Web: serving on http://%s.local (%s)", OTA_HOSTNAME, WiFi.localIP().toString().c_str());
    Log.println();
}

void WebInterface::handleRoot()
{
    m_server.send_P(200, "text/html", WEB_PAGE);
}

void WebInterface::handleSerialPage()
{
    m_server.send_P(200, "text/html", SERIAL_PAGE);
}

void WebInterface::handleStatus()
{
    JsonDocument doc;

    doc["firmware"]    = m_deviceInfo->firmwareVersion();
    doc["hardware"]    = m_deviceInfo->hardwareVersion();
    doc["built"]       = m_deviceInfo->buildTimestamp();
    doc["deviceId"]    = m_deviceInfo->deviceId();
    doc["resetReason"] = m_deviceInfo->resetReason();

    doc["uptimeSeconds"] = millis() / 1000;
    doc["wifiRssi"]      = WiFi.RSSI();
    doc["ip"]            = WiFi.localIP().toString();

    struct tm timeinfo {};
    if (m_networkManager->getLocalTime(&timeinfo))
    {
        char text[9];
        strftime(text, sizeof(text), "%H:%M:%S", &timeinfo);
        doc["time"] = text;
    }
    else
    {
        doc["time"] = nullptr;
    }

    if (m_networkManager->hasSunTimes())
    {
        doc["sunrise"] = clockText(m_networkManager->sunriseMinutes());
        doc["sunset"]  = clockText(m_networkManager->sunsetMinutes());
    }
    else
    {
        doc["sunrise"] = nullptr;
        doc["sunset"]  = nullptr;
    }

    doc["lightsAllowed"] = m_networkManager->isLightsAllowed();
    doc["testBuild"]     = IGNORE_NIGHT_WINDOW;

    doc["amps"]  = serialized(String(m_currentSensor->amps(), 2));
    doc["watts"] = serialized(String(m_currentSensor->watts(), 1));

    String body;
    serializeJson(doc, body);
    m_server.send(200, "application/json", body);
}

void WebInterface::handleGetSettings()
{
    sendSettings();
}

void WebInterface::handleSaveSettings()
{
    JsonDocument doc;
    if (deserializeJson(doc, m_server.arg("plain")) || !doc.is<JsonObject>())
    {
        sendError(400, "Body must be a JSON object");
        return;
    }

    StairSettings settings = Settings.get();

    uint32_t stepDelayMs = settings.stepDelayMs;
    uint32_t holdDelayMs = settings.holdDelayMs;

    bool ok = readNumber(doc["stepDelayMs"],         UINT16_MAX, stepDelayMs)
           && readNumber(doc["holdDelayMs"],         UINT16_MAX, holdDelayMs)
           && readNumber(doc["retriggerCooldownMs"], UINT32_MAX, settings.retriggerCooldownMs)
           && readNumber(doc["exitOffsetMs"],        UINT32_MAX, settings.exitOffsetMs)
           && readClock(doc["nightStart"], settings.nightStartHour, settings.nightStartMinute)
           && readClock(doc["nightEnd"],   settings.nightEndHour,   settings.nightEndMinute)
           && readAddress(doc["staticIp"], settings.staticIp)
           && readAddress(doc["gateway"],  settings.gateway)
           && readAddress(doc["subnet"],   settings.subnet)
           && readAddress(doc["dns"],      settings.dns);

    if (!doc["automaticDaylight"].isNull())
    {
        ok = ok && doc["automaticDaylight"].is<bool>();
        settings.automaticDaylight = doc["automaticDaylight"].as<bool>();
    }

    settings.stepDelayMs = (uint16_t)stepDelayMs;
    settings.holdDelayMs = (uint16_t)holdDelayMs;

    if (!ok || !Settings.save(settings))
    {
        sendError(400, "One or more settings are missing a valid value or out of range");
        return;
    }

    sendSettings();
}

void WebInterface::handleRestoreDefaults()
{
    Settings.restoreDefaults();
    sendSettings();
}

void WebInterface::sendSettings()
{
    const StairSettings& settings = Settings.get();
    JsonDocument doc;

    doc["stepDelayMs"]         = settings.stepDelayMs;
    doc["holdDelayMs"]         = settings.holdDelayMs;
    doc["retriggerCooldownMs"] = settings.retriggerCooldownMs;
    doc["exitOffsetMs"]        = settings.exitOffsetMs;
    doc["automaticDaylight"]   = settings.automaticDaylight;
    doc["nightStart"]          = clockText(settings.nightStartHour, settings.nightStartMinute);
    doc["nightEnd"]            = clockText(settings.nightEndHour, settings.nightEndMinute);
    doc["staticIp"]            = addressText(settings.staticIp);
    doc["gateway"]             = addressText(settings.gateway);
    doc["subnet"]              = addressText(settings.subnet);
    doc["dns"]                 = addressText(settings.dns);

    String body;
    serializeJson(doc, body);
    m_server.send(200, "application/json", body);
}

void WebInterface::sendError(int code, const char* message)
{
    JsonDocument doc;
    doc["error"] = message;

    String body;
    serializeJson(doc, body);
    m_server.send(code, "application/json", body);
}

// Returns the log lines a browser hasn't seen yet. Lines are numbered by the
// logger's running total: a client passes back the "total" it last received
// as ?since=, and gets only newer lines. If it fell further behind than the
// buffer holds, "missed" says how many lines it lost.
void WebInterface::handleLogs()
{
    uint32_t total  = Log.totalLines();
    uint32_t count  = Log.lineCount();
    uint32_t oldest = total - count;

    uint32_t since = m_server.hasArg("since") ? strtoul(m_server.arg("since").c_str(), nullptr, 10) : 0;
    if (since > total)
    {
        since = 0;
    }

    uint32_t first = (since > oldest) ? since : oldest;

    JsonDocument doc;
    doc["total"]  = total;
    doc["missed"] = first - since;

    JsonArray lines = doc["lines"].to<JsonArray>();
    for (uint32_t number = first; number < total; number++)
    {
        lines.add(Log.line(number - oldest));
    }

    String body;
    serializeJson(doc, body);
    m_server.send(200, "application/json", body);
}

void WebInterface::handleNotFound()
{
    m_server.send(404, "text/plain", "Not found");
}
