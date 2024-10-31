# TachometerOptical Library for Arduino 

- This library can used for optical tachometers apllications. eg: Motor RPM measurement.  
- The main class is **TachometerOptical**. 
- Max 3 number optical TachometerOptical object can created from RPTachometerOpticalM class at the same time.   
- Each TachometerOptical object has its own digital pin number for signal interrupts.  
- It should be just use digital pins that can used in hardware external interrupts mode. otherwise it can not work correct.  
- The pins for get input interrupts is pull up.