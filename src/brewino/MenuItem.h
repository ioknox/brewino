#ifndef __MENUITEM_H__
#define __MENUITEM_H__

#include <Fsm.h>

struct MenuItem : public State
{
  State *state;
  String text;

  MenuItem(State *_state, const char *_text)
    : State(MenuItem::enter, MenuItem::leave)
  {
    state = _state;
    text = String(_text);
  }

  static void enter()
  {
  }

  static void leave()
  {
  }
};

#endif
