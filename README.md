# ArduinoLoRaLibrary
Rime Arduino LoRa Library.

1. This is a auduino library for auduino board;
2. This library is develop for trasmiting data between auduino and Rimelink LoRaWAN Node use serial interface;
3. You can use method "int write(const void *p_vData, int bySize)" to send data directly, and the data will be send to lorawan server by lorawan node automaticly; 
4. You should use method "bool available(void)" to check whether received downlink data from a lorawan server, and if return "true" you can use method "read()" to get the downlink data;
5. You must use method "available()" in front of method "read()".
