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
#include <brewino/EditScreen.h>
#include <brewino/EditScreenMenuItem.h>

#define TFT_CS    19
#define TFT_DC    9
#define TFT_RESET 8

#define TC_CS     10

#define OUT_PIN   3

void enterIdle();
void shortCallback();
void longCallback();

Servo myservo;
int val;

Scheduler sched;
Task shortTask(5, TASK_FOREVER, &shortCallback);
Task longTask(100, TASK_FOREVER, &longCallback);
byte counter = 0;
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

EditScreen editConsignState;
EditScreen editScreen;

Menu mainMenu;
Menu settingMenu;

const char consignName[] PROGMEM = { "Edit consign" };
const char propotionalName[] PROGMEM = { "Kp" };
const char integralName[] PROGMEM = { "Ki" };
const char differencialName[] PROGMEM = { "Kd" };
const char automaticName[] PROGMEM = { "Automatic mode" };
const char outputName[] PROGMEM = { "Output type" };
const char settingsName[] PROGMEM = { "Edit settings" };

EditScreenMenuItem<double> consignItem(&editConsignState, consignName, settings.consign);
EditScreenMenuItem<double> propotionalItem(&editScreen, propotionalName, settings.proportional);
EditScreenMenuItem<double> integralItem(&editScreen, integralName, settings.integral);
EditScreenMenuItem<double> differencialItem(&editScreen, differencialName, settings.derivate);
EditScreenMenuItem<bool> autoItem(&editScreen, automaticName, settings.automatic);
EditScreenMenuItem<double> outputItem(&editScreen, outputName, settings.consign);

MenuItem settingsItem(&settingMenu, settingsName);

MenuItem* mainMenuItems[] =
{
  &consignItem,
  &settingsItem
};

MenuItem* settingMenuItems[] =
{
    &propotionalItem,
    &integralItem,
    &differencialItem,
    &autoItem,
    &outputItem
};

void enterIdle()
{
  mainScreen.enable();
}

void shortCallback()
{
  keyPad.loop();

  ButtonsEnum longEvt = keyPad.event(LongKeyDown);
  ButtonsEnum shortEvt = keyPad.event(ShortKeyUp);

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
  if ((counter % 50) == 0)
  {
    float voltage = (analogRead(A3) * 3.3f) / 1024.0f;
    input = (voltage - 0.5f) * 100.0f;
    pid.Compute();

    //myservo.write(map(output, 0, 1, 0, 100));
    digitalWrite(3, output != 0 ? HIGH : LOW);

    mainScreen.setTemp(input);
    mainScreen.setOutput(output);
  }

  mainScreen.setConsign(settings.consign);

  Screen::current()->draw(tft);

  counter++;
}

/**
 *
 */
void setup()
{
  Serial.begin(57600);

  screenFsm.add_transition(&idle, &mainMenu, SELECT_EVENT, NULL);
  mainMenu.setup(screenFsm, &idle, mainMenuItems, 2);
  settingMenu.setup(screenFsm, &mainMenu, settingMenuItems, 5);
  editConsignState.setup(screenFsm, &idle);
  editScreen.setup(screenFsm, &idle);

  tft.begin();

  mainScreen.enable();

  analogReference(EXTERNAL);

  pid.SetOutputLimits(0.0, 1.0);
  pid.SetMode(AUTOMATIC);

  input = 0.0;
  output = 0.0;

  //myservo.attach(3);
  pinMode(3, OUTPUT);

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
