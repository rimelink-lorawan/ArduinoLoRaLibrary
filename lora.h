/**
 * \file
 *         lora.h
 * \description
 *         LoRa library for Arduino by RimeLink (www.rimelink.com)
 * \date
 *         2019-10-27 09:44 by Peter
 *         2021-07-06 13:28 by John
 * \copyright
 *         Revised BSD License, see section \ref LICENSE
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_H__
#define __LORA_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define LORA_LIB_VERSION "1.0.00"

/* Exported types ------------------------------------------------------------*/
class lora
{
public:
    int16_t available(void);
    int16_t read(void *saveBuf, int16_t maxSize);
    int16_t read(void *saveBuf, int16_t maxSize, int16_t *rxRSSI, int8_t *rxSNR);
    int16_t write(const void *srcBuf, int16_t srcSize);
    int16_t write(const void *srcBuf, int16_t srcSize, bool isConfirmed, uint8_t byFPort);
    int16_t write(const void *srcBuf, int16_t srcSize, bool isConfirmed, uint8_t byFPort, char **saveStatus);
    int16_t write(const void *srcBuf, int16_t srcSize, bool isConfirmed, uint8_t byFPort, char **saveStatus, uint16_t msTimeOnAir);

private:
    uint8_t calc_cs(const void *srcBuf, int16_t srcSize);
};

#endif

/*--------------------------------------------------------------------------
                                                                       0ooo
                                                           ooo0     (   )
                                                            (   )      ) /
                                                             \ (      (_/
                                                              \_)
----------------------------------------------------------------------------*/


