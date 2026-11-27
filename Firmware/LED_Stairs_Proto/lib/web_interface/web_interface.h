#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "current_sensor.h"
#include "device_info.h"
#include "network_config.h"
#include "network_manager.h"

/*
 * Small local web server: a status page plus the editable settings. It starts
 * once WiFi is connected and is reachable at http://<OTA_HOSTNAME>.local.
 */

class WebInterface
{
public:
    void init(const DeviceInfo* deviceInfo,
              const CurrentSensor* currentSensor,
              const NetworkManager* networkManager);
    void update();

private:
    void start();

    void handleRoot();
    void handleStatus();
    void handleGetSettings();
    void handleSaveSettings();
    void handleRestoreDefaults();
    void handleNotFound();

    void sendSettings();
    void sendError(int code, const char* message);

    WebServer m_server{WEB_SERVER_PORT};

    const DeviceInfo*     m_deviceInfo     = nullptr;
    const CurrentSensor*  m_currentSensor  = nullptr;
    const NetworkManager* m_networkManager = nullptr;

    bool m_started = false;
};
