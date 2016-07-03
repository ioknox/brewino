#ifndef __BREWINO_SETTINGS_H__
#define __BREWINO_SETTINGS_H__

struct Settings
{
  Settings()
  {
    proportional = 2;
    integral = 5;
    derivate = 1;
    automatic = true;
  }

  double proportional;
  double integral;
  double derivate;
  bool automatic;

  double consign;
};

#endif
