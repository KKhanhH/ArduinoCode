#include "SimCom.h"

SIM7600::SIM7600(Stream* simSerial) : _simSerial(simSerial) {}
SIM7600::SIM7600(Stream& simSerial) : _simSerial(&simSerial) {}

void SIM7600::emptyBuffer() {
    while (_simSerial->available() > 0) _simSerial->read();
}

int SIM7600::readToBuffer(char result[], int maxChars) {
    int index = 0;
    if (_simSerial->available() > 0) {
        index = _simSerial->readBytes(result, maxChars - 1);
    }
    result[index] = '\0';
    return index;  // Length of input, or 0 if nothing read
}

void SIM7600::sendImmediate(const char* cmdStr) { _simSerial->println(cmdStr); }

bool SIM7600::sendATCommand(const char* cmdStr, int timeout, char* result,
                            int maxChars) {
    bool receivedResponse = false;

    // Clean the input buffer
    emptyBuffer();

    // Send the AT command
    _simSerial->println(cmdStr);
    unsigned long startTime = millis();
    do {
        if (readToBuffer(result, 256) != 0) {
            receivedResponse = true;
        }
    } while (receivedResponse == false && (millis() - startTime) < timeout);
    return receivedResponse;
}

int SIM7600::sendATCompare(const char* cmdStr, int timeout, int expectedCount,
                           ...) {
    char responseBuffer[256] = {0};
    va_list expectedStrs;
    va_start(expectedStrs, expectedCount);
    int result = sendATCommand(cmdStr, timeout, responseBuffer, 256);
    int answer = 0;
    if (result != 0) {
        for (int i = 0; i < expectedCount; i++) {
            char* arg = va_arg(expectedStrs, char*);
            if (strstr(responseBuffer, arg) != NULL) {
                answer = i + 1;
                break;
            }
        }
    }
    va_end(expectedStrs);
    return answer;
}

void SIM7600::initConfig(unsigned long timeout) {
    Serial.println("Initiating Sim module");

    _simSerial->setTimeout(1000);

    int answer = 0;
    while (answer == 0) {  // Send AT every 0.5 seconds and wait for the answer
        Serial.println("Sending AT");
        answer = sendATCompare("AT", 2000, 1, "OK");
        delay(500);
    }
    Serial.println("Setting SMS Mode to txt");
    sendATCompare("AT+CMGF=1", 1000, 1, "OK");  // sets the SMS mode to text
    sendATCompare("AT+CPMS=\"MT\",\"SM\", \"ME\"", 1000, 1,
                  "OK");  // Read & store msgs in flash, write with SIM
    sendATCompare("AT+CMGD=0,2", 1000, 1,
                  "OK");  // delete already read messages

    // Check if the network has been registered, timeout after 15 seconds
    unsigned long startTime = millis();
    while (sendATCompare("AT+CREG?", 2000, 2, "+CREG: 0,1", "+CREG: 0,5") ==
               0 &&
           millis() - startTime < timeout) {
        Serial.println("Checking network registration");
        delay(500);
    }
}

bool SIM7600::sendSMS(const char* number, const char* msg) {
    char smsCmd[30];

    sendATCompare("AT+CMGF=1", 1000, 1, "OK");  // sets the SMS mode to text
    snprintf(smsCmd, sizeof(smsCmd), "AT+CMGS=\"%s\"", number);
    bool result =
        sendATCompare(smsCmd, 3000, 1, "> ");  // sets the SMS mode to text
    char buffer[32] = {0};
    if (result) {
        _simSerial->print(msg);
        return sendATCommand("\x1A", 20000, buffer, sizeof(buffer));
    }
    return result;
}

bool SIM7600::readSMS(int index, SMSStruct& smsData) {
    char buffer[256] = {0};
    char metadata[96] = {0};
    char cmd[10] = {0};
    sprintf(cmd, "AT+CMGR=%d", index);  // read and delete message at index
    if (!sendATCommand(cmd, 1000, buffer, sizeof buffer)) return false;
    char* context1;
    char* context2;
    char* bufferToken = strtok_r(buffer, "\n", &context1);
    bufferToken = strtok_r(NULL, "\n", &context1);

    strncpy(metadata, bufferToken, sizeof(metadata));
    char* msgDataToken = strtok_r(metadata, ",", &context2);
    msgDataToken = strtok_r(NULL, ",", &context2);
    strncpy(smsData.number, msgDataToken + 1, sizeof(smsData.number));
    size_t len = strlen(smsData.number);
    smsData.number[len - 1] = 0;

    msgDataToken = strtok_r(NULL, ",", &context2);
    msgDataToken = strtok_r(NULL, "\"\n", &context2);
    strncpy(smsData.timeStr, msgDataToken, sizeof(smsData.timeStr));

    bufferToken = strtok_r(NULL, "\n", &context1);
    strncpy(smsData.message, bufferToken, sizeof(smsData.message));

    return true;
}

SIM7600::GPSStruct SIM7600::getGPSLocation(unsigned long timeout) {
    int i = 0;
    char responseBuffer[256] = {0};
    sendATCompare("AT+CGPS=1,1", 1000, 1, "OK");
    unsigned long startTime = millis();
    while ((millis() - startTime) < timeout) {
        bool answer = sendATCommand("AT+CGPSINFO", 1000, responseBuffer, 256);
        if (!answer) {
            Serial.println("Error occured");
            return (GPSStruct){0, 0, false};
        }
        answer = false;
        if (char* location = strstr(responseBuffer, "+CGPSINFO: ") + 11) {
            memmove(responseBuffer, location,
                    strlen(responseBuffer) - (location - responseBuffer) + 1);
        }
        if (strstr(responseBuffer, ",,,,,,,,") != NULL) {
            // Reset buffer and redo
            memset(responseBuffer, '\0', sizeof(responseBuffer));
            i = 0;
            answer = 0;
        } else {
            sendATCompare("AT+CGPS=0", 1000, 1, "OK:");
            break;
        }
        delay(1000);
    }
    if ((millis() - startTime) > timeout) {
        return (GPSStruct){0, 0, false};
    }

    return formatGPS(responseBuffer);
}

SIM7600::GPSStruct SIM7600::formatGPS(char* GPSBuffer) {
    GPSStruct gpsData = {};
    char* token;
    token = strtok(GPSBuffer, ",");
    gpsData.latitude = atof(token);
    gpsData.latitude =
        ((int)gpsData.latitude / 100) + (fmod(gpsData.latitude, 100) / 60);

    token = strtok(NULL, ",");
    if (strcmp(token, "S") == 0) gpsData.latitude = gpsData.latitude * -1;

    token = strtok(NULL, ",");
    gpsData.longitude = atof(token);
    gpsData.longitude =
        ((int)gpsData.longitude / 100) + (fmod(gpsData.longitude, 100) / 60);

    token = strtok(NULL, ",");
    if (strcmp(token, "W") == 0) gpsData.longitude = gpsData.longitude * -1;

    gpsData.status = true;
    return gpsData;
}

void SIM7600::sendTTS(const char* message) {
    char cmd[256] = "";
    sprintf(cmd, "AT+CTTS=2,\"%s\"", message);
    sendImmediate(cmd);
}

void SIM7600::stopTTS() { sendImmediate("AT+CTTS=0"); }
