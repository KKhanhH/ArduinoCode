#ifndef PRESETFOODS_H
#define PRESETFOODS_H

#include "../MicrowaveControl/MicrowaveControl.h"
#include "../Keypad/Keypad.h"

void handlePresetFood(MicrowaveControl &mcu,int presetIndex, char *responseBuffer, size_t len);

#endif PRESETFOODS_H