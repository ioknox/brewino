#ifndef __BREWINO_KEYPAD_H__
#define __BREWINO_KEYPAD_H__

#include <hardware/Button.h>

#include "ButtonsEnum.h"

#define KEYPAD_BUTTONS 4

class KeyPad
{
  public:
    KeyPad()
    {
      _buttons[0] = new Button(2);
      _buttons[1] = new Button(4);
      _buttons[2] = new Button(6);
      _buttons[3] = new Button(5);
    }

    void loop()
    {
      for (int i = 0; i < KEYPAD_BUTTONS; i++)
      {
        _buttons[i]->loop();
      }
    }

    ButtonsEnum event(KeyEvent keyEvent)
    {
      int matchButtons = 0;

      for (int i = 0; i < KEYPAD_BUTTONS; i++)
      {
        if (_buttons[i]->event() == keyEvent)
        {
          matchButtons |= (1 << i);
        }
      }

      return (ButtonsEnum)matchButtons;
    }

  private:
    Button *_buttons[KEYPAD_BUTTONS];
};

#endif
