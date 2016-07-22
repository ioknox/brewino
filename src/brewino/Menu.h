#ifndef __MENU_H__
#define __MENU_H__

#include <brewino/Screen.h>

#include "MenuItem.h"

class Menu : public Screen, public State
{
public:
  void setup(Fsm &stateMachine, State *parent, MenuItem *items, int count)
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

    _selectEvent.state_from = this;
    _selectEvent.state_to = _items[0].state_to;
    _selectEvent.instance = NULL;
    _selectEvent.event = SELECT_EVENT;

    stateMachine.add_transition(&_upEvent);
    stateMachine.add_transition(&_backEvent);
    stateMachine.add_transition(&_downEvent);
    stateMachine.add_transition(&_selectEvent);
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
    if (value >= 0)
    {
      _current = value % _count;
    }
    else
    {
      _current = _count - 1;
    }

    _selectEvent.state_to = _items[_current].state_to;
    _requireRefresh = true;
  }

  virtual void draw(TFT &hw)
  {
    Screen::draw(hw);

    if (_requireRefresh)
    {
      _requireRefresh = false;

      Color white(255, 255, 255);
      Color black(0, 0, 0);
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
        lbl.setValue(_items[i].text);
        lbl.draw(hw);
        position.y += Size_20x32 * 2;
        lbl.setPosition(position);
      }
    }
  }

private:
  bool _requireRefresh;
  MenuItem* _items;
  unsigned int _count;
  unsigned int _current;
  TTransition<Menu> _selectEvent;
  TTransition<Menu> _backEvent;
  TTransition<Menu> _upEvent;
  TTransition<Menu> _downEvent;
};

#endif
