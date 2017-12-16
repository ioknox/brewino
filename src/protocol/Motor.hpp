#ifndef __MOTOR_HPP__
#define __MOTOR_HPP__

#include <Arduino.h>

#include "Pin.hpp"

class Motor
{
public:
  Motor() : direction(OUTPUT, false), speed(OUTPUT, true) {}
  Pin direction;
  Pin speed;
  virtual void write(Stream *stream);
  virtual void setup();
  virtual void update();
  virtual void reset();
};

#endif
