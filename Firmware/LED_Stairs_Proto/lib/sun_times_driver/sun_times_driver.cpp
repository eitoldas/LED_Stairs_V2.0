#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "network_config.h"
#include "sun_times_driver.h"

static bool parseClockTime(const char* text, uint16_t& minutes)
{
    unsigned hours = 0;
    unsigned mins  = 0;

    if (text == nullptr || sscanf(text, "%u:%u", &hours, &mins) != 2)
    {
        return false;
    }

    if (hours > 23 || mins > 59)
    {
        return false;
    }

    minutes = (uint16_t)(hours * 60 + mins);
    return true;
}

void SunTimesDriver::init(double latitude, double longitude)
{
    m_latitude  = latitude;
    m_longitude = longitude;
}

bool SunTimesDriver::fetch()
{
    char url[128];
    snprintf(url, sizeof(url), "%s?lat=%.6f&lng=%.6f&time_format=24",
        SUN_API_URL, m_latitude, m_longitude);

    // Sunrise times aren't worth pinning a certificate for, and a pinned root
    // would eventually expire and quietly break the lookup.
    WiFiClientSecure client;
    client.setInsecure();
    client.setHandshakeTimeout(SUN_API_TIMEOUT_MS / 1000);

    HTTPClient http;
    http.setConnectTimeout(SUN_API_TIMEOUT_MS);
    http.setTimeout(SUN_API_TIMEOUT_MS);

    if (!http.begin(client, url))
    {
        return false;
    }

    int    status = http.GET();
    String body   = (status == HTTP_CODE_OK) ? http.getString() : String();
    http.end();

    if (body.isEmpty())
    {
        return false;
    }

    JsonDocument filter;
    filter["status"]             = true;
    filter["results"]["sunrise"] = true;
    filter["results"]["sunset"]  = true;

    JsonDocument doc;
    if (deserializeJson(doc, body, DeserializationOption::Filter(filter)))
    {
        return false;
    }

    if (doc["status"] != "OK")
    {
        return false;
    }

    uint16_t sunrise = 0;
    uint16_t sunset  = 0;

    if (!parseClockTime(doc["results"]["sunrise"].as<const char*>(), sunrise)
        || !parseClockTime(doc["results"]["sunset"].as<const char*>(), sunset))
    {
        return false;
    }

    m_sunriseMinutes = sunrise;
    m_sunsetMinutes  = sunset;
    m_hasTimes       = true;
    return true;
}

bool SunTimesDriver::hasTimes() const
{
    return m_hasTimes;
}

uint16_t SunTimesDriver::sunriseMinutes() const
{
    return m_sunriseMinutes;
}

uint16_t SunTimesDriver::sunsetMinutes() const
{
    return m_sunsetMinutes;
}
