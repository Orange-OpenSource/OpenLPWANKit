/*
 * Orange - Franck Roudet 2016
 */

#include <SoftwareSerial.h>

#define _VERSION_MAJOR "1"
#define _VERSION_MINOR "0dev"
#define _VERSION_DESC "orange dash"

#include "loraMsgManager.h"


/**
 * Get Lora Message manager
 */
LoraMsgManager& loraMsgManager = LoraMsgManager::getInstance();

/*
 * Device stuff
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
                char readyToSend = loraMsgManager.sendMessage(&statusToSend,sizeof(statusToSend));
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
SoftwareSerial serialLog(10, 11); // RX, TX
#define LOG_DEBUG(trace) serialLog.print(trace)


/**
 * Sensor/actuator list for checking Hardware;
 */
CheckChangeMixin * hardwareList[]= {
    &led1,
    &led2,
    &blueButton,
    NULL    // +++ WARNING +++: must ends with NULL
}; 

/*! 
 * Include Configuration // for MBED devices
 */
#include "deviceConf.h"

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
static void onSetLed1 ( uint8_t *info ) {
    LOG_DEBUG(( "[Rx] setled1 state to "));
    LOG_DEBUG(( info[0]));
    LOG_DEBUG(( "\n\r"));
    setLedStatus ( led1, info );
}

/**
 * port 30
 */
static void onSetLed2 ( uint8_t *info ) {
    LOG_DEBUG(( "[Rx] setled2 state to "));
    LOG_DEBUG(( info[0]));
    LOG_DEBUG(( "\n\r"));
    setLedStatus ( led2, info );
}


/**
 * Lora Call back according to port (software port if necessary)
 */
ProcessRxFramePortCallback loraPortCallBack[]= {
    {20, onSetLed1},
    {30, onSetLed2},
    {-1, NULL} // this is mandatory
};






void setup() {
  // put your setup code here, to run once:

    serialLog.begin(19200);
    
    Serial.begin(19200);
    // Setup Communication 
    loraMsgManager.setSerial(&Serial);
    loraMsgManager.setSoftwarePort(true);
    
    loraMsgManager.setPortCallBack(loraPortCallBack);
    loraMsgManager.setNwkIds(DevEui, DevAddr, NwkSKey, AppSKey);
    loraMsgManager.setAppPort(AppPort);

    
    // Setup hw
    for (CheckChangeMixin **device=hardwareList; *device; device++) {
        (*device)->startHW();
    }
    
    LOG_DEBUG(( "\n\n\rLoRaWAN Orange Dash " _VERSION_MAJOR "," _VERSION_MINOR "," _VERSION_DESC "," __TIMESTAMP__ "\n\n\r" ));

    led1.off();
    led2.blink();
}


void loop() {
  // put your main code here, to run repeatedly:
  
  // Check for Hardware changes
  for (CheckChangeMixin **device=hardwareList; *device; device++) {
      (*device)->checkChange();
  }
  
  loraMsgManager.monitor(); // Check communication device

}
