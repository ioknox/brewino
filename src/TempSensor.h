#ifndef __TEMPSENSOR_H__
#define __TEMPSENSOR_H__

#include "TimedBuffer.h"

class TempSensor
{
  public:
    TempSensor(int pin, float aref)
      : _aref(aref), _pin(pin), _buffer(5, 200) {
      // Nothing to do...
    }

    void loop()
    {
      if (_buffer.isReady())
      {
        _buffer.set(readValue());
      }
    }
    
    float value()
    {
      return _buffer.avg();
    }

  private:
    float readValue() {
      float voltage = (analogRead(_pin) * _aref) / 1024.0f;
      float tempC = (voltage - 500.0f) / 10.0f;
      return round(tempC * 10.0f) / 10.0f;
    }
  
    float _aref;
    int _pin;
    TimedBuffer<float> _buffer;
};

#endif
