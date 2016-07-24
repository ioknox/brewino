#include <brewino/MenuItem.h>

MenuItem::MenuItem(State *_state_to, const char *_text)
  : Transition()
{
  state_to = _state_to;
  text = _text;
}

void MenuItem::on_transition()
{
  
}
