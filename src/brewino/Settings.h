#ifndef __BREWINO_SETTINGS_H__
#define __BREWINO_SETTINGS_H__

struct Settings
{
public:
  double proportional;
  double integral;
  double derivate;
  bool automatic;

  double consign;
};

#endif
