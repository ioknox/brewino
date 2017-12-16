#include <Arduino.h>
#include "main.hpp"
#include "Base64Stream.hpp"
#include "protocol/Status.hpp"
#include <msgpck.h>

namespace CommandEnum
{
  enum CommandEnum : uint8_t
  {
      Unknown = 0,
      SendStatus = 1,
      SetTankLevel = 2,
      SetPin = 3,
      Status = 4
  };
}

namespace TankEnum
{
  enum TankEnum : uint8_t
  {
      Unknown = 0,
      MashTank = 1,
      BoilTank = 2,
      ExternalTank = 3
  };
}

namespace PinEnum
{
  enum PinEnum : uint8_t
  {
      Unknown = 0,
      MashTankPower = 1,
      BoilTankPower = 2,
      MotorMashSpeed = 3,
      MotorMashDirection = 4,
      MotorPumpSpeed = 5,
      MotorPumpDirection = 6,
      ValveBoilToMash = 7,
      ValveBoilToFilterViaCooler = 8,
      ValveMashToFilter = 9,
      ValveMashToFilterViaCooler = 10,
      ValveCityWaterIn = 11,
      ValveCitySewer = 12,
      ValveCityWaterToBoil = 13,
      ValveFilterToBoil = 14,
      ValveFilterToMash = 15
  };
}

Status status;

SoftwareSerial Debug::serial(2, 3);

int serial_available;
unsigned long serial_last_available;

bool read_send_status()
{
  bool res;
  uint32_t size;
  res = msgpck_read_array_size(&Serial, &size) && size == 0;
  if (!res)
  {
    Debug::serial.print("invalid sendstatus content size = ");
    Debug::serial.println(size);
    return false;
  }

  msgpck_write_array_header(&Serial, 2);
  msgpck_write_integer(&Serial, CommandEnum::Status);
  status.write(&Serial);
  return res;
}

bool read_set_pin()
{
  bool res;
  uint32_t size;

  res = msgpck_read_array_size(&Serial, &size) && size == 3;
  if (!res)
  {
    Debug::serial.print("invalid setpin content size = ");
    Debug::serial.println(size);
    return false;
  }

  PinEnum::PinEnum id = PinEnum::Unknown;
  res = msgpck_read_integer(&Serial, (byte *)&id, sizeof(PinEnum::PinEnum));
  if (!res)
  {
    Debug::serial.print("failed to read setpin id");
    return false;
  }

  int8_t pin = 0;
  res = msgpck_read_integer(&Serial, (byte *)&pin, sizeof(int8_t));
  if (!res)
  {
    Debug::serial.print("failed to read setpin pin");
    return false;
  }

  uint16_t value = 0;
  res = msgpck_read_integer(&Serial, (byte *)&value, sizeof(uint16_t));
  if (!res)
  {
    Debug::serial.print("failed to read setpin value");
    return false;
  }

  uint16_t constrained = constrain(value, 0, 1023);

  switch (id)
  {
    case PinEnum::MashTankPower:
      status.tank_mash.power.pin = pin;
      status.tank_mash.power.value = constrained;
      break;
    case PinEnum::BoilTankPower:
      status.tank_boil.power.pin = pin;
      status.tank_boil.power.value = constrained;
      break;
    case PinEnum::MotorMashSpeed:
      status.motor_mixer.speed.pin = pin;
      status.motor_mixer.speed.value = constrained;
      break;
    case PinEnum::MotorMashDirection:
      status.motor_mixer.direction.pin = pin;
      status.motor_mixer.direction.value = constrained;
      break;
    case PinEnum::MotorPumpSpeed:
      status.motor_pump.speed.pin = pin;
      status.motor_pump.speed.value = constrained;
      break;
    case PinEnum::MotorPumpDirection:
      status.motor_pump.direction.pin = pin;
      status.motor_pump.direction.value = constrained;
      break;
    case PinEnum::ValveBoilToMash:
      status.valve_boil_to_mash.pin = pin;
      status.valve_boil_to_mash.value = constrained;
      break;
    case PinEnum::ValveBoilToFilterViaCooler:
      status.valve_boil_to_filter_via_cooler.pin = pin;
      status.valve_boil_to_filter_via_cooler.value = constrained;
      break;
    case PinEnum::ValveMashToFilter:
      status.valve_mash_to_filter.pin = pin;
      status.valve_mash_to_filter.value = constrained;
      break;
    case PinEnum::ValveMashToFilterViaCooler:
      status.valve_mash_to_filter_via_cooler.pin = pin;
      status.valve_mash_to_filter_via_cooler.value = constrained;
      break;
    case PinEnum::ValveCityWaterIn:
      status.valve_city_water_input.pin = pin;
      status.valve_city_water_input.value = constrained;
      break;
    case PinEnum::ValveCitySewer:
      status.valve_city_sewer.pin = pin;
      status.valve_city_sewer.value = constrained;
      break;
    case PinEnum::ValveCityWaterToBoil:
      status.valve_city_water_to_boil.pin = pin;
      status.valve_city_water_to_boil.value = constrained;
      break;
    case PinEnum::ValveFilterToBoil:
      status.valve_filter_to_boil.pin = pin;
      status.valve_filter_to_boil.value = constrained;
      break;
    case PinEnum::ValveFilterToMash:
      status.valve_filter_to_mash.pin = pin;
      status.valve_filter_to_mash.value = constrained;
      break;
    default:
      Debug::serial.print("invalid setpin id");
      return false;
  }

  msgpck_write_array_header(&Serial, 2);
  msgpck_write_integer(&Serial, CommandEnum::SetPin);
  msgpck_write_array_header(&Serial, 3);
  msgpck_write_integer(&Serial, id);
  msgpck_write_integer(&Serial, pin);
  msgpck_write_integer(&Serial, value);

  return true;
}

bool read_tank_level()
{
  bool res;
  uint32_t size;
  res = msgpck_read_array_size(&Serial, &size) && size == 2;
  if (!res)
  {
    Debug::serial.print("invalid settanklevel content size = ");
    Debug::serial.println(size);
    return false;
  }

  TankEnum::TankEnum id = TankEnum::Unknown;
  res = msgpck_read_integer(&Serial, (byte *)&id, sizeof(TankEnum::TankEnum));
  if (!res)
  {
    Debug::serial.print("failed to read tank id");
    return false;
  }

  uint16_t level = 0;
  res = msgpck_read_integer(&Serial, (byte *)&level, sizeof(uint16_t));
  if (!res)
  {
    Debug::serial.print("failed to read tank level");
    return false;
  }

  switch (id)
  {
    case TankEnum::BoilTank:
      status.tank_boil.level = level;
      break;
    case TankEnum::MashTank:
      status.tank_mash.level = level;
      break;
    default:
      Debug::serial.print("invalid tank id");
      return false;
  }

  msgpck_write_array_header(&Serial, 2);
  msgpck_write_integer(&Serial, CommandEnum::SetTankLevel);
  msgpck_write_array_header(&Serial, 2);
  msgpck_write_integer(&Serial, id);
  msgpck_write_integer(&Serial, level);

  return true;
}

bool read_command()
{
  bool res;
  uint32_t size;

  res = msgpck_read_array_size(&Serial, &size) && size == 2;
  if (!res)
  {
    Debug::serial.print("invalid union size = ");
    Debug::serial.println(size);
    return false;
  }

  CommandEnum::CommandEnum command;
  res = msgpck_read_integer(&Serial, (byte*)&command, sizeof(CommandEnum::CommandEnum));
  if (!res)
  {
    Debug::serial.println("failed to read command integer");
    return false;
  }

  switch (command)
  {
    case CommandEnum::SendStatus:
      res = read_send_status();
      break;
    case CommandEnum::SetPin:
      res = read_set_pin();
      break;
    case CommandEnum::SetTankLevel:
      res = read_tank_level();
      break;
    default:
      res = false;
      Debug::serial.println("invalid command");
  }

  Serial.flush();

  return res;
}

void setup()
{
  serial_available = 0;
  serial_last_available = 0;

  Debug::serial.begin(9600);

  Serial.begin(115200);
  while (!Serial)
  {
    ; // Wait for serial
  }

  status.reset();
  status.setup();

  Debug::serial.println("Started");
}

void loop()
{
  Debug::serial.flush();

  unsigned long now = millis();

  int available = Serial.available();
  if (available > serial_available)
  {
    serial_available = available;
    serial_last_available = now;
  }
  else if (serial_available > 0)
  {
    if (now - serial_last_available > 10) // Read timout
    {
      status.timestamp = now;

      read_command();

      while (Serial.available())
      {
        Serial.read();
      }

      serial_available = 0;
    }
  }

  status.update();
}
