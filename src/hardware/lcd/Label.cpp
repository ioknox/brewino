#include "Label.h"

Label::Label(byte size, byte decimals, const Point &pt, const Color &foreColor)
  : _position(pt),
    _foreColor(foreColor),
    _fontSize(Size_5x8),
    _size(size),
    _decimals(decimals)
{
   // Nothing to do...
}

void spacing(String &left, int i, int s, char c)
{
  while (s > 1)
  {
    if (i > 9)
    {
      i /= 10;
    }
    else
    {
      left.concat(c);
    }

    s--;
  }
}

void Label::setValue(float value)
{
  String left;
  spacing(left, (int)value, _size - (_decimals + 1), ' ');
  setValue(left + String(value, _decimals));
}

void Label::setValue(int value)
{
  String left;
  spacing(left, (int)value, _size, ' ');
  setValue(left + String(value));
}

void Label::setValue(const char* value)
{
  int len(strlen_P(value));
  char buffer[len+1];
  strcpy_P(buffer, value);
  setValue(String(buffer));
}

void Label::setValue(const String& value)
{
  if (!_value.equals(value))
  {
    _value = value;
    _requireRefresh = true;
  }
}

void Label::setPosition(const Point &position)
{
  _position = position;
  _requireRefresh = true;
}

void Label::setForeColor(const Color& foreColor)
{
  _foreColor = foreColor;
  _requireRefresh = true;
}

void Label::setBackColor(const Color& backColor)
{
  _backColor = backColor;
  _requireRefresh = true;
}

void Label::setRequireRefresh(bool refresh)
{
  _requireRefresh = refresh;
  _requireRefresh = true;
}

void Label::setFontSize(FontSize fontSize)
{
  if (_fontSize != fontSize)
  {
    _fontSize = fontSize;
    _requireRefresh = true;
  }
}

void Label::draw(TFT &hw)
{
  if (!_requireRefresh)
  {
    return;
  }

  char valueBuf[_size + 1];
  _value.toCharArray(valueBuf, _size + 1);

  hw.noStroke();
  hw.fill(_backColor.r, _backColor.g, _backColor.b);
  hw.rect(_position.x, _position.y, _size * _fontSize * 6, _fontSize * 8);
  hw.noFill();
  hw.setTextSize(_fontSize);
  hw.stroke(_foreColor.r, _foreColor.g, _foreColor.b);
  hw.text(valueBuf, _position.x, _position.y);

  _requireRefresh = false;
}
