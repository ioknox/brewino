#ifndef __BREWINO_MAINSCREEN_H__
#define __BREWINO_MAINSCREEN_H__

#include <brewino/Events.h>
#include <brewino/Screen.h>

class MainScreen : public Screen
{
  public:
    MainScreen();
    virtual void draw(TFT &hw);
    void setOutput(float output);
    void setConsign(float consign);
    void setTemp(float currentTemp);
    void editConsign(short editDigit, short editPower);
    void incEditDigit();
    void decEditDigit();
    short editDigit();
    void disableEdit();

  private:
    Label _consignLabel;
    Label _editLabel;
    Label _currentTempLabel;
    Label _outputLabel;
    short _editDigit;
    bool _aliveFlag;
};

#endif
