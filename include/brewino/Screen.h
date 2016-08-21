#ifndef __BREWINO_SCREEN_H__
#define __BREWINO_SCREEN_H__

#include <hardware/lcd/Label.h>
#include <Fsm.h>

#include "ButtonsEnum.h"

class Screen
{
public:
  virtual void draw(Adafruit_ST7735 &hw);
  void enable();
  inline bool is_current() { return this == _current; }
  inline bool changed() { return _changed; }
  inline static Screen *current() { return _current; }

private:
  static Screen *_current;
  bool _changed;
};

#endif
