#include <Arduino.h>

#include <SPI.h>
#include <TFT.h>
#include <PID_v1.h>
#include <Servo.h>
#include <TaskScheduler.h>
#include <Fsm.h>

#define SELECT_EVENT 1
#define CANCEL_EVENT 2
#define UP_EVENT 3
#define DOWN_EVENT 4
#define BACK_EVENT 5

#define TFT_CS    19
#define TFT_DC    9
#define TFT_RESET 8

#define TC_CS     10

#include <brewino/Screen.h>
#include <brewino/KeyPad.h>
#include <brewino/Settings.h>

Servo myservo;
int val;

State idle(NULL, NULL);
Fsm screenFsm(&idle);

void shortCallback();
void longCallback();

Scheduler sched;
Task shortTask(5, TASK_FOREVER, &shortCallback);
Task longTask(500, TASK_FOREVER, &longCallback);

double input = 0.0;
double output = 0.0;
Settings settings;
KeyPad keyPad;
Screen mainScreen;

TFT tft(TFT_CS, TFT_DC, TFT_RESET);
PID pid(
  &input,
  &output,
  &settings.consign,
  settings.proportional,
  settings.integral,
  settings.derivate,
  DIRECT
);

void shortCallback()
{
  keyPad.loop();

  ButtonsEnum longEvt = keyPad.event(LongKeyDown);
  ButtonsEnum shortEvt = keyPad.event(ShortKeyUp);

  if (shortEvt != NoButton || longEvt != NoButton)
  {
    Serial.print("LONG:");
    Serial.print(longEvt);
    Serial.print(" SHORT:");
    Serial.println(shortEvt);
  }

  if ((longEvt & SetButton) != 0)
  {
    screenFsm.trigger(CANCEL_EVENT);
  }
  else if ((shortEvt & SetButton) != 0)
  {
    screenFsm.trigger(SELECT_EVENT);
  }
  else if ((shortEvt & UpButton) != 0)
  {
    screenFsm.trigger(UP_EVENT);
  }
  else if ((shortEvt & DownButton) != 0)
  {
    screenFsm.trigger(DOWN_EVENT);
  }
  else if ((shortEvt & RightButton) != 0)
  {
    screenFsm.trigger(BACK_EVENT);
  }
}

void longCallback()
{
  float voltage = (analogRead(A3) * 3.3f) / 1024.0f;
  input = (voltage - 0.5f) * 100.0f;
  pid.Compute();

  myservo.write(map(output, 0, 2000, 0, 100));

  mainScreen.setConsign(settings.consign);
  mainScreen.setTemp(input);
  mainScreen.setOutput(output);
  mainScreen.draw(tft);
}

class ConsignManager
{
public:
  static void enter()
  {
    Serial.println(" *** ENTER EDIT CONSIGN *** ");
    _originalConsign = settings.consign;
    _editState = 0;
    editConsign(settings.consign, mainScreen);
  }

  static void commit()
  {
    Serial.println(" *** COMIT CONSIGN *** ");
    changeModifiedConsign(settings.consign, mainScreen);
    mainScreen.disableEdit();
  }

  static void rollback()
  {
    Serial.println(" *** ROLLBACK CONSIGN *** ");
    settings.consign = _originalConsign;
    mainScreen.disableEdit();
  }

  static void up()
  {
    Serial.println(" *** UP *** ");
    mainScreen.incEditDigit();
  }

  static void down()
  {
    Serial.println(" *** DOWN *** ");
    mainScreen.decEditDigit();
  }

  static void next()
  {
    Serial.println(" *** NEXT *** ");
    changeModifiedConsign(settings.consign, mainScreen);
    nextDigit();
    editConsign(settings.consign, mainScreen);
  }

private:
  static double _originalConsign;
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

double ConsignManager::_originalConsign = 0.0;
short ConsignManager::_editState = 0;
short ConsignManager::_editInit = 0;

struct MenuItem
{
  State *state;
  String text;

  void set(State *_state, const char *_text)
  {
    state = _state;
    text = String(_text);
  }
};

struct Menu : State
{
  Menu(MenuItem[] items)
  {

  }

  static void exit()
  {

  }

  static void begin()
  {
  }

  static void select()
  {
  }

  static void back()
  {
  }

  static void down()
  {
  }

  static void up()
  {
  }

private:
  MenuItem items[];
};

State mainMenu(NULL, NULL);
State editConsign(NULL, NULL);
State editSettings(NULL, NULL);

MenuItem items[2];
items[0].set(&editConsign, "EDIT CONSIGN");
items[1].set(&idle, "EXIT");

void setup() {
  screenFsm.add_transition(&idle, &mainMenu, SELECT_EVENT, &MainMenu::begin);
  screenFsm.add_transition(&editConsign, &idle, SELECT_EVENT, &ConsignManager::commit);
  screenFsm.add_transition(&editConsign, &idle, CANCEL_EVENT, &ConsignManager::rollback);
  screenFsm.add_transition(&editConsign, &editConsign, UP_EVENT, &ConsignManager::up);
  screenFsm.add_transition(&editConsign, &editConsign, DOWN_EVENT, &ConsignManager::down);
  screenFsm.add_transition(&editConsign, &editConsign, BACK_EVENT, &ConsignManager::next);

  Serial.begin(57600);

  tft.begin();
  tft.background(0, 0, 0);

  analogReference(EXTERNAL);

  pid.SetOutputLimits(0.0, 2000.0);
  pid.SetMode(AUTOMATIC);

  input = 0.0;
  output = 0.0;
  //consign = 32.0;

  myservo.attach(3);

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

/*
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
*/
/*
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

    static void shortTask()
    {
      keyPad.loop();

      ButtonsEnum longEvt = keyPad.event(LongKeyDown);
      ButtonsEnum shortEvt = keyPad.event(ShortKeyUp);

      if (shortEvt != NoButton || longEvt != NoButton)
      {
        Serial.print("LONG:");
        Serial.print(longEvt);
        Serial.print(" SHORT:");
        Serial.println(shortEvt);
      }

      if ((longEvt & SetButton) != 0)
      {
        screenFsm.trigger(CANCEL_EVENT);
      }
      else if ((shortEvt & SetButton) != 0)
      {
        screenFsm.trigger(SELECT_EVENT);
      }
      else if ((shortEvt & UpButton) != 0)
      {
        screenFsm.trigger(UP_EVENT);
      }
      else if ((shortEvt & DownButton) != 0)
      {
        screenFsm.trigger(DOWN_EVENT);
      }
      else if ((shortEvt & RightButton) != 0)
      {
        screenFsm.trigger(BACK_EVENT);
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

    PID _pid;
    prg::StateEnum _state;
};

void shortCallback();
void longCallback();

Program prog;

*/
