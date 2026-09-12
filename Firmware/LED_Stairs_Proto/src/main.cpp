#include <Arduino.h>

#include "device_info.h"
#include "network_manager.h"
#include "stairs_manager.h"

DeviceInfo     deviceInfo;
NetworkManager networkManager;
StairsManager  stairsManager;

void setup()
{
    deviceInfo.init();
    networkManager.init();
    stairsManager.init(&networkManager);
}

void loop()
{
    networkManager.update();
    stairsManager.update();
}
