#include "PresetFoods.h"

struct PresetFood {
  char index[2];
  char name[16];
  char description[256];
  unsigned char stepCount;
  Keypad::readPin buttonSteps[8];
};

static PresetFood foods[3] = {
  {"1", "POPCORN", "1 Bag.", 3, {Keypad::BTN_TWO, Keypad::BTN_THREE, Keypad::BTN_ZERO}},
  {"2", "RICE", "1 Cup.", 5,{Keypad::BTN_ONE, Keypad::BTN_TWO, Keypad::BTN_ZERO, Keypad::BTN_ZERO, Keypad:: BTN_MED_LOW_DEFROST}},
  {"3", "POTATO", "1 Baked Potato. Flip Half Way Through.", 3, {Keypad::BTN_ONE, Keypad::BTN_ZERO, Keypad::BTN_ZERO}}
};

void handlePresetFood(MicrowaveControl &mcu,int presetIndex, char *responseBuffer, size_t len) {
  int foodCount = sizeof(foods) / sizeof(PresetFood);
  int responseCounter = 0;
  if(presetIndex <= 0 || presetIndex > foodCount) {

    strcpy(responseBuffer, "\"PRESET <OPT>\"");
    responseCounter += 14;
    Serial.print("CRASH CHECK2");
    for(int i = 0; i < foodCount; ++i) {
      int size = snprintf(NULL, 0, "\n%s: %s - %s", foods[i].index, foods[i].name, foods[i].description);
      snprintf(&responseBuffer[responseCounter], len - responseCounter, "\n%s: %s - %s", foods[i].index, foods[i].name, foods[i].description);
      responseCounter += size - 1;
    }

  } else {
    PresetFood &currentFood = foods[presetIndex - 1];
    snprintf(responseBuffer, len , "Cooking preset %s: %s - %s", currentFood.index, currentFood.name, currentFood.description);
    for(int i = 0; i < currentFood.stepCount; ++i) {
      mcu.simulateButton(currentFood.buttonSteps[i].rowPin, currentFood.buttonSteps[i].colPin);
    }
    mcu.simulateButton(Keypad::BTN_START.rowPin, Keypad::BTN_START.colPin);
  }
}