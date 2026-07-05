#include <Arduino.h>

#include "stairs_manager.h"

StairsManager stairsManager;

void setup() {
  Serial.begin(115200);
  stairsManager.init();
}

void loop() {
  stairsManager.update();
}
