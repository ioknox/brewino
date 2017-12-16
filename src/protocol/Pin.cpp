#include "Pin.hpp"
#include <msgpck.h>

Pin::Pin(uint8_t _mode, bool _is_analog)
  : pin(-1), value(0), reset_value(0), mode(_mode), is_analog(_is_analog)
{
  /* Nothing to declare */
}

void Pin::setup()
{
  if (pin >= 0)
  {
    pinMode(pin, mode);
  }
}

void Pin::write(Stream *stream)
{
  msgpck_write_map_header(stream, 2);
  msgpck_write_string(stream, "p");
  msgpck_write_integer(stream, pin);
  msgpck_write_string(stream, "v");
  msgpck_write_integer(stream, value);
}

void Pin::update()
{
  if (pin >= 0)
  {
    if (mode != OUTPUT)
    {
      if (is_analog)
      {
        value = analogRead(pin);
      }
      else
      {
        value = digitalRead(pin);
      }
    }
    else
    {
      if (is_analog)
      {
        analogWrite(pin, map(value, 0, 1023, 0, 255));
      }
      else
      {
        digitalWrite(pin, value == 0 ? LOW : HIGH);
      }
    }
  }
}

void Pin::reset()
{
  if (mode == OUTPUT)
  {
    value = reset_value;
  }

  update();
}
