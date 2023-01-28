#ifndef MICROWAVECONTROL_H
#define MICROWAVECONTROL_H
#include <Arduino.h>

class MicrowaveControl {
   private:
    int _inhPin0;
    int _inhPin1;
    int _inhPin2;
    int _inhPin3;

    int _chSelPin0;
    int _chSelPin1;
    int _chSelPin2;

   public:
    MicrowaveControl(int inhPin0, int inhPin1, int inhPin2, int inhPin3,
                     int chSelPin0, int chSelPin1, int chSelPin2);
    
    void simulateButton(int rowNum, int colNum);
    void initializePins();
};
#endif  // MICROWAVECONTROL_H