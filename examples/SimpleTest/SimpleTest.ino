
// #######################################################################
// Include libraries:

#include <TachometerOptical.h>

// #######################################################################
// Define macros:

#define RPM1_PIN                     41        // RC PWM input signal pin number.

#define SERIAL_MONITOR_UPDATE_FRQ    50

// ############################################################################
// Define Global variables and objects:

//TachometerOptical object.
TachometerOptical rpm;

uint32_t T,T_monitor;

// ##########################################################################
// Setup:

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  rpm.parameters.CHANNEL_NUM = 1;
  rpm.parameters.PIN_NUM = RPM1_PIN;
  rpm.parameters.UPDATE_FRQ = 100;
  rpm.parameters.FILTER_FRQ = 100;
  rpm.parameters.MIN = 0;
  rpm.parameters.MAX = 15000;

  if(rpm.init() == false)
  {
    Serial.println(rpm.errorMessage);
    while(1);
  }
}

void loop() {
  
  T = millis();

  rpm.update();

  if((T - T_monitor) >= 1000.0/(float)SERIAL_MONITOR_UPDATE_FRQ)
  {
    // Serial.println(rpm.value.rawRPM);
    Serial.println(TachometerOptical::ValuesStructure::sharedRPM);
    T_monitor = T;
  }
  
}
