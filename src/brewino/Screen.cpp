#include <brewino/Screen.h>

void Screen::draw(TFT &hw)
{
  if (_changed)
  {
    hw.background(0, 0, 0);
    _changed = false;
  }
}

void Screen::enable()
{
  _current = this;
  _changed = true;
}

Screen *Screen::_current = NULL;
