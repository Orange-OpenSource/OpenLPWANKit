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
 
#ifndef lpwanMsgManager_h
#define lpwanMsgManager_h

#ifdef __MBED__
#include "mbed.h"
#if defined(MOD_SX1276)
#   include "board.h"
#   include "LoRaMac.h"
#   include "utilities.h"
#   define LOG_DEBUG(trace) debug trace
//#   define LOG_DEBUG(trace)
#else
#   define LOG_DEBUG(trace) printf trace
#   define LOG_DEBUG(trace)
//#define LOG_DEBUG(trace) pc.printf trace
#endif
#define F(info) info
#define SERIALPORT Serial
#endif

#ifdef ARDUINO
#include "Arduino.h"
#define LOG_DEBUG(a)
#define SERIALPORT Stream
#endif

/*!
 * receiving frame Callback function type 
 */
typedef void (*ProcessRxFrameCallback) (
#if defined(__MBED__) && defined(MOD_SX1276)
  LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info
#endif
#if defined(ARDUINO) || ! defined(MOD_SX1276)
  uint8_t *info, uint8_t size
#endif
);

/**
 * Use for call part on (soft)port
 * Last element of call back list must be {-1, NULL}.
 * Can set a default call back if not port match with ProcessRxFramePortCallback::PORTCALLBACK_DEFAULT
 * Ex:
 * ProcessRxFramePortCallback lpwanPortCallBack[]= {
 *   {20, onSetLed1},
 *   {30, onSetLed2},
 *   {ProcessRxFramePortCallback::PORTCALLBACK_DEFAULT, onError},
 *   {-1, NULL} // this is mandatory
 * };
 */
struct ProcessRxFramePortCallback {
    int port;
    ProcessRxFrameCallback callback;
    static const int PORTCALLBACK_DEFAULT=256;
};

class LpwanMsgManager {

public:
    
    // internal status
    enum SendStatusType { IDLE, READYTOSEND, SENDINPROGRESS, SENT };

    SendStatusType getState(void);
    
    static LpwanMsgManager& getInstance();
    

    uint8_t appPort;

#if defined(__MBED__) && defined(MOD_SX1276)
    /*!
     * Indicates if the MAC layer has already joined a network.
     */
    static bool isNetworkJoined;
    
    bool trySendingFrameAgain;

#if( OVER_THE_AIR_ACTIVATION != 0 )
    /*!
     * Defines the join request timer
     */
    static Ticker joinReqTimer;
    
    uint8_t sendFrameStatus = 0;
#endif

    /*!
     * Indicates if a new packet can be sent
     */
    bool txNextPacket;
    bool txDone;

    LoRaMacEvent_t loRaMacEvents;
    friend void onMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info );
#endif

    /*!
     * Device IEEE EUI
     */
    uint8_t *devEui;
    
    /*!
     * Mote Address
    */
    uint32_t devAddr;

    /*!
     * Application IEEE EUI
     */
    uint8_t *appEui;
    
    /*!
     * AES encryption/decryption cipher application session key
     */
    uint8_t appSKey[16];
    
    /*!
     * AES encryption/decryption cipher network session key
     */
    uint8_t nwkSKey[16];

    
    
    /*!
     * Set next data to sent.
     */
    bool sendMessage(uint8_t * appData, uint8_t appDataSize);
    
    ProcessRxFramePortCallback * setPortCallBack(ProcessRxFramePortCallback *portCallBackList);
    void setNwkIds(uint8_t *devEui, uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey);
    
    /*!
     * Send message and check incoming message
     */
    virtual void monitor(void);
    
    int setAppPort(int appPort);

    /**
     * Set serial port when needed
     */
     SERIALPORT* setSerial(SERIALPORT* newSerial);

    /**
     * enable software port.
     * First byte of the message 
     */
     bool setSoftwarePort(bool state);


private:

    bool softwarePort;
    SERIALPORT* serial;
    SendStatusType sendState;
    LpwanMsgManager();

    /*!
     * Current Data to sent, sent when is TxNextPacket is true
     * Client must not modified until sent
     */
    uint8_t *appData;
    /*!
     * Current Data size to sent
     * Client must not modified until sent
     */
    uint8_t appDataSize;
    
    ProcessRxFramePortCallback *portCallBack;
    ProcessRxFrameCallback      defaultCallBack;
    
#if defined(__MBED__) && defined(MOD_SX1276)
    /*!
     * process rx frame and call spécified callbacks
     */
    void processRxFrame(
    LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info
    );
    bool SendFrame( void );
#endif

#if defined(ARDUINO) || ! defined(MOD_SX1276)
#define LPWAN_MSGBUFFERSIZE 100

    uint8_t receivedMsg[LPWAN_MSGBUFFERSIZE];
#endif
};

#endif
