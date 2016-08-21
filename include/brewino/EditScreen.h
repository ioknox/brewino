#ifndef __BREWINO_EDITSCREEN_H__
#define __BREWINO_EDITSCREEN_H__

#include <brewino/Screen.h>
#include <brewino/Events.h>

#include <Fsm.h>

class EditScreen : public Screen, public State
{
  public:
    EditScreen();
    virtual void on_enter();
    virtual void on_exit();
    virtual void draw(Adafruit_ST7735& hw);
    void commit();
    void rollback();
    void up();
    void down();
    void next();
    void setup(Fsm &stateMachine, State *parent);
    void edit(const char *title, double &value);
    void edit(const char *title, bool &value);

  private:
    Label _displayLabel;
    Label _editLabel;
    Label _titleLabel;
    TTransition<EditScreen> _upEvent;
    TTransition<EditScreen> _downEvent;
    TTransition<EditScreen> _nextEvent;
    TTransition<EditScreen> _commitEvent;
    TTransition<EditScreen> _rollbackEvent;

    double *_value;
    double _originalValue;
    short _editState;
    short _editInit;
    short _editDigit;

    void disableEdit();
    void changeModifiedValue();
    void editValue();
};

#endif
