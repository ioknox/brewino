#ifndef __MENUITEM_H__
#define __MENUITEM_H__

#include <Fsm.h>

struct MenuItem : public Transition
{
  const char *text;

  MenuItem(State *_state_to, const char *_text);
  virtual void on_transition();
};

#endif
