/*
 * Copyright (C) 2016 Orange - Franck Roudet
 *
 * This software is distributed under the terms and conditions of the 'Apache-2.0'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'http://www.apache.org/licenses/LICENSE-2.0'.
 */

/* A simple LoRa Message Manager
 *
 * Version:     0.1.0
 * Created:     2015-12-15 by Franck Roudet
 */
 
#include "loraMsgManager.h"

#ifdef ARDUINO
/** Comment for no debug - if debug need to define serialLog in main */
#include <SoftwareSerial.h>
extern SoftwareSerial serialLog; //(10, 11); // RX, TX
#define LOG_DEBUG(trace) serialLog.print(trace)
/* end of debug zone */
#endif


LoraMsgManager& LoraMsgManager::getInstance() {
    static  LoraMsgManager  instance;
    return instance;
}
            
static bool initBoard() { // Init Semtech Hardware
#ifdef __MBED__
    LOG_DEBUG(( "\n\n\r initBoard " __TIMESTAMP__ "\n\r"));
    BoardInitMcu( );
    BoardInitPeriph( );
#endif
    return false;
};

/*!
 * Indicates if the MAC layer has already joined a network.
 */
bool LoraMsgManager::isNetworkJoined = initBoard();


    /**
     * Set serial port when needed
     */
SERIALPORT* LoraMsgManager::setSerial(SERIALPORT* newSerial) {
  SERIALPORT* previous = serial;
  serial = newSerial;
  return previous;
}
     
/*!
 * Defines the join request timer
 */
 
#if( OVER_THE_AIR_ACTIVATION != 0 )
Ticker LoraMsgManager::joinReqTimer;
#endif

    
    /*!
     * process rx frame and call specified callbacks
     */

#ifdef __MBED__
void LoraMsgManager::processRxFrame( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info ) {
    LOG_DEBUG(( "[Rx] Port=%d\n\r" , info->RxPort));
    ProcessRxFramePortCallback *currentPortCallback = portCallBack;
    while (currentPortCallback->callback && currentPortCallback->port != info->RxPort)
        currentPortCallback++;
    if (currentPortCallback->callback) { // Found a callback
        (currentPortCallback->callback)(flags, info);
    }
}
#endif
#ifdef ARDUINO
void LoraMsgManager::processRxFrame( ) { }

void purgeSerial(SERIALPORT* serial) {
  if (serial) {
    LOG_DEBUG((F("purgeSerial: ")));
    uint8_t c;
    while (serial->available()) {
      c=serial->read();
      LOG_DEBUG(((char) c));         
    }
    LOG_DEBUG(("\n\r"));   
  }
}
#endif

#ifdef __MBED__
bool LoraMsgManager::SendFrame( void ) {
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
    static LoraMsgManager& loraManager=LoraMsgManager::getInstance();

    if( flags->Bits.JoinAccept == 1 ) {
#if( OVER_THE_AIR_ACTIVATION != 0 )
        loraManager.joinReqTimer.detach( );
#endif
        LoraMsgManager::isNetworkJoined = true;
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
    loraManager.sendState=LoraMsgManager::READYTOSEND;
}

#endif


LoraMsgManager::LoraMsgManager() : trySendingFrameAgain(false), txNextPacket(true), txDone(false), appPort(3), portCallBack(NULL),
    sendState(IDLE), softwarePort(false), serial(0l) {
    //memset(appSKey,0,sizeof(appSKey));
    //memset(nwkSKey,0,sizeof(nwkSKey));
#ifdef __MBED__
    loRaMacEvents.MacEvent = onMacEvent;
#endif
}

LoraMsgManager::SendStatusType LoraMsgManager::getState(void) {
    return sendState;
}
int LoraMsgManager::setAppPort(int newAppPort) {
    int previous=this->appPort;
    this->appPort=newAppPort;

#ifdef ARDUINO
  if (serial) {
    serial->print("+++");          // Enter command mode
    delay(250);
    purgeSerial(serial);
    serial->print("ATO082=");
    serial->print(this->appPort, HEX);
    serial->print("\n");
    purgeSerial(serial);
    serial->print("ATQ\n");        // Quit command mode
    purgeSerial(serial);
  }
#endif // ARDUINO


    return previous;
}


ProcessRxFramePortCallback * LoraMsgManager::setPortCallBack(ProcessRxFramePortCallback *portCallBackList) {
    ProcessRxFramePortCallback * previous=this->portCallBack;
    this->portCallBack=portCallBackList;
    return previous;
}

void LoraMsgManager::setNwkIds(uint8_t *newDevEui, uint32_t newDevAddr, uint8_t *newNwkSKey, uint8_t *newAppSKey) {
    this->devEui=newDevEui;
    this->devAddr=newDevAddr;

#ifdef __MBED__
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

#ifdef __MBED__
    txNextPacket = true;

    LoRaMacSetAdrOn( true );
    
    LoRaMacSetDutyCycleOn( true );
#endif
    sendState= READYTOSEND; 
}

#ifdef __MBED__
bool LoraMsgManager::sendMessage(uint8_t * appData, uint8_t appDataSize) {
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
void LoraMsgManager::monitor(void) {
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

bool LoraMsgManager::setSoftwarePort(bool state) {
  bool previous=softwarePort;
  softwarePort=state;
  return previous;
}

#ifdef ARDUINO
bool LoraMsgManager::sendMessage(uint8_t * appData, uint8_t appDataSize) {
  LOG_DEBUG(("Send message\n\r"));
  if (serial) { // Serial is set
      for (;appDataSize ;appDataSize--) {
        serial->write(*appData++);
      }
    return true;
  } else {
    return false;  
  }
}


void LoraMsgManager::monitor(void) {
  if (serial && serial->available()) {
    LOG_DEBUG(("Receive message "));
    size_t bytesAvailable = min(serial->available(), LORA_MSGBUFFERSIZE);
    serial->readBytes(receivedMsg, bytesAvailable);
    LOG_DEBUG((bytesAvailable));
    LOG_DEBUG((" bytes \n\r"));
    
    // Dispatch message according to (soft)Port
    if (softwarePort) {
      ProcessRxFramePortCallback *currentPortCallback = portCallBack;
      while (currentPortCallback->callback && currentPortCallback->port != receivedMsg[0])
          currentPortCallback++;
      if (currentPortCallback->callback) { // Found a callback
          (currentPortCallback->callback)(receivedMsg+1);
      }
    }
  }
  
}
#endif

    
