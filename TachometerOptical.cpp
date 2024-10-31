
// ######################################################################
// Include libraries:

#include "TachometerOptical.h"

// #######################################################################
// Define macros:

#define _2PI          6.2831853       // 2*pi

// ########################################################################
// Initialize static variables:

TachometerOptical* TachometerOptical::_instances[3] = {nullptr};

uint16_t TachometerOptical::ParametersStructure::MAX = 0;

uint16_t TachometerOptical::ParametersStructure::MIN = 0;

float TachometerOptical::ParametersStructure::FILTER_FRQ = 0;

float TachometerOptical::ParametersStructure::UPDATE_FRQ = 0;

float TachometerOptical::_alpha = 0;

volatile uint32_t TachometerOptical::_T = 0;

float TachometerOptical::ValuesStructure::sharedRPM = 0;	

// ##########################################################################
// General function definitions:

 void _calcInput_CH1(void)
{
  unsigned long tNow = micros();
  TachometerOptical::_instances[0]->_period = (uint32_t)(tNow - TachometerOptical::_instances[0]->_startPeriod);
  TachometerOptical::_instances[0]->_startPeriod = tNow;
}

 void _calcInput_CH2(void)
{
  unsigned long tNow = micros();
  TachometerOptical::_instances[1]->_period = (uint32_t)(tNow - TachometerOptical::_instances[1]->_startPeriod);
  TachometerOptical::_instances[1]->_startPeriod = tNow;
}

 void _calcInput_CH3(void)
{
  unsigned long tNow = micros();
  TachometerOptical::_instances[2]->_period = (uint32_t)(tNow - TachometerOptical::_instances[2]->_startPeriod);
  TachometerOptical::_instances[2]->_startPeriod = tNow;
}

// ##########################################################################
// TachometerOptical class:

TachometerOptical::TachometerOptical()
{
		// Set default value at construction function:

    parameters.PIN_NUM = -1;	
    parameters.CHANNEL_NUM = 0;	

    value.rawRPM = 0;
    value.RPM = 0;

    _period = 0;
    _startPeriod = 0;

    _attachedFlag = false;
}

TachometerOptical::~TachometerOptical() 
{
    detach();
}

bool TachometerOptical::attach(uint8_t channel_number, uint8_t pin_number)
{	
  if( (channel_number > 3) || (channel_number == 0) )
  {
    errorMessage = "Error TachometerOptical: channel number is not correct.";
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
void TachometerOptical::detach(void) 
{
  // Detach the interrupt and delete the object
  detachInterrupt(digitalPinToInterrupt(parameters.PIN_NUM));
  _instances[parameters.CHANNEL_NUM - 1] = nullptr;
  _attachedFlag = false;
}
	
void TachometerOptical::update(void)
{
	unsigned long t = micros();
  unsigned long dt = t - _T;

  if(TachometerOptical::ParametersStructure::UPDATE_FRQ > 0)
  {
    if(dt < (1000000.0/TachometerOptical::ParametersStructure::UPDATE_FRQ))
    {
      return ;
    }
  }

  if(TachometerOptical::ParametersStructure::FILTER_FRQ > 0)
  {
    TachometerOptical::_alpha = 1.0 / (1.0 + _2PI * TachometerOptical::ParametersStructure::FILTER_FRQ * dt / 1000000.0);
  }
  else
  {
    TachometerOptical::_alpha = 0;
  }

  for(int i = 1; i <= 3; i++)
  {
    if(_instances[i-1]->_attachedFlag == true)
    {
      float temp = (double)60.0/(double)(_instances[i-1]->_period)*1000000.0;

      if( (t - TachometerOptical::_instances[i-1]->_startPeriod) >  1000000.0)
      {
        temp = 0;
      }

      if(temp > TachometerOptical::ParametersStructure::MIN)
      {
        if( (float)(temp - _instances[i-1]->value.rawRPM) / (float)dt > 10000.0)
        {
          _instances[i-1]->value.rawRPM = temp;
          continue;
        }
      }

      _instances[i-1]->value.rawRPM = temp;

      if(temp < TachometerOptical::ParametersStructure::MIN)
      {
        temp = 0;
      }
      else if( (temp > TachometerOptical::ParametersStructure::MAX) && (TachometerOptical::ParametersStructure::MAX > 0) )
      {
        continue;
      }

      if(_instances[i-1]->parameters.FILTER_FRQ > 0)
      {
        _instances[i-1]->value.RPM = _alpha * _instances[i-1]->value.RPM + (1.0 - _alpha) * temp;
      }
      else
      {
        _instances[i-1]->value.RPM = temp;
      }

      ValuesStructure::sharedRPM = _instances[i-1]->value.RPM;
    }
  }	
	
		_T = t;
	
}	

bool TachometerOptical::init(void)
{
  if(!_checkParameters())
  {
    return false;
  }

  _period = 0;
  _startPeriod = 0;

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
  else 
  {
    errorMessage = "Error TachometerOptical: Object has not attached to any channel.";
    return false;
  }
  
  return true;
}

bool TachometerOptical::_checkParameters(void)
{
  bool state = (parameters.FILTER_FRQ >= 0) && (parameters.UPDATE_FRQ >= 0) &&
               (parameters.PIN_NUM >= 0) && (parameters.CHANNEL_NUM >= 1) && (parameters.CHANNEL_NUM <= 3) &&
               (parameters.MAX >= parameters.MIN) ;

  if(state == false)
  {
    errorMessage = "Error TachometerOptical: One or some parameters is not correct.";
    return false;
  }

  return true;
}
