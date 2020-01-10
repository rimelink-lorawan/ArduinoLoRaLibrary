//
//    FILE: lora.cpp
// VERSION: 0.1.00
// PURPOSE: RimeLink LO0RA library for Arduino
//
// DATASHEET: 
//
// HISTORY:
// 0.1.0 by Peter (15/12/2019)
// inspired by LORA library
//

#include "lora.h"

/////////////////////////////////////////////////////
//
// PUBLIC
//
/* Private macro -------------------------------------------------------------*/
#define RX_BUFFER_LEN   255
#define MAX_LEN_UART_FRAME_DATA    200
#define COMM_TRM_HEAD    0x3Cu
#define COMM_TRM_TAIL    0x0Du
#define PTR_NULL    ((void *)0)

/**
* @brief  Make response type of UART frame. 
*/
#define MAKE_UART_TYPE_RESP(byType)    (0x80u + (byType))

/**
* @brief  Type of received communication frame
*/
typedef enum
{
    TYPE_INVALID_MIN = (byte)0,
    TYPE_GET_VER=1, /*!< User System get version of this node. */
    TYPE_TX_RF_DATA=2, /*!< User System send data that need to TX by RF. */
    TYPE_GET_JOINEDSTATE=3, /*!< User System get Network Joined State. */
    TYPE_TX_RF_DATA_BYTYPE=4, 
    TYPE_CLASS=5,
    TYPE_APPEUI=6,
    TYPE_APPKEY=7,
    TYPE_ADR=8,
    TYPE_TX_PWR=9, 
    TYPE_DATARATE=0x0a,
    TYPE_CHANNEL=0x0b,
    TYPE_INVALID_MAX=0x0C,
    TYPE_WAKE_DATA = ((byte)0xC0), /*!< Node send wake data to User System. */
} COMM_FRAME_TYPE_TypeDef;
/**
* @brief  Data object for received communication frame
*/
typedef struct
{
    byte    dataIdx;    /* Index of data field */
    byte    dataLen;    /* Length of data field */
    byte    frameLen; /* Length of frame */
    bool    rx_ready;  /* Indicate whether the whole frame received */
    byte    RxBuf[RX_BUFFER_LEN];
} COMM_RX_DATA;

typedef struct
{
    byte head;
    byte cmd;
    byte dl;    /* data length of rime lora->serial frame */
    byte data;
    byte tail;
}FRAME_IDX;

COMM_RX_DATA RxBuff={0, 0, 0, false, {0}};
byte    s_abyTxUARTFrame[255];

/* start-RX method of LoRa library */
void lora::handleRxData(uint8_t inChar)
{
    COMM_RX_DATA *pRx=&RxBuff;
    
    if (pRx->frameLen>= RX_BUFFER_LEN) return;
    if (!pRx->rx_ready) {
       pRx->RxBuf[pRx->frameLen] = inChar;
       pRx->frameLen++;
    }
}

bool lora::rxDataReady(void)
{
    FRAME_IDX idx;
    COMM_RX_DATA *pRx=&RxBuff;

    for(byte i=0; i<pRx->frameLen; i++)
    {
        idx.head=i;
        if(COMM_TRM_HEAD==pRx->RxBuf[idx.head]) /* Check Frame Head */
        {
            idx.cmd=idx.head+1;
            if(TYPE_WAKE_DATA==pRx->RxBuf[idx.cmd]) /* Check Frame Command */
            {
                idx.dl=idx.head+2;
                idx.data=idx.head+3;
                idx.tail=idx.head+4+(pRx->RxBuf[idx.dl]); /* Calculate the index of Tail in rx Frame*/
                if(COMM_TRM_TAIL==pRx->RxBuf[idx.tail]) /* Check Frame Tail */
                {
                    pRx->dataIdx = idx.data;
                    pRx->dataLen = pRx->RxBuf[idx.dl];
                    pRx->rx_ready = true;
                    break;
                } /* end of check frame tail */
            } /* end of check frame command */
        } /* end of check frame head */
    } /* end of loop */
    return (pRx->rx_ready);
}

bool lora::available(void)
{
    bool rtn=false;
    while(Serial.available()) {
        //get the new byte
        char inChar = (char)Serial.read();
        handleRxData(inChar);
    }
    if(rxDataReady()) {
        rtn=true;
    }
    else
    {
        /* Clear RxBuff */
        RxBuff.dataIdx=0;
        RxBuff.dataLen=0;
        RxBuff.frameLen=0;
        RxBuff.rx_ready=false;
        memset(RxBuff.RxBuf, 0, RX_BUFFER_LEN);
    }
    return rtn;
}

byte lora::read(void *p_bData, byte maxlen)
{
    byte copylen;
    COMM_RX_DATA *pRx=&RxBuff;
    
    if ((PTR_NULL == p_bData) || (0 == maxlen) || (3 >= pRx->dataLen)) /* data field have 3 bytes RSSI+SNR at tail*/
        return (-1);

    /* Calculate memcpy length */
    copylen = pRx->dataLen-3;
    copylen = (copylen>maxlen)?maxlen:copylen;

    memcpy(p_bData, &pRx->RxBuf[pRx->dataIdx], copylen);
    pRx->dataIdx=0;
    pRx->dataLen=0;
    pRx->frameLen=0;
    pRx->rx_ready=false;
    memset(pRx->RxBuf, 0, RX_BUFFER_LEN);
    return copylen;
}
/* end-RX method of LoRa library */

/* start-TX method of LoRa library */
int lora::write(const void *p_vData, int bySize)
{
    int byTxsize;

    byTxsize = bySize + 2;
    if (MAX_LEN_UART_FRAME_DATA < byTxsize)
    {
        return -1; /* Bad size of data. */
    }
    /* Make head of frame. */
    s_abyTxUARTFrame[0] = 0xff;
    s_abyTxUARTFrame[1] = 0x3c;
    s_abyTxUARTFrame[2] = 0x04;
    s_abyTxUARTFrame[3] = byTxsize;
    s_abyTxUARTFrame[4] = 0x00;
    s_abyTxUARTFrame[5] = 0x64;

    /* Copy payload into body of frame. */
    if (p_vData && (0 < bySize))
    {
        memcpy(&s_abyTxUARTFrame[6], p_vData, bySize);
    }

    /* Make tail of frame. */
    s_abyTxUARTFrame[4 + byTxsize] = CalcCS(&s_abyTxUARTFrame[1], byTxsize + 3);
    s_abyTxUARTFrame[5 + byTxsize] = 0x0d;
    Serial.write(s_abyTxUARTFrame, byTxsize+6);
    
    return 0;
}
 

/////////////////////////////////////////////////////
//
// PRIVATE
//

// return values:
//  0 : OK
// -2 : timeout
byte lora::CalcCS(const void *p_vBuf, int nSize)
{
    byte    bySum;
    const byte    *p_byBuf;

    bySum = 0;
    p_byBuf = (const byte *)p_vBuf;
    
    while (nSize-- > 0)
    {
        bySum += *p_byBuf++;
    }

    return bySum;
}
/* end-TX method of LoRa library */
//
// END OF FILE
//
