#include <Arduino.h>

#include "config.h"
#include "device_info.h"
#include "network_manager.h"
#include "stairs_manager.h"

DeviceInfo     deviceInfo;
NetworkManager networkManager;
StairsManager  stairsManager;

void setup()
{
    Serial.begin(SERIAL_BAUD);

    deviceInfo.init();
    deviceInfo.print(Serial);

    networkManager.init();
    stairsManager.init(&networkManager);
}

void loop()
{
    networkManager.update();
    stairsManager.update();
}
