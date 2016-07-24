#include <brewino/MainScreen.h>

MainScreen::MainScreen()
  : _consignLabel(5, 1, Point(0, 32), Color(0, 255, 0)),
    _currentTempLabel(5, 1, Point(0, 0), Color(255, 0, 0)),
    _outputLabel(5, 0, Point(0, 64), Color(0, 0, 255))
{
  _currentTempLabel.setValue("?");
  _currentTempLabel.setFontSize(Size_20x32);

  _consignLabel.setValue("?");
  _consignLabel.setFontSize(Size_20x32);

  _outputLabel.setValue("?");
  _outputLabel.setFontSize(Size_20x32);
}

void MainScreen::draw(TFT &hw)
{
  if (changed())
  {
    _consignLabel.setRequireRefresh(true);
    _currentTempLabel.setRequireRefresh(true);
    _outputLabel.setRequireRefresh(true);
  }

  Screen::draw(hw);

  _consignLabel.draw(hw);
  _currentTempLabel.draw(hw);
  _outputLabel.draw(hw);

  hw.stroke(255, 255, 255);
  if (_aliveFlag)
  {
    hw.fill(255, 255, 255);
  }
  else
  {
    hw.fill(0, 0, 0);
  }
  hw.rect(150, 120, 8, 8);
  _aliveFlag = !_aliveFlag;
}

void MainScreen::setOutput(float output)
{
  _outputLabel.setValue((int)output);
}

void MainScreen::setConsign(float consign)
{
  _consignLabel.setValue(consign);
}

void MainScreen::setTemp(float currentTemp)
{
  _currentTempLabel.setValue(currentTemp);
}
