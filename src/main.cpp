#include <Arduino.h>

#include <SPI.h>
#include <TFT.h>
#include <PID_v1.h>
#include <Servo.h>
#include <TaskScheduler.h>
#include <Fsm.h>

#include <math.h>

#define SELECT_EVENT 1
#define CANCEL_EVENT 2
#define UP_EVENT 3
#define DOWN_EVENT 4
#define BACK_EVENT 5

#define TFT_CS    19
#define TFT_DC    9
#define TFT_RESET 8

#define TC_CS     10

#include <util/Adaptor.h>

#include <brewino/Menu.h>
#include <brewino/Screen.h>
#include <brewino/KeyPad.h>
#include <brewino/Settings.h>

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

State idle(enterIdle, NULL);
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

class ConsignManager
{
public:
  static void enter()
  {
    Serial.println(" *** ENTER EDIT CONSIGN *** ");
    enterIdle();
    _originalConsign = settings.consign;
    _editState = 0;
    editConsign(settings.consign, mainScreen);
  }

  static void leave()
  {
      Serial.println(" *** LEAVE EDIT CONSIGN *** ");
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

  static void changeModifiedConsign(double &consign, MainScreen &screen)
  {
    double value = (screen.editDigit() - _editInit) * 10.0;
    value = round(value * pow(10.0, _editState - 1));
    consign += (value / 10.0);
  }

  static void nextDigit()
  {
    _editState = (_editState + 1) % 4;
  }

  static void editConsign(double &consign, MainScreen &screen)
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
    EditScreen()
      : State(NULL, NULL)
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



    template<EditScreen *self, void (EditScreen::*mf)()>
    static void initialize(Fsm &fsm, int event)
    {
      fsm.add_transition(self, self, event, Adaptor<EditScreen, mf, self>::bind);
    }

    template<EditScreen *self>
    static void initialize(Fsm &fsm)
    {
      initialize<self, &EditScreen::up>(fsm, UP_EVENT);
      initialize<self, &EditScreen::down>(fsm, DOWN_EVENT);
    }


};

double ConsignManager::_originalConsign = 0.0;
short ConsignManager::_editState = 0;
short ConsignManager::_editInit = 0;

State onEditConsign(ConsignManager::enter, ConsignManager::leave);
State onModifiyConsign(NULL, NULL);
State editSettings(NULL, NULL);

EditScreen editKp;

MenuItem settingMenuItems[] =
{
  MenuItem(&editKp, "Kp"),
  MenuItem(NULL, "Ki"),
  MenuItem(NULL, "Kd"),
  MenuItem(NULL, "Automatic mode"),
  MenuItem(NULL, "Output type"),
  MenuItem(NULL, "Bla bla"),
};
Menu settingMenu(settingMenuItems, 6);

MenuItem mainMenuItems[] =
{
  MenuItem(&onEditConsign, "Edit consign"),
  MenuItem(NULL, "Edit settings"),
};
Menu mainMenu(mainMenuItems, 2);

/**
 *
 */
void setup()
{
  Serial.begin(57600);

  Menu::initialize<Menu, &mainMenu>(screenFsm, &idle);
  Menu::initialize<Menu, &settingMenu>(screenFsm, &mainMenuItems[1]);

  EditScreen::initialize<&editKp>(screenFsm);

  screenFsm.add_transition(&onEditConsign, &idle, SELECT_EVENT, &ConsignManager::commit);
  screenFsm.add_transition(&onEditConsign, &idle, CANCEL_EVENT, &ConsignManager::rollback);
  screenFsm.add_transition(&onEditConsign, &onModifiyConsign, UP_EVENT, &ConsignManager::up);
  screenFsm.add_transition(&onEditConsign, &onModifiyConsign, DOWN_EVENT, &ConsignManager::down);
  screenFsm.add_transition(&onEditConsign, &onModifiyConsign, BACK_EVENT, &ConsignManager::next);

  screenFsm.add_transition(&onModifiyConsign, &idle, SELECT_EVENT, &ConsignManager::commit);
  screenFsm.add_transition(&onModifiyConsign, &idle, CANCEL_EVENT, &ConsignManager::rollback);
  screenFsm.add_transition(&onModifiyConsign, &onModifiyConsign, UP_EVENT, &ConsignManager::up);
  screenFsm.add_transition(&onModifiyConsign, &onModifiyConsign, DOWN_EVENT, &ConsignManager::down);
  screenFsm.add_transition(&onModifiyConsign, &onModifiyConsign, BACK_EVENT, &ConsignManager::next);

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
