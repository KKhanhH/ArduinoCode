#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>

class Keypad {
   private:
    int _rowStart;
    int _colStart;

   public:
    /**
     * @brief Structure representing an individual microwave button function.
     * Contains pin number matching microwave diagram and has overloaded
     * comparison functions.
     */
    struct readPin {
        int colPin;
        int rowPin;
        bool operator==(const readPin &other) const {
            return (colPin == other.colPin && rowPin == other.rowPin);
        }

        bool operator!=(const readPin &other) const {
            return (*this == other) == false;
        }
    };
    /*----------------------BUTTON MAPPING------------------------*/
    const static readPin BTN_TIME_MINDER;
    const static readPin BTN_CLOCK;
    const static readPin BTN_EASY_REHEAT;
    const static readPin BTN_START;
    const static readPin BTN_STOP_CANCEL;
    const static readPin BTN_INSTANT_MINUTE;
    const static readPin BTN_EASY_DEFROST;
    const static readPin BTN_HIGH;
    const static readPin BTN_MED_HIGH;
    const static readPin BTN_MEDIUM;
    const static readPin BTN_MED_LOW_DEFROST;
    const static readPin BTN_ONE;
    const static readPin BTN_TWO;
    const static readPin BTN_THREE;
    const static readPin BTN_LOW;
    const static readPin BTN_FOUR;
    const static readPin BTN_FIVE;
    const static readPin BTN_SIX;
    const static readPin BTN_SEVEN;
    const static readPin BTN_EIGHT;
    const static readPin BTN_NINE;
    const static readPin BTN_ZERO;
    const static readPin BTN_UNPRESSED;

    /**
     * @brief Construct a new Keypad object as a physical input for the
     * PhoneMicrowave System. Row pins and column pins must be consecutive.
     * Row pins are set to inputs with pullup, while col pins are set to output.
     *
     * @param rowStart Pin number corresponding to start of row pins for keypad
     * @param colStart Pin number corresponding to start of column pins for
     * keypad
     */
    Keypad(int rowStart, int colStart);

    /**
     * @brief Sets column pin output initial state to HIGH.
     * 
     */
    void initializePins();

    /**
     * @brief Scans keypad for a press and if pressed, return a struct with matching pins. 
     *
     * @return readPin Struct containing pins matching button that was pressed. 
     * Can be compared to constant structs that match the pressed button.
     */
    readPin readKeypad();

    /**
     * @brief Looks up the readPin struct for a regular number button on keypad (not function button)
     * 
     * @param buttonNumber A single digit positive integer
     * @return readPin Struct containing pins matching number button
     */
    readPin dtmfLookup(int buttonNumber);

    /**
     * @brief Get string representation of button pressed for debugging purposes
     * 
     * @return const char* Button string.
     */
    const char *buttonStr(readPin);
};
#endif  // KEYPAD_H