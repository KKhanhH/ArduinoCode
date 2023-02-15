#include "Keypad.h"

/*------------------------------------------------------------*/

extern const Keypad::readPin Keypad::BTN_TIME_MINDER = {2, 10};
extern const Keypad::readPin Keypad::BTN_CLOCK = {2, 9};
extern const Keypad::readPin Keypad::BTN_EASY_REHEAT = {2, 8};
extern const Keypad::readPin Keypad::BTN_START = {3, 11};
extern const Keypad::readPin Keypad::BTN_STOP_CANCEL = {3, 10};
extern const Keypad::readPin Keypad::BTN_INSTANT_MINUTE = {3, 9};
extern const Keypad::readPin Keypad::BTN_EASY_DEFROST = {3, 8};
extern const Keypad::readPin Keypad::BTN_HIGH = {4, 11};
extern const Keypad::readPin Keypad::BTN_MED_HIGH = {4, 10};
extern const Keypad::readPin Keypad::BTN_MEDIUM = {4, 9};
extern const Keypad::readPin Keypad::BTN_MED_LOW_DEFROST = {4, 8};
extern const Keypad::readPin Keypad::BTN_ONE = {5, 11};
extern const Keypad::readPin Keypad::BTN_TWO = {5, 10};
extern const Keypad::readPin Keypad::BTN_THREE = {5, 9};
extern const Keypad::readPin Keypad::BTN_LOW = {5, 8};
extern const Keypad::readPin Keypad::BTN_FOUR = {6, 11};
extern const Keypad::readPin Keypad::BTN_FIVE = {6, 10};
extern const Keypad::readPin Keypad::BTN_SIX = {6, 9};
extern const Keypad::readPin Keypad::BTN_SEVEN = {7, 11};
extern const Keypad::readPin Keypad::BTN_EIGHT = {7, 10};
extern const Keypad::readPin Keypad::BTN_NINE = {7, 9};
extern const Keypad::readPin Keypad::BTN_ZERO = {7, 8};
extern const Keypad::readPin Keypad::BTN_UNPRESSED = {0, 0};

Keypad::Keypad(int rowStart, int colStart)
    : _rowStart(rowStart), _colStart(colStart) {
    // Initialize row pins to input, with a pullup to avoid floating inputs
    for (int r = _rowStart; r < _rowStart + 4; r++) {
        pinMode(r, INPUT_PULLUP);
    }
    // Initialize col pins to output, with active low
    for (int c = _colStart; c < _colStart + 6; c++) {
        pinMode(c, OUTPUT);
    }
}
void Keypad::initializePins() {
    // Initialize col pins to high output
    for (int c = _colStart; c < _colStart + 6; c++) {
        digitalWrite(c, HIGH);
    }
}

Keypad::readPin Keypad::readKeypad() {
    readPin result = {0, 0};

    // cycle through each column pin
    for (int c = _colStart; c < _colStart + 6; c++) {
        digitalWrite(c, LOW);

        // evaluate which row pin is low
        for (int r = _rowStart; r < _rowStart + 4; r++) {
            int roweval = digitalRead(r);
            if (roweval != HIGH) {
                result.rowPin = r - _rowStart + 8;
                result.colPin = c - _colStart + 2;

                digitalWrite(c, HIGH);
                return result;
            }
        }

        digitalWrite(c, HIGH);
    }
    return result;
}

const char* Keypad::buttonStr(readPin input) {
    if (input == BTN_TIME_MINDER)
        return "BTN_TIME_MINDER";
    else if (input == BTN_CLOCK)
        return "BTN_CLOCK";
    else if (input == BTN_EASY_REHEAT)
        return "BTN_EASY_REHEAT";
    else if (input == BTN_START)
        return "BTN_START";
    else if (input == BTN_STOP_CANCEL)
        return "BTN_STOP_CANCEL";
    else if (input == BTN_INSTANT_MINUTE)
        return "BTN_INSTANT_MINUTE";
    else if (input == BTN_EASY_DEFROST)
        return "BTN_EASY_DEFROST";
    else if (input == BTN_HIGH)
        return "BTN_HIGH";
    else if (input == BTN_MED_HIGH)
        return "BTN_MED_HIGH";
    else if (input == BTN_MEDIUM)
        return "BTN_MEDIUM";
    else if (input == BTN_MED_LOW_DEFROST)
        return "BTN_MED_LOW_DEFROST";
    else if (input == BTN_LOW)
        return "BTN_LOW";
    else if (input == BTN_ONE)
        return "BTN_ONE";
    else if (input == BTN_TWO)
        return "BTN_TWO";
    else if (input == BTN_THREE)
        return "BTN_THREE";
    else if (input == BTN_FOUR)
        return "BTN_FOUR";
    else if (input == BTN_FIVE)
        return "BTN_FIVE";
    else if (input == BTN_SIX)
        return "BTN_SIX";
    else if (input == BTN_SEVEN)
        return "BTN_SEVEN";
    else if (input == BTN_EIGHT)
        return "BTN_EIGHT";
    else if (input == BTN_NINE)
        return "BTN_NINE";
    else if (input == BTN_ZERO)
        return "BTN_ZERO";
    else if (input == BTN_UNPRESSED)
        return "BTN_UNPRESSED";
}

Keypad::readPin Keypad::dtmfLookup(int buttonNumber) {
    switch(buttonNumber) {
        case '0':
            return Keypad::BTN_ZERO;
        case '1':
            return Keypad::BTN_ONE;
        case '2':
            return Keypad::BTN_TWO;
        case '3':
            return Keypad::BTN_THREE;
        case '4':
            return Keypad::BTN_FOUR;
        case '5':
            return Keypad::BTN_FIVE;
        case '6':
            return Keypad::BTN_SIX;
        case '7':
            return Keypad::BTN_SEVEN;
        case '8':
            return Keypad::BTN_EIGHT;
        case '9':
            return Keypad::BTN_NINE;
        case '*':
            return Keypad::BTN_STOP_CANCEL;
        case '#':
            return Keypad::BTN_START;
        default:
            return Keypad::BTN_UNPRESSED;
    }
}