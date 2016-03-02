/**
 * Copyright (C) 2016 Orange - Franck Roudet
 *
 * This software is distributed under the terms and conditions of the 'Apache-2.0'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'http://www.apache.org/licenses/LICENSE-2.0'.
 */

/* Open IoT Kit for LPWAN
 *
 * Version:     0.1.0
 * Created:     2015-12-15 by Franck Roudet
 */

#include <SoftwareSerial.h>

#define _VERSION_MAJOR "1"
#define _VERSION_MINOR "0dev"
#define _VERSION_DESC "orange leds and buttons"

#include "loraMsgManager.h"


/**
 * Get LPWAN Message manager
 */
LpwanMsgManager& lpwanMsgManager = LpwanMsgManager::getInstance();

/*
 * Devices stuff
 */
#include "ledPinout.h"
#include "pushPinout.h"

class MyPushSender : public MyPush {
public:
    bool previousState;
    uint8_t statusToSend;

    MyPushSender(const PINNAMETYPE pin): MyPush(pin), previousState(false) {
    }
    /**
     * check hardware changes
     */
    virtual void checkChange(void) {
        MyPush::checkChange();
        if (previousState!=state) {
            LOG_DEBUG(("Button change state to "));
            LOG_DEBUG((state));
            LOG_DEBUG(("\n\r"));
            statusToSend=1-state; // 1 when press
            if (statusToSend) {
                char readyToSend = lpwanMsgManager.sendMessage(&statusToSend,sizeof(statusToSend));
                if (!readyToSend) {
                    LOG_DEBUG(("Not ready to send\n\r"));
                }
            }
            previousState=state;
        }
    }
};

const int GREEN      =5;               // GREEN dimmable LED
const int BLUE       =6;               // BLUE dimmable LED
const int RED        =9;               // RED dimmable LED
const int USER_BUTTON=12;


MyLed led1(BLUE, 3000);
MyLed led2(RED, 500);
MyPushSender blueButton(USER_BUTTON);


// For debug

/** Set to 1 for debug. */
#define DEBUG_MODE 1

#if DEBUG_MODE==1
SoftwareSerial serialLog(10, 11); // RX, TX
#define LOG_DEBUG(trace) serialLog.print(trace)
#endif

/**
 * Sensors/actuators list for checking hardware;
 */
CheckChangeMixin * hardwareList[]= {
    &led1,
    &led2,
    &blueButton,
    NULL    // +++ WARNING +++: must ends with NULL
}; 


/**
 * Port for emmitting message
 */
static uint8_t AppPort = 3;


/**
 * Communication part
 */

static void setLedStatus ( MyLed& led, uint8_t *info ) {
    switch (info[0]) {
        case 0:
            led.off();
            break;
        case 1:
            led.on();
            break;
        default:
            led.blink();
            break;
    }
}


/**
 * port 20
 */
static void onSetLed1 ( uint8_t *info, uint8_t size ) {
    LOG_DEBUG(( "[Rx] setled1 state to "));
    LOG_DEBUG(( info[0]));
    LOG_DEBUG(( "\n\r"));
    setLedStatus ( led1, info );
}

/**
 * port 30
 */
static void onSetLed2 ( uint8_t *info, uint8_t size  ) {
    LOG_DEBUG(( "[Rx] setled2 state to "));
    LOG_DEBUG(( info[0]));
    LOG_DEBUG(( "\n\r"));
    setLedStatus ( led2, info );
}

/**
 * default call back port 
 */
static void onDefaultMsg ( uint8_t *info, uint8_t size ) {
    LOG_DEBUG(( "[Rx] message unknown "));
    LOG_DEBUG(( info[0]));
    LOG_DEBUG(( "\n\r"));
}


/**
 * LPWAN Call back according to port (software port if necessary)
 */
ProcessRxFramePortCallback lpwanPortCallBack[]= {
    {20, onSetLed1},
    {30, onSetLed2},
    {ProcessRxFramePortCallback::PORTCALLBACK_DEFAULT, onDefaultMsg}, // On error or if no other port set
    {-1, NULL} // this is mandatory
};






void setup() {
  // put your setup code here, to run once:
#if DEBUG_MODE==1
    serialLog.begin(19200);
#endif
    
    Serial.begin(19200); // Check your module configuration and adjust serial parameter
    // Setup Communication 
    lpwanMsgManager.setSerial(&Serial);
    lpwanMsgManager.setSoftwarePort(true);
    
    lpwanMsgManager.setPortCallBack(lpwanPortCallBack);
    lpwanMsgManager.setAppPort(AppPort);

    
    // Setup hardware
    for (CheckChangeMixin **device=hardwareList; *device; device++) {
        (*device)->startHW();
    }
    
    LOG_DEBUG(( "\n\n\rLPWAN  " _VERSION_MAJOR "," _VERSION_MINOR "," _VERSION_DESC "," __TIMESTAMP__ "\n\n\r" ));

    led1.off();
    led2.blink();
}


void loop() {
  // put your main code here, to run repeatedly:
  
  // Check for Hardware changes
  for (CheckChangeMixin **device=hardwareList; *device; device++) {
      (*device)->checkChange();
  }
  
  lpwanMsgManager.monitor(); // Check communication device

}
