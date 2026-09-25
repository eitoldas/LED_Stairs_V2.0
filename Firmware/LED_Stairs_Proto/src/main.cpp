#include <Arduino.h>

#include "config.h"
#include "current_sensor.h"
#include "device_info.h"
#include "network_manager.h"
#include "stairs_manager.h"

CurrentSensor  currentSensor;
DeviceInfo     deviceInfo;
NetworkManager networkManager;
StairsManager  stairsManager;

void setup()
{
    Serial.begin(SERIAL_BAUD);

    deviceInfo.init();
    deviceInfo.print(Serial);

    currentSensor.init(CURRENT_SENSE_PIN, LOAD_VOLTAGE);

    networkManager.init();
    stairsManager.init(&networkManager);
}

void loop()
{
    networkManager.update();
    stairsManager.update();
    currentSensor.update();

    // Temporary: dump the sensor reading while it is being tested.
    static uint32_t lastReportMs = 0;

    uint32_t now = millis();
    if (now - lastReportMs >= CURRENT_REPORT_INTERVAL_MS)
    {
        lastReportMs = now;
        Serial.printf("%4u mV  %6.2f A  %6.1f W\n",
            currentSensor.millivolts(), currentSensor.amps(), currentSensor.watts());
    }
}
