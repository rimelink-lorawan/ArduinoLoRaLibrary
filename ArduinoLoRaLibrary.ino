/**
 * \file
 *         ArduinoLoRaLibrary.ino
 * \description
 *         LoRa library for Arduino by RimeLink (www.rimelink.com)
 * \date
 *         2019-10-27 09:44 by Peter
 *         2021-07-06 14:23 by John
 * \copyright
 *         Revised BSD License, see section \ref LICENSE
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <SoftwareSerial.h>
#include "lora.h"


/* Private macro -------------------------------------------------------------*/
#define WAKEUP_PIN    2  // Arduino Pin2 <--> LoRa Module wakeup pin


/* Private variables -----------------------------------------------------------*/
lora    LoRa;

SoftwareSerial    debugSerial(255, 9);  // NO RX, 9=TX
extern SoftwareSerial    debugSerial;


/* Private function prototypes --------------------------------------------------*/
static void TestClassA(void);
static void TestClassC(void);


/*---------------------------------------------------------------------------*/
void setup()
{
    pinMode(WAKEUP_PIN, INPUT);  // LoRa module LOW wakeup connected to pin 2

    Serial.begin(57600);  // for LoRa node
  
    debugSerial.begin(57600); // software serial at 57600-8-N-1 for debug
    debugSerial.println("debug begin");
}

/*---------------------------------------------------------------------------*/
void loop()
{
    TestClassA();
//    TestClassC();
}

/*---------------------------------------------------------------------------*/
static void TestClassA(void)
{
    int8_t    SNR;
    uint8_t    rxBuf[64];
    int16_t    result, RSSI;
    char*    strStatus = NULL;

    debugSerial.println();
    result = LoRa.write("0123456789", 10, false, 100, &strStatus, 2000);  // false=unconfirmed, FPort=100

    if (result < 0) 
    {
        debugSerial.print("LoRa.write() error, return=");
        debugSerial.println(result);
    }
    else
    {
        /* print TX status */
        debugSerial.write(strStatus, strlen(strStatus));
        debugSerial.println();  

        /* print RX wait time */
        debugSerial.print("WaitRX:");
        debugSerial.print(result);
        debugSerial.println("ms");

        if (LoRa.available())
        {
            result = LoRa.read(&rxBuf[0], sizeof(rxBuf), &RSSI, &SNR);
            if (result < 0)
            {
                debugSerial.print("LoRa.read() failed, return=");
                debugSerial.println(result);
            }
            else
            {
                debugSerial.print("RSSI:");
                debugSerial.print(RSSI);
                debugSerial.print("dBm, SNR:");
                debugSerial.print(SNR);
                debugSerial.print("dB, RX:");
                if (0 == result)
                {
                    debugSerial.print("ACK");
                }
                else
                {
                    debugSerial.print(result);
                    debugSerial.print("Bytes ");
                    debugSerial.write(&rxBuf[0], result);
                }
                debugSerial.println();
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
static void TestClassC(void)
{
    int8_t    SNR;
    uint8_t    rxBuf[64];
    int16_t    value, result, RSSI;
    char*    strStatus = NULL;

    /* check looply until the pin is LOW */
    do {
        value = digitalRead(WAKEUP_PIN);
    } while (HIGH == value);  

    debugSerial.println();

    if (LoRa.available())
    {
        result = LoRa.read(&rxBuf[0], sizeof(rxBuf), &RSSI, &SNR);
        if (result < 0)
        {
            debugSerial.print("LoRa.read() failed, return=");
            debugSerial.println(result);
        }
        else
        {
            debugSerial.print("RSSI:");
            debugSerial.print(RSSI);
            debugSerial.print("dBm, SNR:");
            debugSerial.print(SNR);
            debugSerial.print("dB, RX:");
            if (0 == result)
            {
                debugSerial.print("ACK");
            }
            else
            {
                debugSerial.print(result);
                debugSerial.print("Bytes ");
                debugSerial.write(&rxBuf[0], result);
            }
            debugSerial.println();
        }
    }
}


/*--------------------------------------------------------------------------
                                                                       0ooo
                                                           ooo0     (   )
                                                            (   )      ) /
                                                             \ (      (_/
                                                              \_)
----------------------------------------------------------------------------*/

