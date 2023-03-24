/**
 * @file SimCom.h
 * @brief This file contains the declarations of the SIM7600 class.
 *
 * The SIM7600 class provides an interface for working with the SIMCOM SIM7600
 * cellular module. It provides functions to allow the user to send and receive
 * SMS messages, and read GPS data.
 *
 */

#ifndef SIMCOM_H
#define SIMCOM_H

#include <Arduino.h>

class SIM7600 {
   private:
    Stream* _simSerial;

   public:
    /**
     * @brief Overloaded constructor for SIM7600 object
     *
     * @param simSerial The serial port to communicate with the SIM7600 module.
     */
    SIM7600(Stream* simSerial);

    /**
     * @brief Overloaded constructor for SIM7600 object
     *
     * @param simSerial The serial port to communicate with the SIM7600 module.
     */
    SIM7600(Stream& simSerial);

    /**
     * @brief A struct to store data & metadata about an SMS message.
     *
     * The phone number is limited to 29 chars, message is limited to 199 chars,
     * and the time string is limited to 31 chars.
     */
    struct SMSStruct {
        char number[30];
        char message[200];
        char timeStr[32];
    };
    /**
     * @brief A struct to store GPS coordinates
     */
    struct GPSStruct {
        double latitude;
        double longitude;
        bool status;
    };
    /**
     * @brief Completely flush the serial data buffer for the Sim module
     */
    void emptyBuffer();

    /**
     * @brief Reads data in serial buffer into a character buffer and null
     * terminates it.
     *
     * @param result The character buffer to write data into
     * @param maxChars The maximum number of characters to write into the
     * buffer.
     * @return Number of characters read, or 0 if nothing was read.
     */
    int readToBuffer(char result[], int maxChars);

    /**
     * @brief Sends an AT command to the sim module.
     *
     * SendATCommand sends a custom AT command to the SIM7600 and stores the
     * response received after sending the command into a character buffer. This
     * function blocks until either a response has been received or the timeout
     * runs out.
     *
     * @param cmdStr The AT command string to send.
     * @param timeout An integer specifying how long to wait for a response.
     * @param result The character buffer storing the response.
     * @param maxChars The maximum characters to write into the response buffer.
     * @return A boolean representing whether the response was received in time.
     */
    bool sendATCommand(const char* cmdStr, int timeout, char* result,
                       int maxChars);

    /**
     * @brief Sends an AT command to the sim module and compares the response
     * to an expected number of strings.
     *
     * This function sends a custom AT command to the SIM7600 and waits for a
     * response. The response is then compared to each string in the variable
     * arguments list until a match is found. The number of expected strings
     * is specified as a parameter. The function returns the index of the
     * matching string, starting from 1, or 0 if there is no match.
     *
     * @param cmdStr The AT command string to send.
     * @param timeout An integer specifying how long to wait for a response.
     * @param expectedCount An integer specifying the number of expected
     * responses.
     * @param ... The expected response strings.
     * @return An integer representing the index of the matching string, or 0
     * if there is no match.
     */
    int sendATCompare(const char* cmdStr, int timeout, int expectedCount, ...);

    /**
     * @brief Sends an AT command to the sim module without waiting for a
     * response.
     *
     * This function sends an AT command to the SIM7600 without waiting for a
     * response. The response is not buffered, so it should be used for simple,
     * one-off commands only.
     *
     * @param cmdStr The AT command string to send.
     */
    void SIM7600::sendImmediate(const char* cmdStr);

    /**
     * @brief Initializes the SIM7600 configuration by setting various
     * parameters.
     *
     * The function initializes the configuration of the SIM7600 module by
     * setting various parameters such as the SMS text mode, GPS power state,
     * etc. During the initialization, the function sends various AT commands to
     * the module and expects certain responses
     *
     * @param timeout The time in milliseconds to wait for the SIM7600 to
     * connect to the carrier.
     */
    void initConfig(unsigned long timeout);

    /**
     * @brief Sends an SMS message to the specified phone number.
     *
     * @param number The phone number to send the message to.
     * @param msg The message to send.
     * @return A boolean representing whether the message was sent successfully.
     */
    bool sendSMS(const char* number, const char* msg);

    /**
     * @brief Read an SMS message from the SIM module's message storage.
     *
     * Reads an SMS message from the SIM module's message storage into the given
     * SMSStruct. The function blocks until either the message is read or the
     * timeout expires.
     *
     * @param index The index of the message to read. The oldest message has an
     * index of 1, while the most recent message has an index of N, where N is
     * the total number of messages.
     * @param smsData A reference to a SMSStruct that will be filled with the
     * contents of the message.
     * @return True if the message was read successfully, false otherwise.
     */
    bool readSMS(int index, SMSStruct& smsData);

    /**
     * @brief Get the GPS location of the SIM module.
     *
     * Requests the GPS location of the SIM module from the module's GPS
     * subsystem and returns the latitude and longitude as a GPSStruct.
     *
     * The function blocks until a response is received from the module or the
     * timeout expires.
     *
     * @param timeout The maximum amount of time to wait for a response from the
     * module.
     * @return A GPSStruct representing the current location of the SIM module.
     */
    GPSStruct getGPSLocation(unsigned long timeout);

    /**
     * @brief Parse a GPS location string and return the latitude and longitude
     * as a GPSStruct.
     *
     * Takes a NMEA GPS location string and returns the latitude and longitude
     * as a GPSStruct. If the GPS subsystem is not able to provide a valid fix,
     * the returned GPSStruct will have status set to false.
     *
     * @param GPSBuffer The NMEA GPS location string to parse.
     * @return A GPSStruct representing the latitude and longitude in the GPS
     * location string.
     */
    GPSStruct formatGPS(char* GPSBuffer);

    /**
     * @brief Send a text-to-speech message to a connected phone.
     *
     * Sends a text-to-speech message to a connected phone by calling the
     * SIM module's text-to-speech API. The message is synthesized on the module
     * and then played through the phone's earpiece.
     *
     * @param message The message to be spoken.
     */
    void sendTTS(const char* message);

    /**
     * @brief Stop a text-to-speech message that is currently being played.
     *
     * Sends an AT command to the SIM module to stop any text-to-speech message
     * that is currently being played.
     */
    void stopTTS();

    void handleCall();
};
#endif