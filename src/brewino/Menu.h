#ifndef __MENU_H__
#define __MENU_H__

#include "Screen.h"
#include "MenuItem.h"

struct Menu : public Screen
{
  Menu(MenuItem *items, int count)
  {
    _items = items;
    _count = count;
  }

  template <typename TClass, TClass *ptr>
  static void initialize(Fsm &stateMachine, State *parent)
  {
    for (unsigned int i = 0; i < ptr->_count; i++)
    {
      unsigned int p = (i - 1) % ptr->_count;
      unsigned int n = (i + 1) % ptr->_count;

      MenuItem &current(ptr->_items[i]);
      MenuItem &previous(ptr->_items[p]);
      MenuItem &next(ptr->_items[n]);

      if (i == 0)
      {
        stateMachine.add_transition(parent, &current, SELECT_EVENT,
          &Adaptor<Menu, &Menu::begin, ptr>::bind);
      }

      stateMachine.add_transition(&current, &previous, UP_EVENT,
        &Adaptor<Menu, &Menu::up, ptr>::bind);
      if (current.state != nullptr)
      {
        stateMachine.add_transition(&current, current.state, SELECT_EVENT,
          &Adaptor<Menu, &Menu::end, ptr>::bind);
      }
      stateMachine.add_transition(&current, parent, BACK_EVENT, &Adaptor<Menu,
        &Menu::end, ptr>::bind);
      stateMachine.add_transition(&current, &next, DOWN_EVENT,
        &Adaptor<Menu, &Menu::down, ptr>::bind);
    }
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
      for (int i = 0; i < _count && position.y < hw.height(); i++)
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

  void begin()
  {
    _current = 0;
    _requireRefresh = true;
    enable();
  }

  void end()
  {
    _current = 0;
  }

  void up()
  {
    if (_current == 0)
    {
      _current = (_count - 1);
    }
    else
    {
      _current -= 1;
    }
    _requireRefresh = true;
  }

  void down()
  {
    _current = (_current + 1) % _count;
    _requireRefresh = true;
  }

private:
  bool _requireRefresh;
  MenuItem* _items;
  unsigned int _count;
  unsigned int _current;
};

#endif
