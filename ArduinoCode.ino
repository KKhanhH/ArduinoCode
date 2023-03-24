#include <math.h>
#include <EEPROM.h>
#include "src/SimCom/SimCom.h"
#include "src/Keypad/Keypad.h"
#include "src/MicrowaveControl/MicrowaveControl.h"
#include "src/PresetFoods/PresetFoods.h"


#define KEYPAD_COL_START 22
#define KEYPAD_ROW_START 28

#define INH_ROW_8 34
#define INH_ROW_9 35
#define INH_ROW_10 36
#define INH_ROW_11 37

#define CH_SELECTOR_0 40
#define CH_SELECTOR_1 41
#define CH_SELECTOR_2 42


bool onCall = false;
bool isMicrowaving = false;
char pinCode[5] = "";

SIM7600 simModule(Serial1);

Keypad keypad = Keypad(KEYPAD_ROW_START, KEYPAD_COL_START);

MicrowaveControl mcu = MicrowaveControl(INH_ROW_8, INH_ROW_9, INH_ROW_10, INH_ROW_11,
    CH_SELECTOR_0, CH_SELECTOR_1, CH_SELECTOR_2);

bool setPin(char *pinStr) {
    int i = 0;
    for(i = 0; pinStr[i] != '\0' && i < 4; ++i) {
        Serial.println(pinStr[i]);    
        if(i >= 4 || pinStr[i] < '0' || pinStr[i] > '9') {
            return false;
        }
    }
    for(i = 0; i < 4; ++i) {
        EEPROM.update(i,pinStr[i]);
    }
    return true;
}

void getPin() {
    for(int i = 0; i < 4; ++i) {
        pinCode[i] = EEPROM.read(i);
    }
}

// Unused function for texting the location of the microwave with a Google Maps link
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

    getPin();

    // Set up TTS settings and play welcome message
    simModule.sendATCompare("AT+CDTAM=1", 500, 0);
    simModule.sendATCompare("AT+CTTSPARAM=2,3,0,1,2", 500, 0);
    simModule.sendTTS("Please enter pin code");
    onCall = true;
}

void handleCall(char* dataBuffer, int bufferSize) {
    char* strPtr = dataBuffer;
    static bool isUnlocked = false;
    static int lockIndex = 0;
    // Check if call has ended
    if (strstr(dataBuffer, "VOICE CALL: END:") ||
        strstr(dataBuffer, "NO CARRIER")) {
        Serial.print("Call Ended");
        onCall = false;
        lockIndex = 0;
        isUnlocked = false;
    }

    // Search for DTMF data
    strPtr = strstr(strPtr, "+RXDTMF: ");
    while (strPtr) {
        char keyPressed = strPtr[9];
        if(isUnlocked == false) {
            if(keyPressed == '*') {
                lockIndex = 0;
            } else if(keyPressed != pinCode[lockIndex]) {
                onCall = false;
                lockIndex = 0;
                // Hang up call
                simModule.sendATCompare("AT+CHUP", 500, 0);
            } else {
              ++lockIndex;
              if(lockIndex == 4) {
                simModule.sendTTS("Welcome to the Phone Micro wave");
                isUnlocked = true;
                lockIndex = 0;
              }             
            }
        } else {
            // Match key pressed to microwave button 
            Keypad::readPin btnStruct = keypad.dtmfLookup(keyPressed);

            // Simulate the microwave button press
            mcu.simulateButton(btnStruct.rowPin, btnStruct.colPin);
        }
        // Scan for next DTMF key press
        strPtr++;
        strPtr = strstr(strPtr, "+RXDTMF: ");

    }
}

void powerLvlSms(int powerLevel, char *responseBuffer, size_t len) {
    Keypad::readPin button;
    bool success = true;
    char *response;
    switch(powerLevel) {
        case 1:
            response = "SET POWER LEVEL TO LOW. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            button = Keypad::BTN_LOW;
            break;
        case 2:
            response = "SET POWER LEVEL TO MEDIUM. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            button = Keypad::BTN_MED_LOW_DEFROST;
            break;
        case 3:
            response = "SET POWER LEVEL TO MEDIUM. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            button = Keypad::BTN_MEDIUM;
            break;
        case 4:
            response = "SET POWER LEVEL TO MEDIUM HIGH. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            button = Keypad::BTN_MED_HIGH;
            break;
        case 5:
            response = "SET POWER LEVEL TO HIGH. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            button = Keypad::BTN_HIGH;
            break;
        default:
            response = "\"POWER <LEVEL>\", WHERE <LEVEL> IS A NUMBER FROM 1-5, WHERE 5 IS HIGHEST.";
            success = false;
    }
    strncpy(responseBuffer, response, len);
    if(success) {
        Serial.print(button.rowPin);
        Serial.print(button.colPin);
        mcu.simulateButton(button.rowPin, button.colPin);
    }
}

void defrostSms(int defrostOption, char *responseBuffer, size_t len) {
    bool success = true;
    char *response;
    switch(defrostOption) {
        case 1:
            response = "DEFROSTING 1LB GROUND MEAT. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 2:
            response = "DEFROSTING 2LB PORK CHOP. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 3:
            response = "DEFROSTING 2LB STEAKS. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 4:
            response = "DEFROSTING 2LB CHICKEN PIECES. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 5:
            response = "DEFROSTING 3LB WHOLE CHICKEN. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        default:
            response = "\"DEFROST <OPT>\", 1: 1LB GROUND MEAT, 2: 2LB PORK CHOP, 3: 2LB STEAKS, 4:2LB CHICKEN PIECES, 5: 3LB WHOLE CHICKEN.";
            success = false;
    }
    strncpy(responseBuffer, response, len);
    if(success) {
        Keypad::readPin button = keypad.dtmfLookup('0'+defrostOption);
        mcu.simulateButton(Keypad::BTN_EASY_DEFROST.rowPin, Keypad::BTN_EASY_DEFROST.colPin);
        mcu.simulateButton(button.rowPin, button.colPin);
    }
}

void reheatSms(int reheatOption, char *responseBuffer, size_t len) {
    bool success = true;
    char *response;
    switch(reheatOption) {
        case 1:
            response = "REHEATING 1CUP CASSEROLE. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 2:
            response = "REHEATING 1 DINNER PLATE. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 3:
            response = "REHEATING 10-12oz FROZEN ENTREE. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 4:
            response = "REHEATING 1CUP SOUP. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        case 5:
            response = "REHEATING 1CUP VEGETABLES. TYPE START TO CONFIRM, CANCEL OTHERWISE.";
            break;
        default:
            response = "\"REHEAT <OPT>\", 1: 1CUP CASSEROLE, 2: 1 DINNER PLATE, 3: 10-12oz FROZEN ENTREE, 4: 1CUP SOUP, 5: 1CUP VEGETABLES.";
            success = false;
    }
    strncpy(responseBuffer, response, len);
    if(success) {
        Keypad::readPin button = keypad.dtmfLookup('0'+reheatOption);
        mcu.simulateButton(Keypad::BTN_EASY_REHEAT.rowPin, Keypad::BTN_EASY_REHEAT.colPin);
        mcu.simulateButton(button.rowPin, button.colPin);
    }
}

void handleSMS(SIM7600::SMSStruct smsInput) {

    char messageCpy[200] = "";
    strncpy(messageCpy, smsInput.message, sizeof(smsInput.message));

    char *token = strtok(messageCpy, " ");
    char response[180] = "";
    char *postConvert = NULL;
    if(strncmp(token , "PIN", 3) == 0) {
        token = strtok(NULL, " ");
        if(token == NULL || setPin(token) == false) {
            strncpy(response,"\"PIN <4 DIGIT CODE>\"", 180);
        } else {
            strncpy(response,"NEW PIN CODE SET", 180);
        }
    } else if(strncmp(token , "POWER", 5) == 0) {
        token = strtok(NULL, " ");
        int powerVal = (token == NULL)? 0: strtol(token, &postConvert, 10);
        powerLvlSms(powerVal, response, 180);
    } else if(strncmp(token , "DEFROST", 7) == 0) {
        token = strtok(NULL, " ");
        int defrostVal = (token == NULL)? 0: strtol(token, &postConvert, 10);
        defrostSms(defrostVal, response, 180);
    } else if(strncmp(token , "REHEAT", 6) == 0) {
        token = strtok(NULL, " ");
        int reheatVal = (token == NULL)? 0: strtol(token, &postConvert, 10);
        reheatSms(reheatVal, response, 180);
    } else if(strncmp(token , "PRESET", 6) == 0) {
        token = strtok(NULL, " ");
        int presetVal = (token == NULL)? 0: strtol(token, &postConvert, 10);
        handlePresetFood(mcu, presetVal, response, 180);
    } else if(strncmp(token , "CANCEL", 6) == 0) {
        mcu.simulateButton(Keypad::BTN_STOP_CANCEL.rowPin, Keypad::BTN_STOP_CANCEL.colPin);
        mcu.simulateButton(Keypad::BTN_STOP_CANCEL.rowPin, Keypad::BTN_STOP_CANCEL.colPin);
        strncpy(response,"CANCELLING", 180);
    } else if(strncmp(token , "START", 5) == 0) {
        mcu.simulateButton(Keypad::BTN_START.rowPin, Keypad::BTN_START.colPin);
        strncpy(response,"STARTING OPERATION.", 180);
    } else {
        strncpy(response,"AVAILABLE COMMANDS:\nPOWER\nDEFROST\nREHEAT\nPRESET", 180);
    }
    simModule.sendSMS(smsInput.number, response);

}

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing");
    Serial1.begin(9600);
    keypad.initializePins();
    mcu.initializePins();
    delay(500);
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
            handleSMS(smsData);
        }
        if (onCall == false && (index = strstr(dataBuffer, "RING")) != NULL) {
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