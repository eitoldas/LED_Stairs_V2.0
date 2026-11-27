#include <Arduino.h>

#include "config.h"
#include "current_sensor.h"
#include "device_info.h"
#include "logger.h"
#include "network_config.h"
#include "network_manager.h"
#include "settings_store.h"
#include "stairs_manager.h"
#include "web_interface.h"

CurrentSensor  currentSensor;
DeviceInfo     deviceInfo;
NetworkManager networkManager;
StairsManager  stairsManager;
WebInterface   webInterface;

void setup()
{
    Serial.begin(SERIAL_BAUD);
    Log.init(Serial);

    deviceInfo.init();
    deviceInfo.print(Log);

    if (IGNORE_NIGHT_WINDOW)
    {
        Log.println("TEST BUILD: night window ignored, lights respond at any time");
    }

    Settings.init();

    currentSensor.init(CURRENT_SENSE_PIN, LOAD_VOLTAGE);

    networkManager.init();
    stairsManager.init(&networkManager);
    webInterface.init(&deviceInfo, &currentSensor, &networkManager);
}

void loop()
{
    networkManager.update();
    stairsManager.update();
    currentSensor.update();
    webInterface.update();

    // Temporary: dump the sensor reading while it is being tested.
    static uint32_t lastReportMs = 0;

    uint32_t now = millis();
    if (now - lastReportMs >= CURRENT_REPORT_INTERVAL_MS)
    {
        lastReportMs = now;
        Log.printf("%4u mV  %6.2f A  %6.1f W\n",
            currentSensor.millivolts(), currentSensor.amps(), currentSensor.watts());
    }
}
