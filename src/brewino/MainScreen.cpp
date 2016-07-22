#include <brewino/MainScreen.h>

MainScreen::MainScreen()
  : _consignLabel(5, 1, Point(0, 32), Color(0, 255, 0)),
    _editLabel(1, 0, Point(0, 32), Color(255, 255, 255)),
    _currentTempLabel(5, 1, Point(0, 0), Color(255, 0, 0)),
    _outputLabel(5, 0, Point(0, 64), Color(0, 0, 255)),
    _editDigit(-1)
{
  _currentTempLabel.setValue("?");
  _currentTempLabel.setFontSize(Size_20x32);

  _consignLabel.setValue("?");
  _consignLabel.setFontSize(Size_20x32);

  _editLabel.setValue("?");
  _editLabel.setFontSize(Size_20x32);

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

  if (_editDigit >= 0) {
    if (_consignLabel.requireRefresh() && _editDigit >= 0)
    {
      _editLabel.setRequireRefresh(true);
    }
  }

  _consignLabel.draw(hw);
  _currentTempLabel.draw(hw);
  _outputLabel.draw(hw);

  if (_editDigit >= 0) {
    _editLabel.draw(hw);
  }

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

void MainScreen::editConsign(short editDigit, short editPower)
{
  _editDigit = editDigit;
  _editLabel.setValue(_editDigit);
  Point pt(_consignLabel.position());

  short decimals = _consignLabel.decimals();
  short pos = _consignLabel.size() - 1;

  if (editPower >= 0)
  {
    if (decimals > 0)
    {
      pos -= (decimals + 1);
    }
  }

  if (editPower < 0)
  {
    pos -= (decimals + editPower);
  }
  else
  {
    pos -= editPower;
  }

  pt.x = _editLabel.fontSize() * 6 * pos;
  _consignLabel.setRequireRefresh(true);
  _editLabel.setPosition(pt);
}

void MainScreen::incEditDigit()
{
  _editDigit = (_editDigit + 1) % 10;
  _editLabel.setValue(_editDigit);
}

void MainScreen::decEditDigit()
{
  _editDigit--;
  if (_editDigit < 0)
  {
    _editDigit = 10 + _editDigit;
  }
  _editLabel.setValue(_editDigit);
}

short MainScreen::editDigit()
{
  return _editDigit;
}

void MainScreen::disableEdit()
{
  _editDigit = -1;
  _editLabel.setValue(_editDigit);
  _consignLabel.setRequireRefresh(true);
}
