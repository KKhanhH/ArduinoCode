/**
 * @file MicrowaveControl.h
 * @brief Header file for the MicrowaveControl class.
 * This file declares the interface for simulating button presses on a
 * microwave.
 */

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
    /**
     * @brief Constructor.
     *
     * @param inhPin0 The input enable pin for the mux corresponding to 1st row pin.
     * @param inhPin1 The input enable pin for the mux corresponding to 2nd row pin.
     * @param inhPin2 The input enable pin for the mux corresponding to 3rd row pin.
     * @param inhPin3 The input enable pin for the mux corresponding to 4th row pin.
     * @param chSelPin0 The channel select pin for the 1st bit of the muxes.
     * @param chSelPin1 The channel select pin for the 2nd bit of the muxes.
     * @param chSelPin2 The channel select pin for the 3rd bit of the muxes.
     */
    MicrowaveControl(int inhPin0, int inhPin1, int inhPin2, int inhPin3,
                     int chSelPin0, int chSelPin1, int chSelPin2);

    /**
     * @brief Simulates a button press on the microwave keypad.
     * @param rowNum The row number of the button to be pressed (8-11).
     * @param colNum The column number of the button to be pressed (2-7).
     */
    void simulateButton(int rowNum, int colNum);

    /**
     * @brief Initializes the pins used to control the keypad.
     * This function should be called before any button presses are simulated.
     */
    void initializePins();
};
#endif  // MICROWAVECONTROL_H