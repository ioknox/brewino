#ifndef __BREWINO_EDITSCREENMENUITEM_H__
#define __BREWINO_EDITSCREENMENUITEM_H__

#include <brewino/EditScreen.h>
#include <brewino/MenuItem.h>

template <typename TValue>
struct EditScreenMenuItem : public MenuItem
{
  EditScreenMenuItem(State *screen, const char *text, TValue &value);
  void on_transition();
  TValue &value;
};

template <typename TValue>
EditScreenMenuItem<TValue>::EditScreenMenuItem(State *screen, const char *text, TValue &value)
  : MenuItem(screen, text), value(value)
{
  // Nothing to do...
}

template <typename TValue>
void EditScreenMenuItem<TValue>::on_transition()
{
  EditScreen *instance(static_cast<EditScreen*>(state_to));
  instance->edit(text, value);
}

#endif
