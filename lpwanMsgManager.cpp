/*
 * Copyright (C) 2016 Orange - Franck Roudet
 *
 * This software is distributed under the terms and conditions of the 'Apache-2.0'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'http://www.apache.org/licenses/LICENSE-2.0'.
 */

/* A simple lpwan Message Manager
 *
 * Version:     0.1.0
 * Created:     2015-12-15 by Franck Roudet
 */
 
#include "lpwanMsgManager.h"

#ifdef __MBED__
#define BYTEAVAILLABLE(comms) (comms)->readable()
#define READONECHAR(comms)    (comms)->getc()
#define PRINTONECHAR(comms,c) (comms)->putc(c)
#define PRINTSTRING(comms,s)  (comms)->puts(s)
#define PRINTHEX(comms,s)     (comms)->printf("%02X", s);
#define WAIT_MS(ms)           wait_ms(ms)
#define millis()              (us_ticker_read()/1000)
#endif
#ifdef ARDUINO
#define BYTEAVAILLABLE(comms) (comms)->available()
#define READONECHAR(comms)    (comms)->read()
#define PRINTONECHAR(comms,c) (comms)->print(c)
#define WRITEONECHAR(comms,c) (comms)->write(c)
#define PRINTSTRING(comms,s)  (comms)->print(s)
#define PRINTHEX(comms,s)     (comms)->print(s, HEX);
#define WAIT_MS(ms)           delay(ms)
#endif


#if defined(ARDUINO)
/** Comment for no debug - if debug need to define serialLog in main */
#include <SoftwareSerial.h>
extern SoftwareSerial serialLog; //(10, 11); // RX, TX
#define LOG_DEBUG(trace) serialLog.print trace
/* end of debug zone */
#endif



LpwanMsgManager& LpwanMsgManager::getInstance() {
    static  LpwanMsgManager  instance;
    return instance;
}
            
static bool initBoard() { // Init Semtech Hardware
#if defined(__MBED__) && defined(MOD_SX1276)
    LOG_DEBUG(( "\n\n\r initBoard " __TIMESTAMP__ "\n\r"));
    BoardInitMcu( );
    BoardInitPeriph( );
#endif
    return false;
};

#if defined(__MBED__) && defined(MOD_SX1276)
/*!
 * Indicates if the MAC layer has already joined a network.
 */
bool LpwanMsgManager::isNetworkJoined = initBoard();
     
/*!
 * Defines the join request timer
 */
 
#if( OVER_THE_AIR_ACTIVATION != 0 )
Ticker LpwanMsgManager::joinReqTimer;
#endif

    
    /*!
     * process rx frame and call specified callbacks
     */

void LpwanMsgManager::processRxFrame( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info ) {
    LOG_DEBUG(( "[Rx] Port=%d\n\r" , info->RxPort));
    ProcessRxFramePortCallback *currentPortCallback = portCallBack;
    while (currentPortCallback->callback && currentPortCallback->port != info->RxPort)
        currentPortCallback++;
    if (currentPortCallback->callback) { // Found a callback
        (currentPortCallback->callback)(flags, info);
    } else {
        if (this->defaultCallBack) {
            (this->defaultCallBack)(flags, info);
        }
    }
}
#endif


    /**
     * Set serial port when needed
     */
SERIALPORT* LpwanMsgManager::setSerial(SERIALPORT* newSerial) {
  SERIALPORT* previous = this->serial;
  this->serial = newSerial;
  return previous;
}



#if defined(ARDUINO)
void purgeSerial(SERIALPORT* theSerial) {
    LOG_DEBUG((F("purgeSerial: ")));
    if (theSerial) {
        uint8_t c;
        while (BYTEAVAILLABLE(theSerial)) {
            c=READONECHAR(theSerial);
            LOG_DEBUG(((char) c));
        }
        LOG_DEBUG(("\n\r"));
  } else {
      LOG_DEBUG(("Serial not yet set"));
  }
}
#endif

const unsigned long MSGWINDOW=5;

#if defined(__MBED__) && ! defined(MOD_SX1276)

void purgeSerial(SERIALPORT* theSerial) {
    //LOG_DEBUG((F("purgeS: \n\r")));
    if (theSerial) {
        uint8_t c;
        unsigned long now=millis();
        unsigned long lastCharTime=now;
        while (now-lastCharTime < MSGWINDOW) {
            while (BYTEAVAILLABLE(theSerial)) {
                c=READONECHAR(theSerial);
                lastCharTime=millis();
                // printf("%c", (char) c); // uncomment to see response to AT command
            }
            now=millis();
        }
        //LOG_DEBUG(("\n\r"));
  } else {
      LOG_DEBUG(("Serial not yet set"));
  }
}
#endif

#if defined(__MBED__) && defined(MOD_SX1276)

bool LpwanMsgManager::SendFrame( void ) {
    LOG_DEBUG(("Send Frame\n\r"));
    uint8_t sendFrameStatus = 0;

    sendFrameStatus = LoRaMacSendFrame( appPort, appData, appDataSize );
 //   sendFrameStatus = LoRaMacSendConfirmedFrame( AppPort, AppData, AppDataSize, 8 );
    switch( sendFrameStatus ) {
    case 5: // NO_FREE_CHANNEL
        // Try again later
        return true;
    default:
        sendState=SENT;
        return false;
    }
}

static void onMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info ) {
    static LpwanMsgManager& loraManager=LpwanMsgManager::getInstance();

    if( flags->Bits.JoinAccept == 1 ) {
#if( OVER_THE_AIR_ACTIVATION != 0 )
        loraManager.joinReqTimer.detach( );
#endif
        LpwanMsgManager::isNetworkJoined = true;
    }
    
    if( flags->Bits.Tx == 1 ) {
    }

    if( flags->Bits.Rx == 1 ) {
        if( flags->Bits.RxData == true ) {
            loraManager.processRxFrame( flags, info );
        }
    }

    // Schedule a new transmission
    loraManager.txDone = true;
    loraManager.sendState=LpwanMsgManager::READYTOSEND;
}

#endif


LpwanMsgManager::LpwanMsgManager() :
#if defined(__MBED__) && defined(MOD_SX1276)
    trySendingFrameAgain(false), txNextPacket(true), txDone(false),
#endif
     appPort(3), portCallBack(NULL), defaultCallBack(NULL),
        sendState(IDLE), softwarePort(false), serial(0l) {
    //memset(appSKey,0,sizeof(appSKey));
    //memset(nwkSKey,0,sizeof(nwkSKey));
#if defined(__MBED__) && defined(MOD_SX1276)
    loRaMacEvents.MacEvent = onMacEvent;
#endif
}

LpwanMsgManager::SendStatusType LpwanMsgManager::getState(void) {
    return sendState;
}
int LpwanMsgManager::setAppPort(int newAppPort) {
    int previous=this->appPort;
    this->appPort=newAppPort;

#if defined(ARDUINO) || ! defined(MOD_SX1276)
  if (this->serial) {
    PRINTSTRING(this->serial, "+++");          // Enter command mode
    //WAIT_MS(50); // needed for arduino ???
    purgeSerial(this->serial);
    PRINTSTRING(this->serial, "ATO082=");
    PRINTHEX(this->serial, this->appPort);
    PRINTSTRING(this->serial, "\n");
    purgeSerial(this->serial);
    PRINTSTRING(this->serial, "ATQ\n");        // Quit command mode
    purgeSerial(this->serial);
  } else {
      LOG_DEBUG(("serial port not set\n"));
  }
#endif // ARDUINO


    return previous;
}


ProcessRxFramePortCallback * LpwanMsgManager::setPortCallBack(ProcessRxFramePortCallback *portCallBackList) {
    ProcessRxFramePortCallback * previous=this->portCallBack;
    this->portCallBack=portCallBackList;
    this->defaultCallBack=NULL;
    ProcessRxFramePortCallback *currentPortCallback = portCallBack;
    while (currentPortCallback->callback && currentPortCallback->port != ProcessRxFramePortCallback::PORTCALLBACK_DEFAULT)
          currentPortCallback++;
    if (currentPortCallback->callback) { // Found a default callback
          this->defaultCallBack=currentPortCallback->callback;
    }
    return previous;
}

void LpwanMsgManager::setNwkIds(uint8_t *newDevEui, uint32_t newDevAddr, uint8_t *newNwkSKey, uint8_t *newAppSKey) {
    this->devEui=newDevEui;
    this->devAddr=newDevAddr;

#if defined(__MBED__) && defined(MOD_SX1276)
    memcpy1(this->nwkSKey, newNwkSKey, 16);
    memcpy1(this->appSKey, newAppSKey, 16);

    LoRaMacInit( &this->loRaMacEvents );

    isNetworkJoined = false;
#endif

#if( defined(OVER_THE_AIR_ACTIVATION) && OVER_THE_AIR_ACTIVATION == 0 )
    // Random seed initialization
    srand( RAND_SEED );
    // Choose a random device address
    // NwkID = 0
    // NwkAddr rand [0, 33554431]
    if( ( this->devAddr == 0 ) || ( this->devAddr == 0xFFFFFFFF ) ) {
        // Generate random DevAddr if it does not exist
        LOG_DEBUG(("Generate random DevAddr\n\r"));
        this->devAddr = randr( 0, 0x01FFFFFF );
    }
    //LOG_DEBUG(( "- DevAddr = 0x%x\n\r" , this->devAddr));
    LoRaMacInitNwkIds( 0x000000, this->devAddr, this->nwkSKey, this->appSKey );
    isNetworkJoined = true;
#endif

#if defined(__MBED__) && defined(MOD_SX1276)
    txNextPacket = true;

    LoRaMacSetAdrOn( true );
    
    LoRaMacSetDutyCycleOn( true );
#endif
    sendState= READYTOSEND; 
}

#if defined(__MBED__) && defined(MOD_SX1276)
bool LpwanMsgManager::sendMessage(uint8_t * appData, uint8_t appDataSize) {
    if (sendState == READYTOSEND) {
        this->appData = appData;
        this->appDataSize = appDataSize;
        sendState = SENDINPROGRESS;
        return true;
    } else {
        return false;
    }
}
    
/*!
 * Send message and check incoming message
 */
void LpwanMsgManager::monitor(void) {
    while( isNetworkJoined == false ) {
#if( defined (OVER_THE_AIR_ACTIVATION) && OVER_THE_AIR_ACTIVATION != 0 )
        if( txNextPacket == true ) {
            txNextPacket = false;
            
            sendFrameStatus = LoRaMacJoinReq( devEui, appEui, appKey );
            LOG_DEBUG(("Req Sent\n\r"));
            switch( sendFrameStatus ) {
                case 1: // BUSY
                    break;
                case 0: // OK
                case 2: // NO_NETWORK_JOINED
                case 3: // LENGTH_PORT_ERROR
                case 4: // MAC_CMD_ERROR
                case 6: // DEVICE_OFF
                default:
                    // Relaunch timer for next trial
                    // Missing OnJoinReqTimerEvent function ??
                    JoinReqTimer.attach_us( &OnJoinReqTimerEvent, OVER_THE_AIR_ACTIVATION_DUTYCYCLE );
                    break;
            }
        }
//            TimerLowPowerHandler( );
#endif
    }
    
    if( sendState==SENDINPROGRESS) {       
        txNextPacket = false;
        trySendingFrameAgain = SendFrame( );
    }
}
#endif

bool LpwanMsgManager::setSoftwarePort(bool state) {
  bool previous=softwarePort;
  softwarePort=state;
  return previous;
}

#if defined(ARDUINO) || ! defined(MOD_SX1276)
bool LpwanMsgManager::sendMessage(uint8_t * appData, uint8_t appDataSize) {
  LOG_DEBUG(("Send message\n\r"));
  if (this->serial) { // Serial is set
      for (;appDataSize ;appDataSize--) {
        WRITEONECHAR(this->serial,*appData++);
      }
    return true;
  } else {
    return false;  
  }
}


void LpwanMsgManager::monitor(void) {
  if (this->serial && BYTEAVAILLABLE(this->serial)) {
    //LOG_DEBUG(("Receive message "));
    size_t bytesAvailable=0;
    unsigned long now=0;
    unsigned long lastCharTime=0;
    do {
        for (;BYTEAVAILLABLE(this->serial) && bytesAvailable < LPWAN_MSGBUFFERSIZE; bytesAvailable++) {
            receivedMsg[bytesAvailable]=(uint8_t) READONECHAR(this->serial);
            lastCharTime=millis();
        }
        now=millis();
    } while (now-lastCharTime < MSGWINDOW);
    LOG_DEBUG((bytesAvailable));
    LOG_DEBUG((" bytes \n\r"));

    //for (int byteno=0; byteno < bytesAvailable; byteno ++) {
    //    printf("%02X", receivedMsg[byteno]);
    //}

    // Dispatch message according to (soft)Port
    if (softwarePort) {
      ProcessRxFramePortCallback *currentPortCallback = portCallBack;
      while (currentPortCallback->callback && currentPortCallback->port != receivedMsg[0])
          currentPortCallback++;
      if (currentPortCallback->callback) { // Found a callback
          (currentPortCallback->callback)(receivedMsg+1, bytesAvailable-1);
      } else { // default Port
            if (this->defaultCallBack) {
                (this->defaultCallBack)(receivedMsg, bytesAvailable);
            }
      }
    }
  }
  
}
#endif

    
