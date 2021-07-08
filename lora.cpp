/**
 * \file
 *         lora.cpp
 * \description
 *         LoRa library for Arduino by RimeLink (www.rimelink.com)
 * \date
 *         2019-10-27 09:44 by Peter
 *         2021-07-06 13:33 by John
 * \copyright
 *         Revised BSD License, see section \ref LICENSE
 */

/* Includes ------------------------------------------------------------------*/
#include "lora.h"
#include <stdlib.h>
#include <SoftwareSerial.h>

extern SoftwareSerial    debugSerial;

/* Compile switch-------------------------------------------------------------*/
#define PRINT_STATUS_TIME    0  // 0=disable, 1=enable


/* Private macro -------------------------------------------------------------*/
#define MAX_SIZE_UART_PAYLOAD    (222 + 2)  // 2=MType+FPort, SF7~9=222
#define MAX_SIZE_UART_BUF   (6 + MAX_SIZE_UART_PAYLOAD)  // 6=0xFF+Head+Type+Length+CS+Tail


/* Private variables -----------------------------------------------------------*/
static uint8_t    rxtxBuf[MAX_SIZE_UART_BUF];
static uint8_t    rxSizeInBuf = 0;
	

/*---------------------------------------------------------------------------*/
int16_t lora::available(void)
{
    if (Serial.available())
    {
        delay(10);  // 10ms=57Bytes@57600-8-n-1
    }
    else
    {
        return 0;  // have NO data
    }

    /* get the RX frame from LoRa module */
    rxSizeInBuf = 0;
    while( Serial.available())
    {
        int    rxData = Serial.read();  // drain all data on Serial buffer
        if (rxSizeInBuf < sizeof(rxtxBuf))
        {
            rxtxBuf[rxSizeInBuf++] = rxData;
        }
    }

#if 0 // only for debug
    debugSerial.print("rxSizeInBuf=");
    debugSerial.println(rxSizeInBuf);
//    debugSerial.write(rxtxBuf, rxSizeInBuf);
#endif

    return rxSizeInBuf;
}

/*---------------------------------------------------------------------------*/
int16_t lora::read(void *saveBuf, int16_t maxSize, int16_t *rxRSSI, int8_t *rxSNR)
{
    uint8_t    *startIndex, *endIndex, byte1, byte2;
    int16_t    payloadSize, dataSize, copySize;

    /* check all arguments is valid */
    if (!saveBuf)
    {
        return -1;  // arguments error
    }

    /* find the start of downlink frame */
    endIndex = &rxtxBuf[rxSizeInBuf] - 5; // 5=Head+Type+Length+CS+Tail
    for (startIndex = &rxtxBuf[0]; startIndex < endIndex; ++startIndex)
    {
        if ((0x3C == startIndex[0]) && (0xC0 == startIndex[1])) // Head=0x3C, DownLink=0xC0
        {
            break;  // have found
        }
    }

    if (endIndex <= startIndex)
    {
        return -2;  // Head or Type error
    }

    payloadSize = (int16_t)startIndex[2];  // [2]=Length
    dataSize = payloadSize - 3; // RSSI=2Bytes, SNR=1Byte
    if (dataSize < 0)
    {
        return -3;  // Size error
    }

    if (rxRSSI)
    {
        byte1 = startIndex[3 + dataSize + 0]; // 3=Head+Type+Length
        byte2 = startIndex[3 + dataSize + 1];
        *rxRSSI = (int16_t)((byte1 << 8) | byte2);
    }
	
    if (rxSNR)
    {
        *rxSNR = (int8_t)startIndex[3 + dataSize + 2]; // 3=Head+Type+Length, 2=RSSI
    }

    copySize = (maxSize < dataSize) ? maxSize : dataSize;  // avoid overflow
    if (0 < copySize)
    {
        memcpy(saveBuf, &startIndex[3], copySize); // 3=Head+Type+Length
    }

    return copySize;
}

/*---------------------------------------------------------------------------*/
int16_t lora::read(void *saveBuf, int16_t maxSize)
{
    return read(saveBuf, maxSize, NULL, NULL);
}

/*---------------------------------------------------------------------------*/
int16_t lora::write(const void *srcBuf, int16_t srcSize, bool isConfirmed, uint8_t byFPort)
{
    int16_t    txSize;

    txSize = srcSize + 2; // 2=MType+FPort

    /* check all arguments is valid */
    if (!(srcBuf && (txSize <= MAX_SIZE_UART_PAYLOAD) && (1 <= byFPort && byFPort <= 199)))
    {
        return -1;  // arguments error
    }

    /* Make head of frame. */
    rxtxBuf[0] = 0xFF; // wake LoRa node
    rxtxBuf[1] = 0x3C; // Head
    rxtxBuf[2] = 0x04; // 4=Send LoRa Frame
    rxtxBuf[3] = txSize;
    rxtxBuf[4] = isConfirmed ? 1 : 0;
    rxtxBuf[5] = byFPort;

    /* Copy payload into body of frame. */
    if (srcBuf && (0 < srcSize))
    {
        memcpy(&rxtxBuf[6], srcBuf, srcSize);
    }

    /* Make tail of frame. */
    rxtxBuf[4 + txSize] = calc_cs(&rxtxBuf[1], txSize + 3); // 3=Head, Type, Size
    rxtxBuf[5 + txSize] = 0x0D; // Tail
    
    Serial.write(rxtxBuf, txSize + 6); // 6=0xFF+Head+Type+Length+CS+Tail
    
    return 0;
}

/*---------------------------------------------------------------------------*/
int16_t lora::write(const void *srcBuf, int16_t srcSize)
{
    return write(srcBuf, srcSize, false, 100);  // Unconfirmed, FPort=100
}


/*---------------------------------------------------------------------------*/
int16_t lora::write(const void *srcBuf, int16_t srcSize, bool isConfirmed, uint8_t byFPort, char **saveStatus)
{
    #define MAXINSZ    24
    static uint8_t    instr[MAXINSZ + 1];  // add 1 for '\0'

    int16_t    index, result;
    uint32_t    msStart;

    result = write(srcBuf, srcSize, isConfirmed, byFPort);
    if (result < 0)
    {
        return result;
    }

    /* waiting until reveived or timeout */
    msStart = millis();
    do
    {
        if (Serial.available())
        {
            break;
        }
    } while ((millis() - msStart) < 3000);

#if PRINT_STATUS_TIME
    uint32_t msWait = millis() - msStart;
#endif

    delay(10);  // 10ms=57Bytes@57600-8-n-1

    /* get the responsed string from LoRa module */
    index = 0;
    while( Serial.available())
    {
        int    rxData = Serial.read();  // drain all data on Serial buffer
        if (index < MAXINSZ)
        {
            instr[index++] = rxData;
        }
    }
    instr[index - 2] = '\0';  // 2=CS+Tail

    if (saveStatus)
    {
        *saveStatus = &instr[3];  // 3=Head+Type+Length
    }

#if PRINT_STATUS_TIME
    debugSerial.print("WaitStatus:");
    debugSerial.print(msWait);
    debugSerial.println("ms");
#endif

    if (('T' == instr[3]) && ('X' == instr[4]) && (' ' == instr[5]) && ('O' == instr[6]) && ('K' == instr[7]))
    {
        return 0;
    }
    else
    {
        return -2;
    }
}

/*---------------------------------------------------------------------------*/
int16_t lora::write(const void *srcBuf, int16_t srcSize, bool isConfirmed, uint8_t byFPort, char **saveStatus, uint16_t msTimeOnAir)
{
    int16_t    result;
    uint16_t    msTimeout;
    uint32_t    msStart;

    result = write(srcBuf, srcSize, isConfirmed, byFPort, saveStatus);
    if (result < 0)
    {
        return result;
    }

    if (3000 < msTimeOnAir)
    {
        msTimeout = 3000;  // MAX time on Air is 3000ms
    }
    else
    {
        msTimeout = msTimeOnAir;
    }
    msTimeout += 2000;  // RX1=1000ms, RX2=2000ms

    /* waiting until reveived or timeout */
    msStart = millis();
    do
    {
        if (Serial.available())
        {
            break;
        }
    } while ((millis() - msStart) < msTimeout);

    return (millis() - msStart);
}

/*---------------------------------------------------------------------------*/
uint8_t lora::calc_cs(const void *srcBuf, int16_t srcSize)
{
    uint8_t    bySum;
    const uint8_t    *p_byBuf;

    bySum = 0;
    p_byBuf = (const uint8_t *)srcBuf;
    
    while (0 < srcSize--)
    {
        bySum += *p_byBuf++;
    }

    return bySum;
}

/*--------------------------------------------------------------------------
                                                                       0ooo
                                                           ooo0     (   )
                                                            (   )      ) /
                                                             \ (      (_/
                                                              \_)
----------------------------------------------------------------------------*/


