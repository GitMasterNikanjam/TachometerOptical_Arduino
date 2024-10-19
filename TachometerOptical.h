#ifndef TACHOMETER_OPTICAL_H
#define TACHOMETER_OPTICAL_H

// ##################################################################
// Library information:
/*
TachometerOptical - a small optical tachometer library for Arduino.
Developed by: Mohammad Nikanjam

For more information read README.md file.
*/
// ###################################################################
// Include libraaries:

#include <Arduino.h>

// ####################################################################
// Define macros:


// ###################################################################################
//  General function declarations:

// ##################################################################################3
// RPM class

class RPM
{
  public:

    // Last error accured for object.
    String errorMessage;

    // Parameters struct
    struct ParametersStruct
    {
      /**
       * Min RPM value that accept in update method. Bellow that return zero value.
       * Default value: 0.
      */
      static uint16_t MIN;

      /**
       * Max RPM value that accesp in update method. Upper that return last value updatation.
       * Default value: 0. Means it disabled.
      */
      static uint16_t MAX;

      /*
       * [Hz]. Low pass filter frequency(Cutoff filter frequency). **Hint: 0 value means disable it.
       * Default value: 0.
      */ 
      static float FILTER_FRQ;

      // Update frequency. This value insure that RPM filtered values just update in certain frequency. **Hint: 0 value means disable it.
      static float UPDATE_FRQ;

      // Digital pin number of arduino that used for input pwm signal.
      // -1 value means no pin assigned.
      int8_t PIN_NUM;	

      // Channel number. Max 3 deferent channel can be used for all RPM objects.
      uint8_t CHANNEL_NUM;											

    }parameters;

    // Values struct.
    struct VariablesStruct
    {
      // [RPM]. Raw input RPM signal measurement values.
      uint16_t raw;
      
      // [RPM]. RPM value after lowpass filter and MIN/MAX saturation.
      float filtered;		
    }value;

    /**
    * Constructor. Init default value of variables and parameters.
    */
    RPM();

    // Destructor
    ~RPM();

    /**
     * Initialize object. Check parameters validation.
     * @return true if successed.
     */ 
    bool init(void);

    /**
     * Attach a digital pin to RPM channels. 
     * Static function to create or get the instance for a specific channel
     * Creates an instance of the class for a specific channel and pin. If an object for the same channel exists, it is replaced.
     */ 
    bool attach(uint8_t channel_number, uint8_t pin_number);
	
    // Static function to detach the object for a specific channel
    // Removes an object for a specific channel.
    bool detach(void);

    /**
     * update and calculate filtered RPM value.
     */
    static void update(void);

  private:

    /**
     * [us]. Signal PWM value.
    */
    volatile uint32_t pwmValue;

    /**
     * Static array to store instances per channel
     * Array to hold one object per channel (1-3)  
     * Cell 0 is for channel 1. Cell 1 is for channel 2. Cell 2 is for channel 3.
    */
    static RPM* _instances[3];

    // Define function pointer type
    typedef void (*FunctionPtr)();

    // FunctionPtr object for RPM signals interrupts handler.
    FunctionPtr _funPointer;

    // Flag for store state of channeles that attached(true) or not_attached(false)
    bool _attachedFlag;

    // Start timer value for each pwm channel.
    volatile unsigned long _startPeriod;						
    
    // Gain that used at low pass filter calculations.
    // _alpha = 1.0 / (1.0 + _2PI * parameters.FILTER_FRQ / UPDATE_FRQ)
    static float _alpha;
    
    // [us]. Time at update() method.
    static volatile unsigned long _T;

    /** 
    * Check parameters validation.
    * @return true if successed.
    */
    bool _checkParameters(void);

    // Interrupt handler function for RPM channel 1.
    friend void _calcInput_CH1(void);

    // Interrupt handler function for RPM channel 2.
    friend void _calcInput_CH2(void);

    // Interrupt handler function for RPM channel 3.
    friend void _calcInput_CH3(void);
    
};


#endif