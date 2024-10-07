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

#define TachometerOptical_VERSION  "v1.0" // software version of this library

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

      // Dead zone value [us] of channels from RAW_MID value point that ignored.
      uint16_t DEADZONE;

      // [Hz]. Low pass filter frequency(Cutoff filter frequency). **Hint: 0 value means disable it.
      static float FILTER_FRQ;

      // Update method frequency. This value insure that RC maped/filtered values just update in certain frequency. **Hint: 0 value means disable it.
      static float UPDATE_FRQ;

      // Digital pin number of arduino that used for input pwm signal.
      // -1 value means no pin.
      int8_t PIN_NUM;	

      uint8_t CHANNEL_NUM;											

    }parameters;

    // Values struct.
    struct VariablesStruct
    {
      // [us]. Raw input pwm signal measurement values. (For 8 channel).
      volatile uint16_t raw;

      // [us]. Maped input pwm signal values. (For 8 channel).
      float maped;
      
      // [us]. Filtered maped input pwm signal values. (For 8 channel).
      float filtered;		
    }value;

    /**
    * Constructor. Init default value of variables and parameters.
    */
    RPM();

    // Destructor
    ~RPM();

    /**
     * Initialize RCIN_PWM object. Check parameters validation.
     * @return true if successed.
     */ 
    bool init(void);

    /**
     * Attach a digital pin to RCIN_PWM channels. 
     * Static function to create or get the instance for a specific channel
     * Creates an instance of the class for a specific channel and pin. If an object for the same channel exists, it is replaced.
     */ 
    bool attach(uint8_t channel_number, uint8_t pin_number);
	
    // Static function to detach the object for a specific channel
    // Removes an object for a specific channel.
    bool detach(void);
    
    /**
     * Set dead zone of raw pwm value [us] for channels.
     * @return true if successed.
     */
    void setDeadzone(uint16_t value);
    
    /**
     * Set filter frequency [Hz] for low pass filter. **Hint: 0 value means it is disable.
     * @return true if successed.
     */
    bool setFilterFrequency(float value);
    
    /**
     * Set update method frequency [Hz]. **Hint: 0 value means it is disable.
     * @return true if successed.
     */
    bool setUpdateFrequency(float value);

    /**
     * update and calculate maped/filtered pwm value.
     */
    static void update(void);

  private:

    // Static array to store instances per channel
    // Array to hold one object per channel (1-3)
    static RPM* _instances[3];

    // Define function pointer type
    typedef void (*FunctionPtr)();

    // FunctionPtr object for RCIN PWM interrupts handler.
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
    * Calculate pwm maped value.
    * @return PWM maped value.
    */
    uint16_t _map(void);

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