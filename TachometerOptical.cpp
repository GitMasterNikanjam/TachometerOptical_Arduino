#include "TachometerOptical.h"

#define _2PI  6.2831853

// Initialize static array to store instances for each 8 channels
RPM* RPM::_instances[3] = {nullptr};

float RPM::ParametersStruct::FILTER_FRQ = 0;

float RPM::ParametersStruct::UPDATE_FRQ = 0;

float RPM::_alpha = 0;

volatile unsigned long RPM::_T = 0;

// ##########################################################################
// General function definitions:

 void _calcInput_CH1(void)
{
  // if the pin is high, its the start of an interrupt
  if(digitalRead(RPM::_instances[0]->parameters.PIN_NUM) == HIGH)
  { 
    RPM::_instances[0]->_startPeriod = micros();
  }
  else
  {
      RPM::_instances[0]->value.raw = (int)(micros() - RPM::_instances[0]->_startPeriod);
      RPM::_instances[0]->_startPeriod = 0;
  }
}

 void _calcInput_CH2(void)
{
  // if the pin is high, its the start of an interrupt
  if(digitalRead(RPM::_instances[1]->parameters.PIN_NUM) == HIGH)
  { 
    RPM::_instances[1]->_startPeriod = micros();
  }
  else
  {
      RPM::_instances[1]->value.raw = (int)(micros() - RPM::_instances[1]->_startPeriod);
      RPM::_instances[1]->_startPeriod = 0;
  }
}

 void _calcInput_CH3(void)
{
  // if the pin is high, its the start of an interrupt
  if(digitalRead(RPM::_instances[2]->parameters.PIN_NUM) == HIGH)
  { 
    RPM::_instances[2]->_startPeriod = micros();
  }
  else
  {
      RPM::_instances[2]->value.raw = (int)(micros() - RPM::_instances[2]->_startPeriod);
      RPM::_instances[2]->_startPeriod = 0;
  }
}

// ##########################################################################

// RCIN_PWM RCIN_PWM;

RPM::RPM()
{
		// Set default value at construction function:

    parameters.DEADZONE = 0;	
    parameters.PIN_NUM = -1;	
    parameters.CHANNEL_NUM = 0;	

    value.raw = 0;
    value.maped = 0;
    value.filtered = 0;

    _T = 0;
    _attachedFlag = false;
}

// Destructor
RPM::~RPM() 
{
    // Detach interrupt when the object is destroyed
    detachInterrupt(digitalPinToInterrupt(parameters.PIN_NUM));
}

bool RPM::attach(uint8_t channel_number, uint8_t pin_number)
{	
  if( (channel_number > 8) || (channel_number == 0) )
  {
    errorMessage = "Error RCIN_PWM: channel number is not correct.";
    return false;
  }

  if (_instances[channel_number - 1] != nullptr) 
  {
      // Detach any existing object for this channel
      _instances[channel_number - 1]->detach();
      delete _instances[channel_number - 1];
  }

  // Create a new object for this channel
  _instances[channel_number - 1] = this;

  parameters.CHANNEL_NUM = channel_number;
  parameters.PIN_NUM = pin_number;
  _attachedFlag = true;
  
  return true;
}

// Static function to detach a channel and remove the object
bool RPM::detach(void) 
{
  // Detach the interrupt and delete the object
  detachInterrupt(digitalPinToInterrupt(parameters.PIN_NUM));
  _instances[parameters.CHANNEL_NUM - 1] = nullptr;
  _attachedFlag = false;
  return true;
}


uint16_t RPM::_map(void)
{

  value.maped = (60.0/(float)value.raw)*1000000.0;

  return value.maped;
}

void RPM::setDeadzone(uint16_t value)
{
  parameters.DEADZONE = value;
}

bool RPM::setFilterFrequency(float frq)
{
  if(frq < 0)
  {
    errorMessage = "Error RCIN_PWM: filter frequency can not be negative.";
    return false;
  }
	RPM::parameters.FILTER_FRQ = frq;
}

bool RPM::setUpdateFrequency(float frq)
{
  if(frq < 0)
  {
    errorMessage = "Error RCIN_PWM: Update frequency can not be negative.";
    return false;
  }

	RPM::parameters.UPDATE_FRQ = frq;
}
	
void RPM::update(void)
{
	unsigned long t = micros();
  unsigned long dt = t - _T;

  if(RPM::ParametersStruct::UPDATE_FRQ > 0)
  {
    if(dt < (1000000.0/RPM::ParametersStruct::UPDATE_FRQ))
    {
      return ;
    }
  }

  if(RPM::ParametersStruct::FILTER_FRQ > 0)
  {
    RPM::_alpha = 1.0 / (1.0 + _2PI * RPM::ParametersStruct::FILTER_FRQ * dt / 1000000.0);
  }
  else
  {
    RPM::_alpha = 0;
  }

  for(int i = 1; i <= 8; i++)
  {
    if(_instances[i-1]->_attachedFlag == true)
    {

      _instances[i-1]->_map();
      
      if(_instances[i-1]->parameters.FILTER_FRQ > 0)
      {
        _instances[i-1]->value.filtered = _alpha * _instances[i-1]->value.filtered + (1.0 - _alpha) * _instances[i-1]->value.maped;
      }
      else
      {
        _instances[i-1]->value.filtered = _instances[i-1]->value.maped;
      }
    }
  }	
	
		_T = t;
	
}	

bool RPM::init(void)
{
  if(!_checkParameters())
  {
    return false;
  }

  if( (_attachedFlag == true) && (parameters.PIN_NUM >= 0) )
  {
    pinMode(parameters.PIN_NUM,INPUT_PULLUP);
    
    _funPointer = nullptr;
    
    switch(parameters.CHANNEL_NUM)
    {
      case 1:
        _funPointer = _calcInput_CH1;
      break;
      case 2:
        _funPointer = _calcInput_CH2;
      break;
      case 3:
        _funPointer = _calcInput_CH3;
      break;	
    }

    attachInterrupt(digitalPinToInterrupt(parameters.PIN_NUM), _funPointer, CHANGE);
  }
  else
  {
    _attachedFlag = false;
  }
  

  return true;
}

bool RPM::_checkParameters(void)
{
  bool state = (parameters.FILTER_FRQ >= 0) && (parameters.UPDATE_FRQ >= 0) &&
               (parameters.PIN_NUM >= 0) && (parameters.CHANNEL_NUM >= 1) && (parameters.CHANNEL_NUM <= 3) ;

  if(state == false)
  {
    errorMessage = "Error RCIN_PWM: One or some parameters is not correct.";
    return false;
  }

  return true;
}
