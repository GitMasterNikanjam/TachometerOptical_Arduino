#include "TachometerOptical.h"

#define _2PI  6.2831853

// Initialize static array to store instances for each 3 channels.
RPM* RPM::_instances[3] = {nullptr};

uint16_t RPM::ParametersStruct::MAX = 0;

uint16_t RPM::ParametersStruct::MIN = 0;

float RPM::ParametersStruct::FILTER_FRQ = 0;

float RPM::ParametersStruct::UPDATE_FRQ = 0;

float RPM::_alpha = 0;

volatile unsigned long RPM::_T = 0;

// ##########################################################################
// General function definitions:

 void _calcInput_CH1(void)
{
  unsigned long tNow = micros();
  RPM::_instances[0]->pwmValue = (uint32_t)(tNow - RPM::_instances[0]->_startPeriod);
  RPM::_instances[0]->_startPeriod = tNow;
}

 void _calcInput_CH2(void)
{
  unsigned long tNow = micros();
  RPM::_instances[1]->pwmValue = (uint32_t)(tNow - RPM::_instances[1]->_startPeriod);
  RPM::_instances[1]->_startPeriod = tNow;
}

 void _calcInput_CH3(void)
{
  unsigned long tNow = micros();
  RPM::_instances[2]->pwmValue = (uint32_t)(tNow - RPM::_instances[2]->_startPeriod);
  RPM::_instances[2]->_startPeriod = tNow;
}

// ##########################################################################

// RCIN_PWM RCIN_PWM;

RPM::RPM()
{
		// Set default value at construction function:

    parameters.PIN_NUM = -1;	
    parameters.CHANNEL_NUM = 0;	

    value.raw = 0;
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
  if( (channel_number > 3) || (channel_number == 0) )
  {
    errorMessage = "Error RPM: channel number is not correct.";
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

  for(int i = 1; i <= 3; i++)
  {
    if(_instances[i-1]->_attachedFlag == true)
    {
      uint32_t temp = 60.0/(float)(_instances[i-1]->pwmValue)*1000000.0;
      _instances[i-1]->value.raw = temp;

      if(temp < RPM::ParametersStruct::MIN)
      {
        temp = 0;
      }
      else if( (temp > RPM::ParametersStruct::MAX) && (RPM::ParametersStruct::MAX > 0) )
      {
        continue;
      }

      if(_instances[i-1]->parameters.FILTER_FRQ > 0)
      {
        _instances[i-1]->value.filtered = _alpha * _instances[i-1]->value.filtered + (1.0 - _alpha) * temp;
      }
      else
      {
        _instances[i-1]->value.filtered = temp;
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

  if(_attachedFlag == true)
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

    attachInterrupt(digitalPinToInterrupt(parameters.PIN_NUM), _funPointer, RISING);
  }
  
  return true;
}

bool RPM::_checkParameters(void)
{
  bool state = (parameters.FILTER_FRQ >= 0) && (parameters.UPDATE_FRQ >= 0) &&
               (parameters.PIN_NUM >= 0) && (parameters.CHANNEL_NUM >= 1) && (parameters.CHANNEL_NUM <= 3) &&
               (parameters.MAX >= parameters.MIN) ;

  if(state == false)
  {
    errorMessage = "Error RPM: One or some parameters is not correct.";
    return false;
  }

  return true;
}
