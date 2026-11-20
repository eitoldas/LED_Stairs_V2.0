#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "logger.h"
#include "network_config.h"
#include "settings_store.h"
#include "web_interface.h"

static String clockText(uint16_t minutes)
{
    char text[6];
    snprintf(text, sizeof(text), "%02u:%02u", minutes / 60, minutes % 60);
    return String(text);
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
        if (WiFi.status() == WL_CONNECTED)
        {
            start();
        }
        return;
    }

    m_server.handleClient();
}

void WebInterface::start()
{
    m_server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    m_server.onNotFound([this]() { handleNotFound(); });

    m_server.begin();
    MDNS.addService("http", "tcp", WEB_SERVER_PORT);

    m_started = true;
    Log.printf("Web: serving on http://%s.local (%s)", OTA_HOSTNAME, WiFi.localIP().toString().c_str());
    Log.println();
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

void WebInterface::handleNotFound()
{
    m_server.send(404, "text/plain", "Not found");
}
