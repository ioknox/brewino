#include <brewino/MainScreen.h>

MainScreen::MainScreen()
  : _consignLabel(5, 1, Point(0, 32), ST7735_GREEN),
    _currentTempLabel(5, 1, Point(0, 0), ST7735_RED),
    _outputLabel(5, 0, Point(0, 64), ST7735_BLUE)
{
  _currentTempLabel.setValue("?");
  _currentTempLabel.setFontSize(Size_20x32);

  _consignLabel.setValue("?");
  _consignLabel.setFontSize(Size_20x32);

  _outputLabel.setValue("?");
  _outputLabel.setFontSize(Size_20x32);
}

void MainScreen::draw(Adafruit_ST7735 &hw)
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

  uint16_t color(ST7735_BLACK);
  if (_aliveFlag)
  {
    color = ST7735_WHITE;
  }

  hw.fillRect(150, 120, 8, 8, color);
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
