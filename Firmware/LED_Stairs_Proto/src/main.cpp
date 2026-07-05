#include <Arduino.h>

#include "stairs_manager.h"

StairsManager stairsManager;

void setup() {
  stairsManager.init();
}

void loop() {
  stairsManager.update();
}
