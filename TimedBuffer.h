#ifndef __TIMEDBUFFER_H__
#define __TIMEDBUFFER_H__

#include "Buffer.h"

template <typename TValue>
class TimedBuffer
{
  public: 
    TimedBuffer(int length, int timeStep);
    bool isReady();
    virtual void set(TValue value);
    TValue avg();

  private:
    int _timeStep;
    unsigned long _lastSet;
    TValue _value;
};

template <typename TValue>
TimedBuffer<TValue>::TimedBuffer(int length, int timeStep)
  : /*Buffer<TValue>(length), */_timeStep(timeStep)
{
  // Nothing to do... 
}

template <typename TValue>
bool TimedBuffer<TValue>::isReady()
{
  return (millis() - _lastSet) >= _timeStep;
}

template <typename TValue>
void TimedBuffer<TValue>::set(TValue value)
{
  _lastSet = millis();
  _value = value;
  //Buffer<TValue>::set(value);
}

template <typename TValue>
TValue TimedBuffer<TValue>::avg()
{
  return _value;
}

#endif
