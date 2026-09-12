#include <Arduino.h>
#include <esp_system.h>

#include "device_info.h"

static const char* resetReasonText(esp_reset_reason_t reason)
{
    switch (reason)
    {
        case ESP_RST_POWERON:   return "Power-on";
        case ESP_RST_EXT:       return "External reset";
        case ESP_RST_SW:        return "Software restart";
        case ESP_RST_PANIC:     return "Panic or exception";
        case ESP_RST_INT_WDT:   return "Interrupt watchdog";
        case ESP_RST_TASK_WDT:  return "Task watchdog";
        case ESP_RST_WDT:       return "Other watchdog";
        case ESP_RST_DEEPSLEEP: return "Deep sleep wake";
        case ESP_RST_BROWNOUT:  return "Brownout";
        case ESP_RST_SDIO:      return "SDIO reset";
        default:                return "Unknown";
    }
}

void DeviceInfo::init()
{
    uint64_t mac = ESP.getEfuseMac();

    snprintf(m_deviceId, sizeof(m_deviceId),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)(mac),
        (uint8_t)(mac >> 8),
        (uint8_t)(mac >> 16),
        (uint8_t)(mac >> 24),
        (uint8_t)(mac >> 32),
        (uint8_t)(mac >> 40));

    m_resetReason = resetReasonText(esp_reset_reason());
}

const char* DeviceInfo::hardwareVersion() const
{
    return HARDWARE_VERSION;
}

const char* DeviceInfo::firmwareVersion() const
{
    return FIRMWARE_VERSION;
}

const char* DeviceInfo::buildTimestamp() const
{
    return BUILD_TIMESTAMP;
}

const char* DeviceInfo::deviceId() const
{
    return m_deviceId;
}

const char* DeviceInfo::resetReason() const
{
    return m_resetReason;
}
