#include "Motor.hpp"
#include <msgpck.h>

void Motor::setup()
{
  direction.setup();
  speed.setup();
}

void Motor::write(Stream *stream)
{
  msgpck_write_map_header(stream, 2);
  msgpck_write_string(stream, "dir");
  direction.write(stream);
  msgpck_write_string(stream, "spd");
  speed.write(stream);
}

void Motor::update()
{
  direction.update();
  speed.update();
}

void Motor::reset()
{
  direction.reset();
  speed.reset();
}
