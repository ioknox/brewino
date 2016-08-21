#include <math.h>

#include <Arduino.h>

#include <SPI.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
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

/*
#define LED_PIN 4
void setup() {
  pinMode(LED_PIN, OUTPUT);


  //Save Power by writing all Digital IO LOW - note that pins just need to be tied one way or another, do not damage devices!
  for (int i = 0; i < 20; i++) {
    if(i != 2)//just because the button is hooked up to digital pin 2
    pinMode(i, OUTPUT);
  }

  attachInterrupt(0, digitalInterrupt, FALLING); //interrupt for waking up


  //SETUP WATCHDOG TIMER
WDTCSR = (24);//change enable and WDE - also resets
WDTCSR = (33);//prescalers only - get rid of the WDE and WDCE bit
WDTCSR |= (1<<6);//enable interrupt mode

  //Disable ADC - don't forget to flip back after waking up if using ADC in your application ADCSRA |= (1 << 7);
  ADCSRA &= ~(1 << 7);

  //ENABLE SLEEP - this enables the sleep mode
  SMCR |= (1 << 2); //power down mode
  SMCR |= 1;//enable sleep
}

void loop() {

  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);





  //BOD DISABLE - this must be called right before the __asm__ sleep instruction
  MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
  __asm__  __volatile__("sleep");//in line assembler to go to sleep


}

void digitalInterrupt(){
  //needed for the digital input interrupt
}

ISR(WDT_vect){
  //DON'T FORGET THIS!  Needed for the watch dog timer.  This is called after a watch dog timer timeout - this is the interrupt function called after waking up
}// watchdog interrup
*/

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

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RESET);
PID pid(
  &input,
  &output,
  &settings.consign,
  settings.proportional,
  settings.integral,
  settings.derivate,
  DIRECT
);
PID_ATune aTune(&input, &output);

CBState idle(enterIdle, NULL);
Fsm screenFsm(&idle);
/*
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
*/
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
  SerialUSB.begin(57600);
/*
  screenFsm.add_transition(&idle, &mainMenu, SELECT_EVENT, NULL);
  mainMenu.setup(screenFsm, &idle, mainMenuItems, 2);
  settingMenu.setup(screenFsm, &mainMenu, settingMenuItems, 5);
  editConsignState.setup(screenFsm, &idle);
  editScreen.setup(screenFsm, &idle);
*/
  tft.initR(INITR_BLACKTAB);  // Initialize 1.8" TFT

  SerialUSB.println("OK!");
  tft.fillScreen(ST7735_BLACK);
/*
  mainScreen.enable();

  analogReference((eAnalogReference)EXTERNAL);

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
  */
}

void loop()
{
  SerialUSB.println("Hello world");
  delay(1000);
  //sched.execute();
}
