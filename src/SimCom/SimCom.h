#ifndef SIMCOM_H
#define SIMCOM_H

#include <Arduino.h>

class SIM7600 {
private:
  Stream *_simSerial;
public:
  SIM7600(Stream *simSerial);
  SIM7600(Stream &simSerial);

  struct SMSStruct {
    char number[30];
    char message[200];
    char timeStr[32];
  };

  struct GPSStruct {
    double latitude;
    double longitude;
    bool status;
  };

  void emptyBuffer();
  int readToBuffer(char result[], int maxChars);

  bool sendATCommand(const char* cmdStr, int timeout, char* result, int maxChars);
  int sendATCompare(const char* cmdStr, int timeout, int expectedCount, ...);
  void SIM7600::sendImmediate(const char* cmdStr);

  void initConfig();

  bool sendSMS(const char* number, const char* msg);
  bool readSMS(int index, SMSStruct& smsData);

  GPSStruct getGPSLocation(unsigned long timeout);
  GPSStruct formatGPS(char* GPSBuffer);

  void sendTTS(const char *message);
  void stopTTS();

  void handleCall();
};
#endif