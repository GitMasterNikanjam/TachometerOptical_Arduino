
// #######################################################################
// Include libraries:

#include "TachometerOptical.h"

// #######################################################################
// Define macros:


#define RPM1_PIN                     41        // RC PWM input signal pin number.

// ############################################################################
// Define Global variables and objects:

//RPM object.
RPM rpm;

// ##########################################################################
// Setup:

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(rpm.attach(1, RPM1_PIN) == false)
  {
    Serial.println(rpm.errorMessage);
    while(1);
  }
  
  if(rpm.init() == false)
  {
    Serial.println(rpm.errorMessage);
    while(1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  rpm.update();

  Serial.println(rpm.value.raw);

  delay(100);
}
