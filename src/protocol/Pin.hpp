#ifndef __PIN_HPP__
#define __PIN_HPP__

#include <Arduino.h>

class Pin
{
  public:
    Pin(uint8_t _mode, bool _is_analog);
    int8_t pin;
    uint16_t value;
    uint16_t reset_value;
    uint8_t mode;
    bool is_analog;
    virtual void write(Stream *stream);
    virtual void setup();
    virtual void update();
    virtual void reset();
};

#endif
