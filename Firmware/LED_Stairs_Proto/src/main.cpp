#include <Arduino.h>

#include "network_manager.h"
#include "stairs_manager.h"

NetworkManager networkManager;
StairsManager  stairsManager;

void setup()
{
    networkManager.init();
    stairsManager.init(&networkManager);
}

void loop()
{
    networkManager.update();
    stairsManager.update();
}
