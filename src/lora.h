// 
//    FILE: lora.h
// VERSION: 0.1.00
// PURPOSE: RimeLink LORA library for Arduino
//
//     URL: http://arduino.cc/
//
// HISTORY:
// see lora.cpp file
// 

#ifndef lora_h
#define lora_h

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define LORA_LIB_VERSION "0.1.00"

class lora
{
public:
    bool available(void);
    byte read(void *p_bData, byte maxlen);
    int write(const void *p_vData, int bySize);

private:
	byte CalcCS(const void *p_vBuf, int nSize);
	void handleRxData(uint8_t inChar);
	bool rxDataReady(void);
};
#endif
//
// END OF FILE
//
