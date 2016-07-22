#include <math.h>

#include <Arduino.h>

#include <SPI.h>
#include <TFT.h>
#include <PID_v1.h>
#include <Servo.h>
#include <TaskScheduler.h>
#include <Fsm.h>

#include <brewino/MainScreen.h>
#include <brewino/Menu.h>
#include <brewino/Screen.h>
#include <brewino/KeyPad.h>
#include <brewino/Settings.h>

#define TFT_CS    19
#define TFT_DC    9
#define TFT_RESET 8

#define TC_CS     10

void enterIdle();
void shortCallback();
void longCallback();

Servo myservo;
int val;

Scheduler sched;
Task shortTask(5, TASK_FOREVER, &shortCallback);
Task longTask(500, TASK_FOREVER, &longCallback);

double input = 0.0;
double output = 0.0;
Settings settings;
KeyPad keyPad;
MainScreen mainScreen;

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

CBState idle(enterIdle, NULL);
Fsm screenFsm(&idle);

void enterIdle()
{
  mainScreen.enable();
}

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

  Screen::current()->draw(tft);
}

struct ConsignManager : public State
{
public:
  ConsignManager()
   : _originalConsign(0.0), _editState(0), _editInit(0)
  {

  }

  virtual void on_enter()
  {
    if (!mainScreen.is_current())
    {
      mainScreen.enable();
      _originalConsign = settings.consign;
      _editState = 0;
      editConsign(settings.consign, mainScreen);
    }
  }

  virtual void on_exit()
  {
  }

  void commit()
  {
    changeModifiedConsign(settings.consign, mainScreen);
    mainScreen.disableEdit();
  }

  void rollback()
  {
    settings.consign = _originalConsign;
    mainScreen.disableEdit();
  }

  void up()
  {
    mainScreen.incEditDigit();
  }

  void down()
  {
    mainScreen.decEditDigit();
  }

  void next()
  {
    changeModifiedConsign(settings.consign, mainScreen);
    nextDigit();
    editConsign(settings.consign, mainScreen);
  }

  void setup(Fsm &stateMachine, State *parent)
  {
    _upEvent.state_from = this;
    _upEvent.state_to = this;
    _upEvent.instance = this;
    _upEvent.method = &ConsignManager::up;
    _upEvent.event = UP_EVENT;

    _downEvent.state_from = this;
    _downEvent.state_to = this;
    _downEvent.instance = this;
    _downEvent.method = &ConsignManager::down;
    _downEvent.event = DOWN_EVENT;

    _nextEvent.state_from = this;
    _nextEvent.state_to = this;
    _nextEvent.instance = this;
    _nextEvent.method = &ConsignManager::next;
    _nextEvent.event = BACK_EVENT;

    _commitEvent.state_from = this;
    _commitEvent.state_to = parent;
    _commitEvent.instance = this;
    _commitEvent.method = &ConsignManager::commit;
    _commitEvent.event = SELECT_EVENT;

    _rollbackEvent.state_from = this;
    _rollbackEvent.state_to = parent;
    _rollbackEvent.instance = this;
    _rollbackEvent.method = &ConsignManager::rollback;
    _rollbackEvent.event = CANCEL_EVENT;

    stateMachine.add_transition(&_upEvent);
    stateMachine.add_transition(&_downEvent);
    stateMachine.add_transition(&_nextEvent);
    stateMachine.add_transition(&_commitEvent);
    stateMachine.add_transition(&_rollbackEvent);
  }

private:
  TTransition<ConsignManager> _upEvent;
  TTransition<ConsignManager> _downEvent;
  TTransition<ConsignManager> _nextEvent;
  TTransition<ConsignManager> _commitEvent;
  TTransition<ConsignManager> _rollbackEvent;
  double _originalConsign;
  short _editState;
  short _editInit;

  void changeModifiedConsign(double &consign, MainScreen &screen)
  {
    double value = (screen.editDigit() - _editInit) * 10.0;
    value = round(value * pow(10.0, _editState - 1));
    consign += (value / 10.0);
  }

  void nextDigit()
  {
    _editState = (_editState + 1) % 4;
  }

  void editConsign(double &consign, MainScreen &screen)
  {
    long value = (long)round(consign * 10.0 / pow(10.0, _editState - 1.0));
    value = (value / 10L) % 10L;
    _editInit = (short)(value);

    screen.editConsign(_editInit, _editState - 1);
  }
};

class EditScreen : public Screen, public State
{
  public:
    virtual void on_enter()
    {
      enable();
    }

    virtual void on_exit()
    {

    }

    virtual void draw(TFT& hw)
    {

    }

    void up()
    {
      Serial.println("EDIT SCREEN UP BUTTON PRESSED. GO SLEEP");
    }

    void down()
    {
      Serial.println("REALLY DUDE...");
    }

    void setup()
    {

    }

  private:
    //TTransition<EditScreen>
};

ConsignManager editConsignState;
EditScreen editKp;

Menu mainMenu;
Menu settingMenu;

MenuItem mainMenuItems[] =
{
  MenuItem(&editConsignState, "Edit consign"),
  MenuItem(&settingMenu, "Edit settings"),
};

MenuItem settingMenuItems[] =
{
  MenuItem(&editKp, "Kp"),
  MenuItem(NULL, "Ki"),
  MenuItem(NULL, "Kd"),
  MenuItem(NULL, "Automatic mode"),
  MenuItem(NULL, "Output type"),
  MenuItem(NULL, "Bla bla"),
};

/**
 *
 */
void setup()
{
  Serial.begin(57600);

  screenFsm.add_transition(&idle, &mainMenu, SELECT_EVENT, NULL);

  mainMenu.setup(screenFsm, &idle, mainMenuItems, 2);
  settingMenu.setup(screenFsm, &mainMenu, settingMenuItems, 6);

  editConsignState.setup(screenFsm, &idle);

  tft.begin();

  mainScreen.enable();

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
