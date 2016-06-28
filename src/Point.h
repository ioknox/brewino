#ifndef __POINT_H__
#define __POINT_H__

struct Point
{
  Point(int _x, int _y) 
    : x(_x), y(_y) {
      // Nothing to do...
  }
  Point(const Point &pt) 
    : x(pt.x), y(pt.y) {
      // Nothing to do...
  }
  int x;
  int y;
};

#endif
