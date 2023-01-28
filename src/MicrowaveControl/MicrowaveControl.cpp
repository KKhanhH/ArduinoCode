#include "MicrowaveControl.h"
MicrowaveControl::MicrowaveControl(int inhPin0, int inhPin1, int inhPin2,
                                   int inhPin3, int chSelPin0, int chSelPin1,
                                   int chSelPin2)
    : _inhPin0(inhPin0),
      _inhPin1(inhPin1),
      _inhPin2(inhPin2),
      _inhPin3(inhPin3),
      _chSelPin0(chSelPin0),
      _chSelPin1(chSelPin1),
      _chSelPin2(chSelPin2) {

        pinMode(_inhPin0, OUTPUT);
        pinMode(_inhPin1, OUTPUT);
        pinMode(_inhPin2, OUTPUT);
        pinMode(_inhPin3, OUTPUT);
        pinMode(_chSelPin0, OUTPUT);
        pinMode(_chSelPin1, OUTPUT);
        pinMode(_chSelPin2, OUTPUT);
    }
void MicrowaveControl::initializePins() {
    digitalWrite(_inhPin0, HIGH);
    digitalWrite(_inhPin1, HIGH);
    digitalWrite(_inhPin2, HIGH);
    digitalWrite(_inhPin3, HIGH);
    digitalWrite(_chSelPin0, LOW);
    digitalWrite(_chSelPin1, LOW);
    digitalWrite(_chSelPin2, LOW);
}

void MicrowaveControl::simulateButton(int rowNum, int colNum) {
    // Row num 8-11, col num 2-7
    int rowOffsetted = rowNum - 8;
    // No button pressed
    if(rowNum == 0 && colNum == 0) {
        return;
    }
    digitalWrite(_chSelPin0, colNum & 0x1);
    digitalWrite(_chSelPin1, colNum & 0x2);
    digitalWrite(_chSelPin2, colNum & 0x4);
    
    delay(1);
    // Simulate button press
    switch(rowOffsetted) {
        case 0:
            digitalWrite(_inhPin0, LOW);
            break;
        case 1:
            digitalWrite(_inhPin1, LOW);
            break;
        case 2:
            digitalWrite(_inhPin2, LOW);
            break;
        case 3:
            digitalWrite(_inhPin3, LOW);
            break;
        default:
            break;
    }


    // Keep button held down
    delay(20);
    // Unpress
    digitalWrite(_inhPin0, HIGH);
    digitalWrite(_inhPin1, HIGH);
    digitalWrite(_inhPin2, HIGH);
    digitalWrite(_inhPin3, HIGH);
    delay(120);

}

