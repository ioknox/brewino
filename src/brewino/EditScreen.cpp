
#include <brewino/EditScreen.h>

const char undefineValue[] PROGMEM = { "?" };

EditScreen::EditScreen()
  : _displayLabel(5, 1, Point(0, 32), Color(0, 255, 0)),
    _editLabel(1, 0, Point(0, 32), Color(255, 255, 255)),
    _titleLabel(30, 0, Point(0, 0), Color(255, 255, 255))
{
    _titleLabel.setValue(undefineValue);
    _titleLabel.setFontSize(Size_20x32);

    _displayLabel.setValue(undefineValue);
    _displayLabel.setFontSize(Size_20x32);

    _editLabel.setValue(undefineValue);
    _editLabel.setFontSize(Size_20x32);
}

void EditScreen::on_enter()
{
  enable();
  _originalValue = *_value;
  _editState = 0;
  editValue();
}

void EditScreen::on_exit()
{

}

void EditScreen::draw(TFT& hw)
{
    Screen::draw(hw);

    _displayLabel.draw(hw);
    _editLabel.draw(hw);
    _titleLabel.draw(hw);
}

void EditScreen::commit()
{
  changeModifiedValue();
  disableEdit();
}

void EditScreen::rollback()
{
  *_value = _originalValue;
  disableEdit();
}

void EditScreen::up()
{
  _editDigit = (_editDigit + 1) % 10;
  _editLabel.setValue(_editDigit);
}

void EditScreen::down()
{
  _editDigit--;
  if (_editDigit < 0)
  {
    _editDigit = 10 + _editDigit;
  }
  _editLabel.setValue(_editDigit);
}

void EditScreen::next()
{
  changeModifiedValue();

  _editState = (_editState + 1) % 4;

  editValue();
}

void EditScreen::setup(Fsm &stateMachine, State *parent)
{
  _upEvent.state_from = this;
  _upEvent.state_to = this;
  _upEvent.instance = this;
  _upEvent.method = &EditScreen::up;
  _upEvent.event = UP_EVENT;

  _downEvent.state_from = this;
  _downEvent.state_to = this;
  _downEvent.instance = this;
  _downEvent.method = &EditScreen::down;
  _downEvent.event = DOWN_EVENT;

  _nextEvent.state_from = this;
  _nextEvent.state_to = this;
  _nextEvent.instance = this;
  _nextEvent.method = &EditScreen::next;
  _nextEvent.event = BACK_EVENT;

  _commitEvent.state_from = this;
  _commitEvent.state_to = parent;
  _commitEvent.instance = this;
  _commitEvent.method = &EditScreen::commit;
  _commitEvent.event = SELECT_EVENT;

  _rollbackEvent.state_from = this;
  _rollbackEvent.state_to = parent;
  _rollbackEvent.instance = this;
  _rollbackEvent.method = &EditScreen::rollback;
  _rollbackEvent.event = CANCEL_EVENT;

  stateMachine.add_transition(&_upEvent);
  stateMachine.add_transition(&_downEvent);
  stateMachine.add_transition(&_nextEvent);
  stateMachine.add_transition(&_commitEvent);
  stateMachine.add_transition(&_rollbackEvent);
}

void EditScreen::edit(const char *title, double &value)
{
  _value = &value;
  _titleLabel.setValue(title);
}

void EditScreen::edit(const char *title, bool &value)
{
  //_value = (double)value;
  _titleLabel.setValue(title);
}

void EditScreen::disableEdit()
{
  _editDigit = -1;
  _editLabel.setValue(_editDigit);
  _displayLabel.setRequireRefresh(true);
}

void EditScreen::changeModifiedValue()
{
  double value = (_editDigit - _editInit) * 10.0;
  value = round(value * pow(10.0, _editState - 1));
  *_value += (value / 10.0);
}

void EditScreen::editValue()
{
  _displayLabel.setValue((float)*_value);

  long value = (long)round(*_value * 10.0 / pow(10.0, _editState - 1.0));
  value = (value / 10L) % 10L;
  _editInit = (short)(value);

  short editPower = _editState - 1;

  _editDigit = _editInit;
  _editLabel.setValue(_editDigit);
  Point pt(_displayLabel.position());

  short decimals = _displayLabel.decimals();
  short pos = _displayLabel.size() - 1;

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
  _displayLabel.setRequireRefresh(true);
  _editLabel.setPosition(pt);
}
