/*
 * Copyright (C) 2016 Franck Roudet
 *
 * This software is distributed under the terms and conditions of the 'Apache-2.0'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'http://www.apache.org/licenses/LICENSE-2.0'.
 */

#ifndef ledPinout_H_
#define ledPinout_H_
#include "checkChangeMixin.h"

#ifdef __MBED__
#include "mbed.h"
#define PINNAMETYPE PinName
#define HIGH 1
#define LOW 0
#define SETPINSTATE(state) devPin=state
#define ledmillis() (us_ticker_read()/1000) /*ledTimer.read_ms()*/
#endif

#ifdef ARDUINO
#include "Arduino.h"
#define PINNAMETYPE char
#define SETPINSTATE(state) digitalWrite(pin,state)
#define ledmillis() millis()
#endif

class MyLed : public CheckChangeMixin {
public:
    PINNAMETYPE pin;
    bool state;
    unsigned long blinkperiod;
    unsigned long lastblink;
    bool blinkmode;
#ifdef __MBED__
    DigitalOut devPin;
#endif
    /**
     * Contructors
     */
    MyLed(const PINNAMETYPE pin, unsigned long blinkperiodms=0) : pin(pin) , blinkperiod(blinkperiodms), blinkmode(blinkperiodms!=0)
#ifdef __MBED__
        ,devPin(pin)
#endif
    {
#ifndef __MBED__
        pinMode(pin, OUTPUT);
#endif
    };

    virtual void startHW(void);
    virtual void on(void) {
        state = HIGH;
        blinkmode=false;
        SETPINSTATE(state);
    };
    virtual void off(void) {
        state=LOW;
        blinkmode=false;
        SETPINSTATE(state);
    };

    virtual void blink(void) {
        if (blinkperiod==0) {
            state = HIGH-state;
            SETPINSTATE(state);
        } else {
            blinkmode=true;
        }
    }
    /**
     * check hardware changes
     */
    virtual void checkChange(void) {
        if (blinkmode) {
            if (ledmillis() > lastblink + blinkperiod) {
                lastblink=ledmillis();
                state = HIGH-state;
                SETPINSTATE(state);
            }

        }
        return;
    }

};

#endif
