#ifndef __BREWINO_MAINSCREEN_H__
#define __BREWINO_MAINSCREEN_H__

#include <brewino/Screen.h>

class MainScreen : public Screen
{
  public:
    MainScreen();
    virtual void draw(TFT &hw);
    void setOutput(float output);
    void setConsign(float consign);
    void setTemp(float currentTemp);

  private:
    Label _consignLabel;
    Label _currentTempLabel;
    Label _outputLabel;
    bool _aliveFlag;
};

#endif
