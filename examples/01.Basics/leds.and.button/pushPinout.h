/*
 * Copyright (C) 2016 - Franck Roudet
 *
 * This software is distributed under the terms and conditions of the 'Apache-2.0'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'http://www.apache.org/licenses/LICENSE-2.0'.
 */

#ifndef pushPinout_H_
#define pushPinout_H_
#include "checkChangeMixin.h"

#ifdef __MBED__
#include "mbed.h"
#define PINNAMETYPE PinName
#define HIGH 1
#define LOW 0
#define GETPINSTATE(state) state=devPin
#define ledmillis() (us_ticker_read()/1000) /*ledTimer.read_ms()*/
#endif

#ifdef ARDUINO
#include "Arduino.h"
#define PINNAMETYPE char
#define GETPINSTATE(state) state = digitalRead(pin)
#define ledmillis() millis()
#endif

class MyPush : public CheckChangeMixin
{
public:
    PINNAMETYPE pin;
    bool state;
#ifdef __MBED__
    DigitalIn devPin;
#endif
    /**
     * Contructors
     */
    MyPush(const PINNAMETYPE pin) : pin(pin), state(false)
#ifdef __MBED__
        ,devPin(pin)
#endif
    {
#ifndef __MBED__
        pinMode(pin, OUTPUT);
#endif
    };

    virtual void startHW(void);

  /**
   * check hardware changes
   */
  virtual void checkChange(void) {
    char actualValue;
    GETPINSTATE(actualValue);
    if (actualValue!=state) {
       state=actualValue;
    }
  }

};


#endif
