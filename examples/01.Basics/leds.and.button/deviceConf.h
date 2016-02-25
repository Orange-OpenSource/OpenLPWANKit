/*
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

/*!
 * Mote device IEEE EUI
 */
static uint8_t DevEui[] =
{
    0x01, 0x20, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22                  /// <<< Change here
};

/*!
 * Device address
 */
static uint32_t DevAddr = 0x11223301;                               /// <<< Change here

/*!
 * AES encryption/decryption cipher network session key
 */
static uint8_t NwkSKey[] = 
{ 
    0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde,                 /// <<< Change here
    0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0x01
};

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t AppSKey[] = 
{ 
    0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef,                 /// <<< Change here
    0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef
};



