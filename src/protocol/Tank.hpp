#ifndef __TANK_HPP__
#define __TANK_HPP__

#include <Arduino.h>

#include "Pin.hpp"

class Tank
{
  public:
    Tank() : temperature(-127), level(0),
      power(OUTPUT, false) { /* Nothing to declare... */ }
    int8_t temperature;
    uint16_t level;
    Pin power;
    virtual void write(Stream *stream);
    virtual void setup();
    virtual void update();
    virtual void reset();
};


#endif
