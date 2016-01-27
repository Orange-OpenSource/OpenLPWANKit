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
 
#ifndef loraMsgManager_h
#define loraMsgManager_h

#ifdef __MBED__
#include "mbed.h"
#include "board.h"
#include "LoRaMac.h"
#include "utilities.h"
#define LOG_DEBUG(trace) debug trace
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
#ifdef __MBED__
  LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info
#endif
#ifdef ARDUINO
  uint8_t *info
#endif
);

struct ProcessRxFramePortCallback {
    int port;
    ProcessRxFrameCallback callback;
};

class LoraMsgManager {

public:
    
    // internal status
    enum SendStatusType { IDLE, READYTOSEND, SENDINPROGRESS, SENT };

    SendStatusType getState(void);
    
    static LoraMsgManager& getInstance();
    
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

    uint8_t appPort;

#ifdef __MBED__
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
    LoraMsgManager();

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
    
    /*!
     * process rx frame and call spécified callbacks
     */
    void processRxFrame(
#ifdef __MBED__
    LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info
#endif
    );

#ifdef __MBED__
    bool SendFrame( void );
#endif

#ifdef ARDUINO
#define LORA_MSGBUFFERSIZE 100

    uint8_t receivedMsg[LORA_MSGBUFFERSIZE];
#endif
};

#endif
