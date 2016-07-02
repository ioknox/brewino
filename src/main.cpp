#include <Arduino.h>


#include <SPI.h>
#include <TFT.h>
#include <PID_v1.h>
#include <Servo.h>
#include <TaskScheduler.h>

#define TFT_CS    19
#define TFT_DC    9
#define TFT_RESET 8

#define TC_CS     10

#include <brewino/Screen.h>


#define DEBOUNCE_DELAY 30
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
          changeModifiedConsign(consign, screen);
          _running = false;
        }
        else if ((shortEvt & UpButton) != 0)
        {
          screen.incEditDigit();
        }
        else if ((shortEvt & DownButton) != 0)
        {
          screen.decEditDigit();
        }
        else if ((shortEvt & RightButton) != 0)
        {
          changeModifiedConsign(consign, screen);
          nextDigit();
          editConsign(consign, screen);
        }



        if (shortEvt != NoButton || longEvt != NoButton)
        {
          Serial.print("consign:");
          Serial.print(consign);
          Serial.print(" state:");
          Serial.println(_editState);
          Serial.print(" init:");
          Serial.println(_editInit);
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
        _tft(TFT_CS, TFT_DC, TFT_RESET),
        _pid(&_input, &_output, &_consign, _p, _i, _d, DIRECT)
    {
      // Nothing to do...
    }

    void setup() {
      Serial.begin(57600);

      _tft.begin();
      _tft.background(0, 0, 0);

      analogReference(EXTERNAL);

      _pid.SetOutputLimits(0.0, 2000.0);
      _pid.SetMode(AUTOMATIC);

      _input = 0.0;
      _output = 0.0;
      _consign = 32.0;

      myservo.attach(3);

      _state = prg::Idle;
    }

    void shortTask() {
      _keyPad.loop();

      ButtonsEnum longEvt = _keyPad.event(LongKeyDown);
      ButtonsEnum shortEvt = _keyPad.event(ShortKeyUp);

      if (shortEvt != NoButton || longEvt != NoButton)
      {
        Serial.print("LONG:");
        Serial.print(longEvt);
        Serial.print(" SHORT:");
        Serial.println(shortEvt);
      }

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
      float voltage = (analogRead(A3) * 3.3f) / 1024.0f;
      _input = (voltage - 0.5f) * 100.0f;
      _pid.Compute();

      myservo.write(map(_output, 0, 2000, 0, 100));

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
    Screen _mainScreen;
    KeyPad _keyPad;
    PID _pid;
    prg::StateEnum _state;
};

void shortCallback();
void longCallback();

Program prog;
Scheduler sched;
Task shortTask(5, TASK_FOREVER, &shortCallback);
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
