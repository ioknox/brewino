#ifndef __STATUS_HPP__
#define __STATUS_HPP__

#include <Arduino.h>

#include "Pin.hpp"
#include "Tank.hpp"
#include "Motor.hpp"

class Status
{
public:
  Status() : valve_boil_to_mash(OUTPUT, false),
    valve_boil_to_filter_via_cooler(OUTPUT, false),
    valve_mash_to_filter(OUTPUT, false),
    valve_mash_to_filter_via_cooler(OUTPUT, false),
    valve_city_water_input(OUTPUT, false),
    valve_city_sewer(OUTPUT, false),
    valve_city_water_to_boil(OUTPUT, false),
    valve_filter_to_boil(OUTPUT, false),
    valve_filter_to_mash(OUTPUT, false) { /* Nothing to do ... */ }
  unsigned long timestamp;
  Tank tank_mash;
  Tank tank_boil;
  Pin valve_boil_to_mash;
  Pin valve_boil_to_filter_via_cooler;
  Pin valve_mash_to_filter;
  Pin valve_mash_to_filter_via_cooler;
  Pin valve_city_water_input;
  Pin valve_city_sewer;
  Pin valve_city_water_to_boil;
  Pin valve_filter_to_boil;
  Pin valve_filter_to_mash;
  Motor motor_pump;
  Motor motor_mixer;
  virtual void write(Stream *stream);
  virtual void setup();
  virtual void update();
  virtual void reset();
};

#endif
