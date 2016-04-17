#ifndef __LABEL_H__
#define __LABEL_H__

#include <TFT.h>

#include "Point.h"
#include "Color.h"

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
    Label(byte size, byte decimals, const Point &pt, const Color &foreColor);
    void setValue(float value);
    void setValue(int value);
    void setValue(const String& value);
    inline const String& value() { return _value; }
    inline const Point& position() { return _position; }
    inline void setPosition(const Point &position) { _position = position; }
    inline const Color& foreColor() { return _foreColor; }
    inline void setForeColor(const Color& foreColor) { _foreColor = foreColor; }
    inline const Color& backColor() { return _backColor; }
    inline void setBackColor(const Color& backColor) { _backColor = backColor; }
    inline int size() { return _size; }
    inline int decimals() { return _decimals; }
    inline bool requireRefresh() { return _requireRefresh; }
    inline bool setRequireRefresh(bool refresh) { _requireRefresh = refresh; }
    FontSize fontSize();
    void setFontSize(FontSize fontSize);
    virtual void draw(TFT &hw);
  
  private:
    Point _position;
    Color _foreColor;
    Color _backColor;
    FontSize _fontSize;
    String _value;
    bool _requireRefresh;
    byte _size;
    byte _decimals;
};

#endif __LABEL_H__
