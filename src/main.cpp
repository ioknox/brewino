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



void shortCallback();
void longCallback();

Scheduler sched;
Task shortTask(5, TASK_FOREVER, &shortCallback);
Task longTask(500, TASK_FOREVER, &longCallback);

double input = 0.0;
double output = 0.0;
Settings settings;
KeyPad keyPad;
MainScreen mainScreen;
Screen *screen = &mainScreen;

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

void enterIdle()
{
  screen = &mainScreen;
}

State idle(enterIdle, NULL);
Fsm screenFsm(&idle);

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

  screen->draw(tft);
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

  static void changeModifiedConsign(double &consign, MainScreen &screen)
  {
    consign += (screen.editDigit() - _editInit) * pow(10.0, _editState - 1);
  }

  static void nextDigit()
  {
    _editState = (_editState + 1) % 4;
  }

  static void editConsign(double &consign, MainScreen &screen)
  {
    _editInit = (short)(consign / pow(10.0, _editState - 1)) % 10;
    screen.editConsign(_editInit, _editState - 1);
  }
};

double ConsignManager::_originalConsign = 0.0;
short ConsignManager::_editState = 0;
short ConsignManager::_editInit = 0;

struct MenuItem : State
{
  State *state;
  String text;

  MenuItem(State *_state, const char *_text)
    : State(NULL, NULL)
  {
    state = _state;
    text = String(_text);
  }
};

template <typename TClass, void (TClass::*mf)(), TClass *ptr>
struct Adaptor
{
  static void bind()
  {
    (ptr->*mf)();
  }
};

struct Menu : public Screen
{
  Menu(MenuItem *items, int count)
  {
    _items = items;
    _count = count;
  }

  template <typename TClass, TClass *ptr>
  static void initialize(Fsm &stateMachine, State *parent)
  {
    for (unsigned int i = 0; i < ptr->_count; i++)
    {
      unsigned int p = (i - 1) % ptr->_count;
      unsigned int n = (i + 1) % ptr->_count;

      MenuItem &current(ptr->_items[i]);
      MenuItem &previous(ptr->_items[p]);
      MenuItem &next(ptr->_items[n]);

      if (i == 0)
      {
        stateMachine.add_transition(parent, &current, SELECT_EVENT, &Adaptor<Menu, &Menu::begin, ptr>::bind);
      }

      stateMachine.add_transition(&current, &previous, UP_EVENT, &Adaptor<Menu, &Menu::up, ptr>::bind);
      stateMachine.add_transition(&current, current.state, SELECT_EVENT, &Adaptor<Menu, &Menu::end, ptr>::bind);
      stateMachine.add_transition(&current, parent, BACK_EVENT, &Adaptor<Menu, &Menu::end, ptr>::bind);
      stateMachine.add_transition(&current, &next, DOWN_EVENT, &Adaptor<Menu, &Menu::down, ptr>::bind);
    }
  }


  virtual void draw(TFT &hw)
  {
    if (_requireRefresh)
    {
      _requireRefresh = false;

      Color white(255, 255, 255);
      Color black(0, 0, 0);
      Point position(0, 0);

      Label lbl(hw.width() / Size_20x32, 0, position, white);
      for (int i = 0; i < _count && position.y < hw.height(); i++)
      {
        if (i == _current)
        {
          lbl.setForeColor(black);
          lbl.setBackColor(white);
        }
        else
        {
          lbl.setForeColor(white);
          lbl.setBackColor(black);
        }
        lbl.setValue(_items[i].text);
        lbl.draw(hw);
        position.y += Size_20x32 * 2;
        lbl.setPosition(position);
      }
    }
  }

  void begin()
  {
    _current = 0;
    _requireRefresh = true;
    screen = this;
  }

  void end()
  {
    _current = 0;
  }

  void up()
  {
    _current = (_current + 1) % _count;
    _requireRefresh = true;
  }

  void down()
  {
    _current = (_current - 1) % _count;
    _requireRefresh = true;
  }

private:
  bool _requireRefresh;
  MenuItem* _items;
  unsigned int _count;
  unsigned int _current;
};

State editConsign(enterIdle, NULL);


MenuItem settingMenuItems[] =
{
  MenuItem(&idle, "Kp"),
  MenuItem(&idle, "Ki"),
  MenuItem(&idle, "Kd"),
  MenuItem(&idle, "Automatic mode"),
  MenuItem(&idle, "Output type"),
  MenuItem(&idle, "Bla bla"),
};
Menu settingMenu(settingMenuItems, 6);

MenuItem mainMenuItems[] =
{
  MenuItem(&editConsign, "Edit consign"),
  MenuItem(&editSettings, "Edit settings"),
};
Menu mainMenu(mainMenuItems, 2);


void setup() {

  Menu::initialize<Menu, &mainMenu>(screenFsm, &idle);
  Menu::initialize<Menu, &settingMenu>(screenFsm, &editSettings);

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
