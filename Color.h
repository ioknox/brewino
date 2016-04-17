#ifndef __COLOR_H__
#define __COLOR_H__

struct Color
{
  Color()
    : r(0), g(0), b(0) {
    // Nothing to do...
  }
  
  Color(int _r, int _g, int _b)
    : r(_r), g(_g), b(_b) {
    // Nothing to do...
  }
  Color(const Color& _other)
    : r(_other.r), g(_other.g), b(_other.b) {
    
  }
  int r;
  int g;
  int b;
};

#endif
