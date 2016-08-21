#include <brewino/Screen.h>

void Screen::draw(Adafruit_ST7735 &hw)
{
  if (_changed)
  {
    hw.fillScreen(ST7735_BLACK);
    _changed = false;
  }
}

void Screen::enable()
{
  _current = this;
  _changed = true;
}

Screen *Screen::_current = NULL;
