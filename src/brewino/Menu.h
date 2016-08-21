#ifndef __MENU_H__
#define __MENU_H__

#include <brewino/Events.h>
#include <brewino/Screen.h>
#include <brewino/MenuItem.h>

class Menu : public Screen, public State
{
public:
  Menu()
    :  _count(0), _current(0)
  {

  }

  void setup(Fsm &stateMachine, State *parent, MenuItem** items, int count)
  {
    _count = count;
    _items = items;

    _upEvent.state_from = this;
    _upEvent.state_to = this;
    _upEvent.instance = this;
    _upEvent.method = &Menu::up;
    _upEvent.event = UP_EVENT;

    _downEvent.state_from = this;
    _downEvent.state_to = this;
    _downEvent.instance = this;
    _downEvent.method = &Menu::down;
    _downEvent.event = DOWN_EVENT;

    _backEvent.state_from = this;
    _backEvent.state_to = parent;
    _backEvent.instance = NULL;
    _backEvent.event = BACK_EVENT;

    for (unsigned int i = 0; i < _count; i++)
    {
      _items[i]->state_from = this;
      _items[i]->event = NO_EVENT;

      stateMachine.add_transition(_items[i]);
    }

    _items[0]->event = SELECT_EVENT;

    stateMachine.add_transition(&_upEvent);
    stateMachine.add_transition(&_backEvent);
    stateMachine.add_transition(&_downEvent);
  }

  virtual void on_enter()
  {
    if (!is_current())
    {
      set_current(0);
      enable();
    }
  }

  virtual void on_exit()
  {
    // Nothing to do...
  }

  void up()
  {
    set_current(_current - 1);
  }

  void down()
  {
    set_current(_current + 1);
  }

  void set_current(unsigned int value)
  {
    _items[_current]->event = NO_EVENT;

    if (value >= 0)
    {
      _current = value % _count;
    }
    else
    {
      _current = _count - 1;
    }
    _items[_current]->event = SELECT_EVENT;
    _requireRefresh = true;
  }

  virtual void draw(Adafruit_ST7735 &hw)
  {
    Screen::draw(hw);

    if (_requireRefresh)
    {
      _requireRefresh = false;

      uint16_t white(ST7735_WHITE);
      uint16_t black(ST7735_BLACK);
      Point position(0, 0);

      Label lbl(hw.width() / Size_20x32, 0, position, white);
      for (unsigned int i = 0; i < _count && position.y < hw.height(); i++)
      {
        if (i == _current)
        {
          lbl.setForeColor(black);
          lbl.setBackColor(white);
        }
        else
        {
          lbl.setForeColor(white);
          lbl.setBackColor(black);
        }

        lbl.setValue(_items[i]->text);
        lbl.draw(hw);
        position.y += Size_20x32 * 2;
        lbl.setPosition(position);
      }
    }
  }

private:
  bool _requireRefresh;
  MenuItem** _items;
  unsigned int _count;
  unsigned int _current;
  TTransition<Menu> _backEvent;
  TTransition<Menu> _upEvent;
  TTransition<Menu> _downEvent;
};

#endif
