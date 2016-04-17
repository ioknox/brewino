#include <SPI.h>
#include <TFT.h>
#include <PID_v1.h>
#include <Servo.h>
#include <TaskScheduler.h>

#include "TempSensor.h"
#include "Label.h"

#define CS   10
#define DC   9
#define RESET  8

#define RATE 100L

enum ButtonsEnum
{
  NoButton = 0,
  SetButton = 1,
  UpButton = 2,
  DownButton = 4,
  RightButton = 8
};

/*
  class TextBox : public Label
  {
  public:
    TextBox(const Point &pt, const Color &foreColor)
      : Label(pt, foreColor), _currentPart(-1)
    {
      // Nothing to do...
    }

    virtual void draw(TFT &hw)
    {
      Label::draw(hw);

      if (_currentPart >= 0)
      {
        int x =
        hw.noStroke();
        hw.fill(_foreColor.r, _foreColor.g, _foreColor.b);
        hw.rect(_position.x + _fontSize * 5, _position.y, _fontSize * 5, _fontSize * 8);
        hw.noFill();
        hw.setTextSize(_fontSize);
        hw.stroke(_backColor.r, _backColor.g, _backColor.b);
        hw.text(charBuffer, _position.x + _fontSize * 5 * _currentPart, _position.y);
      }
    }

    void setEditMode(bool edition)
    {
      if (edition && _currentPart < 0)
      {
        _currentPart = 0;
      }
      else
      {
        _currentPart = -1;
      }
    }

    void moveLeft()
    {

    }

    void moveRight()
    {

    }

    void next()
    {

    }

    void previous()
    {

    }

  private:
    int _currentPart;
    float _value;
  };*/


class Screen
{
  public:
    Screen()
      : _consignLabel(5, 1, Point(0, 32), Color(0, 255, 0)),
        _editLabel(1, 0, Point(0, 32), Color(255, 255, 255)),
        _currentTempLabel(5, 1, Point(0, 0), Color(255, 0, 0)),
        _outputLabel(5, 0, Point(0, 64), Color(0, 0, 255)),
        _editDigit(-1)
    {
      _currentTempLabel.setValue("?");
      _currentTempLabel.setFontSize(Size_20x32);

      _consignLabel.setValue("?");
      _consignLabel.setFontSize(Size_20x32);

      _editLabel.setValue("?");
      _editLabel.setFontSize(Size_20x32);

      _outputLabel.setValue("?");
      _outputLabel.setFontSize(Size_20x32);
    }

    virtual void draw(TFT &hw)
    {
      if (_editDigit >= 0) {
        if (_consignLabel.requireRefresh())
        {
          _editLabel.setRequireRefresh(true);
        }

        if (_editLabel.requireRefresh())
        {
          _consignLabel.setRequireRefresh(true);
        }
      }

      _consignLabel.draw(hw);
      _currentTempLabel.draw(hw);
      _outputLabel.draw(hw);

      if (_editDigit >= 0) {
        _editLabel.draw(hw);
      }
    }

    void setOutput(float output)
    {
      _outputLabel.setValue((int)output);
    }

    void setConsign(float consign)
    {
      _consignLabel.setValue(consign);
    }

    void setTemp(float currentTemp)
    {
      _currentTempLabel.setValue(currentTemp);
    }

    void editConsign(short editDigit, short editPower)
    {
      _editDigit = editDigit;
      _editLabel.setValue(_editDigit);
      Point pt(_consignLabel.position());

      short decimals = _consignLabel.decimals();
      short pos = _consignLabel.size() - 1;

      if (editPower >= 0)
      {
        if (decimals > 0)
        {
          pos -= (decimals + 1);
        }
      }

      if (editPower < 0)
      {
        pos -= (decimals + editPower);
      }
      else
      {
        pos -= editPower;
      }

      pt.x = _editLabel.fontSize() * 6 * pos;
      _editLabel.setPosition(pt);
    }

    void incEditDigit()
    {
      _editDigit = (_editDigit + 1) % 10;
      _editLabel.setValue(_editDigit);
    }

    void decEditDigit()
    {
      _editDigit--;
      if (_editDigit < 0)
      {
        _editDigit = 10 + _editDigit;
      }
      _editLabel.setValue(_editDigit);
    }

    short editDigit()
    {
      return _editDigit;
    }

    void disableEdit()
    {
      _editDigit = -1;
      _editLabel.setValue(_editDigit);
    }

    /*
      void editConsign(float consign)
      {
      _editPos = 0;
      _editValue = consign;

      refreshEditConsign();
      }

      int editDigit()
      {
      return (int)(_editValue * pow(10, _editPos + 1)) % 10;
      }

      void refreshEditConsign()
      {
      int digit = ;
      _editLabel.setValue(digit);
      Point pt(_consignLabel.position());
      pt.x = _editLabel.fontSize() * 6 * (_consignLabel.value().length() - (_editPos + 1));
      _editLabel.setPosition(pt);
      }

      void moveEditPos()
      {
      _editPos = (_editPos + 1);
      refreshEditConsign();
      }

      void incEditValue()
      {
      _editDigit = (_editDigit + 1) % 10;
      _editLabel.setValue(_editDigit);
      }

      void decEditValue()
      {
      _editDigit--;
      if (_editDigit < 0)
      {
        _editDigit = 10 + _editDigit;
      }
      _editLabel.setValue(_editDigit);
      }

      float editValue()
      {
      return _editValue;
      }
    */

  private:
    Label _consignLabel;
    Label _editLabel;
    Label _currentTempLabel;
    Label _outputLabel;
    short _editDigit;
};

#define DEBOUNCE_DELAY 50
#define LONG_PRESSED 1000

enum KeyEvent
{
  NoKeyEvent,
  ShortKeyUp,
  LongKeyDown
};

class Button
{
  public:
    Button(int pin)
      : _pin(pin),
        _lastReading(0),
        _longKeyDownSignaled(false),
        _event(NoKeyEvent),
        _duration(0),
        _highSince(0),
        _lowSince(0),
        _lastChanged(0)
    {
      unsigned long now(millis());

      _highSince = now;
      _lowSince = now;
      _lastChanged = now;
      loop();

      pinMode(pin, INPUT);
    }

    void loop()
    {
      unsigned long now(millis());
      int reading = digitalRead(_pin);

      _event = NoKeyEvent;

      if (now - _lastChanged > DEBOUNCE_DELAY)
      {
        if (_lastReading == LOW && _lowSince <= _highSince)
        {
          _lowSince = now;
          _duration = _lowSince - _highSince;

          if (_duration < LONG_PRESSED)
          {
            _event = ShortKeyUp;
          }
        }
        else if (_lastReading == HIGH)
        {
          if (_highSince <= _lowSince)
          {
            _longKeyDownSignaled = false;
            _highSince = now;
          }

          _duration = now - _highSince;

          if (_duration >= LONG_PRESSED && !_longKeyDownSignaled)
          {
            _event = LongKeyDown;
            _longKeyDownSignaled = true;
          }
        }
      }

      if (reading != _lastReading)
      {
        _lastChanged = now;
        _lastReading = reading;
      }
    }

    KeyEvent event()
    {
      return _event;
    }

  private:
    int _pin;
    int _lastReading;
    bool _longKeyDownSignaled;
    KeyEvent _event;
    unsigned long _duration;
    unsigned long _highSince;
    unsigned long _lowSince;
    unsigned long _lastChanged;

};

#define KEYPAD_BUTTONS 4

class KeyPad
{
  public:
    KeyPad()
    {
      _buttons[0] = new Button(2);
      _buttons[1] = new Button(3);
      _buttons[2] = new Button(4);
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
    Button *_buttons[4];
};

namespace prg
{
  enum StateEnum
  {
    Idle,
    EditConsign
  };
}

class UpdateConsignStateMachine
{
  public:
    static bool run(double &consign, ButtonsEnum shortEvt, ButtonsEnum longEvt, Screen &screen)
    {
      if (!_running)
      {
        _running = true;
        _editState = 0;
        _originalConsign = consign;
        editConsign(consign, screen);
      }
      else
      {
        if ((longEvt & SetButton) != 0)
        {
          consign = _originalConsign;
          _running = false;
        }
        else if ((shortEvt & SetButton) != 0)
        {
          _running = false;
        }
        else if ((shortEvt & UpButton) != 0)
        {
          screen.incEditDigit();
          changeModifiedConsign(consign, screen);
        }
        else if ((shortEvt & DownButton) != 0)
        {
          screen.decEditDigit();
          changeModifiedConsign(consign, screen);
        }
        else if ((shortEvt & RightButton) != 0)
        {
          changeModifiedConsign(consign, screen);
          nextDigit();
          editConsign(consign, screen);
        }
      }

      if (!_running)
      {
        screen.disableEdit();
      }

      return _running;
    }

  private:
    static double _originalConsign;
    static bool _running;
    static short _editState;
    static short _editInit;

    static void changeModifiedConsign(double &consign, Screen &screen)
    {
      consign += (screen.editDigit() - _editInit) * pow(10.0, _editState - 1);
    }

    static void nextDigit()
    {
      _editState = (_editState + 1) % 4;
    }

    static void editConsign(double &consign, Screen &screen)
    {
      _editInit = (short)(consign / pow(10.0, _editState - 1)) % 10;
      screen.editConsign(_editInit, _editState - 1);
    }
};

double UpdateConsignStateMachine::_originalConsign = 0.0;
bool UpdateConsignStateMachine::_running = false;
short UpdateConsignStateMachine::_editState = 0;
short UpdateConsignStateMachine::_editInit = 0;

Servo myservo;
int val;

class Program
{
  public:

    Program()
      : _p(2), _i(5), _d(1),
        _input(0), _output(0), _consign(0),
        _tft(CS, DC, RESET),
        _tempSensor(A1, 3300.0f),
        _pid(&_input, &_output, &_consign, _p, _i, _d, DIRECT)
    {
      // Nothing to do...
    }

    void setup() {
      Serial.begin(9600);
  
      analogReference(EXTERNAL);

      _tft.begin();
      _tft.background(0, 0, 0);

      _pid.SetOutputLimits(0.0, 2000.0);
      _pid.SetMode(AUTOMATIC);

      _input = 0.0;
      _output = 0.0;
      _consign = 32.0;

      myservo.attach(6);

      _state = prg::Idle;
    }

    void shortTask() {
      _keyPad.loop();

      ButtonsEnum longEvt = _keyPad.event(LongKeyDown);
      ButtonsEnum shortEvt = _keyPad.event(ShortKeyUp);

      switch (_state)
      {
        case prg::Idle:
          if ((longEvt & SetButton) != 0)
          {
            _state = prg::EditConsign;
          }
          break;
        case prg::EditConsign:
          if (!UpdateConsignStateMachine::run(_consign, shortEvt, longEvt, _mainScreen))
          {
            _state = prg::Idle;
          }
          break;
      }
    }

    void longTask() {
      _tempSensor.loop();

      _input = _tempSensor.value();
      _pid.Compute();

      myservo.write((int)_output);

      _mainScreen.setConsign(_consign);
      _mainScreen.setTemp(_input);
      _mainScreen.setOutput(_output);
      _mainScreen.draw(_tft);
    }

  private:
    double _p;
    double _i;
    double _d;
    double _input;
    double _consign;
    double _output;
    TFT _tft;
    TempSensor _tempSensor;
    Screen _mainScreen;
    KeyPad _keyPad;
    PID _pid;
    prg::StateEnum _state;
};

void shortCallback();
void longCallback();

Program prog;
Scheduler sched;
Task shortTask(10, TASK_FOREVER, &shortCallback);
Task longTask(500, TASK_FOREVER, &longCallback);

void shortCallback()
{
  prog.shortTask();
}

void longCallback()
{
  prog.longTask();
}

void setup() {
  prog.setup();
  sched.init();
  sched.addTask(shortTask);
  sched.addTask(longTask);
  shortTask.enable();
  longTask.enable();
}

void loop()
{
  sched.execute();
}

