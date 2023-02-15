#include <math.h>
#include "src/SimCom/SimCom.h"
#include "src/Keypad/Keypad.h"
#include "src/MicrowaveControl/MicrowaveControl.h"

bool onCall = false;
bool isMicrowaving = false;

SIM7600 simModule(Serial1);

#define KEYPAD_COL_START 22
#define KEYPAD_ROW_START 28
Keypad keypad = Keypad(KEYPAD_ROW_START, KEYPAD_COL_START);

#define INH_ROW_8 34
#define INH_ROW_9 35
#define INH_ROW_10 36
#define INH_ROW_11 37

#define CH_SELECTOR_0 40
#define CH_SELECTOR_1 41
#define CH_SELECTOR_2 42
MicrowaveControl mcu = MicrowaveControl(INH_ROW_8, INH_ROW_9, INH_ROW_10, INH_ROW_11,
    CH_SELECTOR_0, CH_SELECTOR_1, CH_SELECTOR_2);

void textGPS(const char* phone_number) {
    char latStr[16];
    char longStr[16];
    char replyStr[128];

    SIM7600::GPSStruct gpsData = simModule.getGPSLocation(45000);
    if (!gpsData.status) {
        simModule.sendSMS(phone_number, "Failed to get GPS data");
    }

    dtostrf(gpsData.latitude, 3, 8, latStr);
    dtostrf(gpsData.longitude, 4, 8, longStr);
    sprintf(replyStr,
            "https://www.google.com/maps/search/?api=1&query=%s%%2C%s", latStr,
            longStr);
    simModule.sendSMS(phone_number, replyStr);
}

void initCall(char* dataBuffer, int bufferSize) {
    char phone_number[30] = "\0";
    int number_length = 0;
    char* token;

    // Get the phone number of the phone calling
    simModule.sendATCommand("AT+CLCC", 1000, dataBuffer, bufferSize);
    token = strtok(dataBuffer, ",");
    for (int i = 0; i < 5; i++) token = strtok(NULL, ",");
    strcpy(phone_number, token + 1);

    // Null terminate the number string
    number_length = strlen(phone_number);
    phone_number[--number_length] = '\0';

    // Print phone number
    Serial.print("Call from: ");
    Serial.println(phone_number);

    // Answer Phone Call
    simModule.sendATCompare("ATA", 500, 0);

    // Set up TTS settings and play welcome message
    simModule.sendATCompare("AT+CDTAM=1", 500, 0);
    simModule.sendATCompare("AT+CTTSPARAM=2,3,0,1,2", 500, 0);
    simModule.sendTTS("Welcome To The Phone Micro Wave");
    onCall = true;
}

void handleCall(char* dataBuffer, int bufferSize) {
    char* strPtr = dataBuffer;
    // Check if call has ended
    if (strstr(dataBuffer, "VOICE CALL: END:") ||
        strstr(dataBuffer, "NO CARRIER")) {
        Serial.print("Call Ended");
        onCall = false;
    }

    // Search for DTMF data
    strPtr = strstr(strPtr, "+RXDTMF: ");
    while (strPtr) {
        char keyPressed = strPtr[9];
        // Match key pressed to microwave button 
        Keypad::readPin btnStruct = keypad.dtmfLookup(keyPressed);

        // Simulate the microwave button press
        mcu.simulateButton(btnStruct.rowPin, btnStruct.colPin);

        // Scan for next DTMF key press
        strPtr++;
        strPtr = strstr(strPtr, "+RXDTMF: ");
    }
}

// Keypad routed from pins 2-11, GND connection on pin 12
// Rows: 8-11 - Input ; Columns: 2-7 - Output

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing");
    Serial1.begin(57600);
    keypad.initializePins();
    mcu.initializePins();
    mcu.simulateButton(Keypad::BTN_STOP_CANCEL.rowPin, Keypad::BTN_STOP_CANCEL.colPin);

    simModule.initConfig(15000);
    Serial.println("READY");
    delay(500);


}

void loop() {
    static char dataBuffer[256] = {0};
    static SIM7600::SMSStruct smsData = {};
    static boolean btnPressed = false;
    char* index;
    if (simModule.readToBuffer(dataBuffer, sizeof(dataBuffer)) > 0) {
        if ((index = strstr(dataBuffer, "+CMTI: \"ME\",")) != NULL) {
            index += 12;
            smsData = {};
            simModule.readSMS(atoi(index), smsData);
            Serial.println("New SMS:");
            Serial.print(" ");
            Serial.println(smsData.number);
            Serial.print(" ");
            Serial.println(smsData.timeStr);
            Serial.print(" ");
            Serial.println(smsData.message);
        }
        i f (onCall == false && (index = strstr(dataBuffer, "RING")) != NULL) {
            index += 4;
            initCall(dataBuffer, sizeof(dataBuffer));
        }
        if (onCall == true) {
            handleCall(dataBuffer, sizeof(dataBuffer));
        }
    }
    if (onCall == false) {
        Keypad::readPin keypadRead = keypad.readKeypad();
        if (keypadRead != Keypad::BTN_UNPRESSED && !btnPressed) {
            Serial.println(keypad.buttonStr(keypadRead));
            mcu.simulateButton(keypadRead.rowPin, keypadRead.colPin);
            btnPressed = true;
        }
        else if(keypadRead == Keypad::BTN_UNPRESSED) {
            btnPressed = false;         
        }
    }
}