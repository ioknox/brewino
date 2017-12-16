#include "Status.hpp"
#include <msgpck.h>

void Status::setup()
{
  tank_mash.setup();
  tank_boil.setup();
  motor_mixer.setup();
  motor_pump.setup();
  valve_boil_to_mash.setup();
  valve_boil_to_filter_via_cooler.setup();
  valve_mash_to_filter.setup();
  valve_mash_to_filter_via_cooler.setup();
  valve_city_water_input.setup();
  valve_city_sewer.setup();
  valve_city_water_to_boil.setup();
  valve_filter_to_boil.setup();
  valve_filter_to_mash.setup();
}

void Status::write(Stream *stream)
{
  msgpck_write_map_header(stream, 6);
  msgpck_write_string(stream, "ts");
  msgpck_write_integer(stream, timestamp);
  msgpck_write_string(stream, "tk0");
  tank_mash.write(stream);
  msgpck_write_string(stream, "tk1");
  tank_boil.write(stream);
  msgpck_write_string(stream, "m0");
  motor_mixer.write(stream);
  msgpck_write_string(stream, "m1");
  motor_pump.write(stream);
  msgpck_write_string(stream, "v");
  msgpck_write_map_header(stream, 9);
  msgpck_write_string(stream, "bm");
  valve_boil_to_mash.write(stream);
  msgpck_write_string(stream, "bfc");
  valve_boil_to_filter_via_cooler.write(stream);
  msgpck_write_string(stream, "mf");
  valve_mash_to_filter.write(stream);
  msgpck_write_string(stream, "mfc");
  valve_mash_to_filter_via_cooler.write(stream);
  msgpck_write_string(stream, "cwi");
  valve_city_water_input.write(stream);
  msgpck_write_string(stream, "cs");
  valve_city_sewer.write(stream);
  msgpck_write_string(stream, "cwb");
  valve_city_water_to_boil.write(stream);
  msgpck_write_string(stream, "fb");
  valve_filter_to_boil.write(stream);
  msgpck_write_string(stream, "fm");
  valve_filter_to_mash.write(stream);
}

void Status::update()
{
  tank_mash.update();
  tank_boil.update();
  motor_mixer.update();
  motor_pump.update();
  valve_boil_to_mash.update();
  valve_boil_to_filter_via_cooler.update();
  valve_mash_to_filter.update();
  valve_mash_to_filter_via_cooler.update();
  valve_city_water_input.update();
  valve_city_sewer.update();
  valve_city_water_to_boil.update();
  valve_filter_to_boil.update();
  valve_filter_to_mash.update();
}

void Status::reset()
{
  tank_mash.reset();
  tank_boil.reset();
  motor_mixer.reset();
  motor_pump.reset();
  valve_boil_to_mash.reset();
  valve_boil_to_filter_via_cooler.reset();
  valve_mash_to_filter.reset();
  valve_mash_to_filter_via_cooler.reset();
  valve_city_water_input.reset();
  valve_city_sewer.reset();
  valve_city_water_to_boil.reset();
  valve_filter_to_boil.reset();
  valve_filter_to_mash.reset();
}
