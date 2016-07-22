#ifndef __MENUITEM_H__
#define __MENUITEM_H__

#include <Fsm.h>

struct MenuItem : public CBTransition
{
  String text;

  MenuItem(State *_state_to, const char *_text)
  {
    state_to = _state_to;
    text = String(_text);
  }
};

#endif
