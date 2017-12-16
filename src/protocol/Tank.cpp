#include "Tank.hpp"
#include <msgpck.h>

void Tank::setup()
{
  power.setup();
}

void Tank::write(Stream *stream)
{
  msgpck_write_map_header(stream, 3);
  msgpck_write_string(stream, "tmp");
  msgpck_write_integer(stream, temperature);
  msgpck_write_string(stream, "lvl");
  msgpck_write_integer(stream, level);
  msgpck_write_string(stream, "pwr");
  power.write(stream);
}

void Tank::update()
{
  power.update();
}

void Tank::reset()
{
  power.reset();
}
