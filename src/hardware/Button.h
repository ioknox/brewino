#ifndef __HARDWARE_BUTTON_H__
#define __HARDWARE_BUTTON_H__

#include <Arduino.h>

#define DEBOUNCE_DELAY 30
#define LONG_PRESSED 1000

enum KeyEvent
{
  NoKeyEvent,
  ShortKeyUp,
  LongKeyDown
};

class Button
{
  public:
    Button(int pin)
      : _pin(pin),
        _lastReading(0),
        _longKeyDownSignaled(false),
        _event(NoKeyEvent),
        _duration(0),
        _highSince(0),
        _lowSince(0),
        _lastChanged(0)
    {
      unsigned long now(millis());

      _highSince = now;
      _lowSince = now;
      _lastChanged = now;
      loop();

      pinMode(pin, INPUT);
    }

    void loop()
    {
      unsigned long now(millis());
      int reading = digitalRead(_pin);

      _event = NoKeyEvent;

      if (now - _lastChanged > DEBOUNCE_DELAY)
      {
        if (_lastReading == LOW && _lowSince <= _highSince)
        {
          _lowSince = now;
          _duration = _lowSince - _highSince;

          if (_duration < LONG_PRESSED)
          {
            _event = ShortKeyUp;
          }
        }
        else if (_lastReading == HIGH)
        {
          if (_highSince <= _lowSince)
          {
            _longKeyDownSignaled = false;
            _highSince = now;
          }

          _duration = now - _highSince;

          if (_duration >= LONG_PRESSED && !_longKeyDownSignaled)
          {
            _event = LongKeyDown;
            _longKeyDownSignaled = true;
          }
        }
      }

      if (reading != _lastReading)
      {
        _lastChanged = now;
        _lastReading = reading;
      }
    }

    KeyEvent event()
    {
      return _event;
    }

  private:
    int _pin;
    int _lastReading;
    bool _longKeyDownSignaled;
    KeyEvent _event;
    unsigned long _duration;
    unsigned long _highSince;
    unsigned long _lowSince;
    unsigned long _lastChanged;

};

#endif
