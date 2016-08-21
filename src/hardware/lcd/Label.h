#ifndef __LABEL_H__
#define __LABEL_H__

#include <Adafruit_ST7735.h>

#include <util/Point.h>

enum FontSize
{
  Size_5x8 = 1,
  Size_10x16 = 2,
  Size_15x24 = 3,
  Size_20x32 = 4,
  Size_25x40 = 5,
  Size_30x48 = 6,
  Size_35x56 = 7,
  Size_40x64 = 8
};

class Label
{
  public:
    Label(byte size, byte decimals, const Point &pt, const uint16_t &foreColor);

    inline const String& value() { return _value; }
    inline const Point& position() { return _position; }
    inline const uint16_t foreColor() { return _foreColor; }
    inline const uint16_t backColor() { return _backColor; }
    inline const int size() { return _size; }
    inline const int decimals() { return _decimals; }
    inline const bool requireRefresh() { return _requireRefresh; }
    inline const FontSize fontSize() { return _fontSize; }

    void setValue(float value);
    void setValue(int value);
    void setValue(const String& value);
    void setValue(const char *value);
    void setPosition(const Point &position);
    void setForeColor(const uint16_t& foreColor);
    void setBackColor(const uint16_t& backColor);
    void setRequireRefresh(bool refresh);
    void setFontSize(FontSize fontSize);

    virtual void draw(Adafruit_ST7735 &hw);

  private:
    Point _position;
    uint16_t _foreColor;
    uint16_t _backColor;
    FontSize _fontSize;
    String _value;
    bool _requireRefresh;
    byte _size;
    byte _decimals;
};

#endif
