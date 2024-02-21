/*  MaxiClock_by_platelayer.ino, by Svein-Martin Holt, www.platelayer.com february 2024

 * 3D printed 7 segment, maxi size 4-digit clock with an ESP8266, using wifi to sync the time.
 * To configure wifi, a phone can be used to access the ESP8266 to configure and connect to the network. The networkname and password is set using the phone.
 * Every hour, the bell sound, using a DFPlayer mini, with predefined audiofiles with bell ringing from one time, up to 18 times. 
 * All audio files are stored on a SD-card, in a mp3 directory. All mp3 files have 4 digit numbers, like 0001.mp3.
 
 More info can be found on my homepage: http://www.platelayer.com/maxi-7-segment-clock.aspx

 The software can be found here: https://github.com/smholt/MaxiClock

 The 3D-model of the digits, can be found on my cults3d-page: https://cults3d.com/en/users/smholt

 ------------------------------------------------------------------------------------------------------
 
 The original idea for part of this build, and the inspiration to build my own clock, is found here: https://www.youtube.com/watch?v=8E0SeycTzHw

 The ESP8266 I use has an integrated 0.96" 128x64 OLED Module, but you can also use a standard OLED Module, using the Adafruit SSD1306 library.
 I had 2 different types of ESP8266, where the Clock and data pin for the display was swapped causing some confusion until it was figured out, see comments below.

 To save the time, and use it if the network is not found and there are not possible to get the time, I use a DS3231 clock module. Temperature can also be read via ths module.

==========
There are one buttons, where a short push, change between clock mode and LED-strip demo mode.
A 5 second or longer push, let you change the volume for the bell, 0(No bell) to 30 in interval of 10.
Wait 5 seconds after last change, the new value is stored in EEPROM. On startup, the saved Volumevalue is restored from EEPROM.
A 10 second or longer push, reset the wifisetup and you must set the network and password, using a phone.
A webbpage can be opened, selecting the network you will see on the list, "AutoConnectClock".
The password for the connections is: "password".

The 4-digit seven segment sign has a total of 282 LED's in the strip. Each segment has 10 LED's, 
with the 2 last LED's used for the one "colon" and second blinker.
I have used the FastLED library for the LED-strip.

Sound is created using a DFPlayer mini, using the DFPlayerMini_Fast library: //https://github.com/PowerBroker2/DFPlayerMini_Fast
I had some problems with the standard DFPlayer library, but this Fast_Mini version, works great.


LDR Brightness control:
A LDR is connected to A0 to measure the light in the room, and set the brightness to 
a HIGH value when there are much light, and a lower value when the room is darker.
A 10 Kohm resistor is connected from A0 to GND.
The LDR is connected from A0 to VCC, + 3.3 or 5 volt.
On A0, the analogRead command read the value, between 0 and 1023.
The map command convert the value between 0-255.
If no LDR is connected, connect A0 to VCC, +3.3V/+5V to set maximum brightness

         |-----> VCC, 3.3 or 5 Volt
         |
        LDR
         |
         O-----> A0, pin 2
         |
    Resistor 10 Kohm
         |
         |-----> GND


==========

*/
#define PROG_NAME "MaxiClock"
#define BY_NAME "by platelayer"
#define PROG_VERSION "0.9, 2024.02.21"  // Set the program version number and date

//some pins on MCU
// pin 5 (GPIO5(20)/D1)ok
// pin 13 (GPIO13(7)/D7)ok
// pin 15 (GPIO15(16)/D8)ok
// pin 9 (GPIO09(11)/SD2)no
// pin 7 (GPIO07(10)/SDD)no
// pin 6 (GPIO06(14)/CLK)no

//const char * compiledOn = "compiled: " __DATE__ "\t" __TIME__;
const char* compiledOn = __DATE__ ", " __TIME__;  // Get the compiled date and time

/**
 * WiFiManager
 * Implements TRIGGEN_PIN button press, hold for 5 seconds for reset settings.
 */
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

//#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Adafruit_TFTLCD.h> // Hardware-specific library

#include <NTPClient.h>
// change next line to use with another board/shield
//#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
//#include <WiFiUdp.h>
#include <UnixTime.h>
#include <TimeLib.h>
#include <RTClib.h>

RTC_DS3231 RTC;

//https://www.instructables.com/RTC-Time-Synchronization-With-NTP-ESP8266/

UnixTime stamp(0);

unsigned long myTimeMillis;  //millis when custom time is set

// the ideaspark ESP8266 0.96" OLED Module VR:2.1 pin numbers
int pin_sca = 12;
int pin_sdl = 14;

WiFiUDP ntpUDP;

// Specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionally you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Setup for ESP8266 with 0.96" OLED Module

// Pins ESP8266: https://m.media-amazon.com/images/I/71n5cYRJInL._AC_SL1500_.jpg

// Selected board in boardmanager Arduino IDE:
// NodeMCU 1.0 (ESP-12E Module)

// At least two different types of pcb and configuration for the esp8266, available from this 2 shops:
// ideaspark Offical Store, pcb labeled: ideaspark ESP8266 0.96" OLED Module VR:2.1
// https://www.aliexpress.com/item/1005005242283189.html
// D6(GPIO12): SCA
// D5(GPIO14): SDL

// diymore module store, pcb labeled: HW-364A ESP8266 0.96" OLED Module V2.1.0
// https://www.aliexpress.com/item/1005006011203112.html
// D6(GPIO14): SDA
// D5(GPIO12): SCL

//temperature in celcius or fahrenheit?
//comment/uncomment
bool temp_celcius = true;  // temperature in celcius
//bool temp_celcius = false;  // temperature in fahrenheit

//different types of card, the sca and sdl pins are swapped.
// select board configuration, uncomment one of the lines:
bool card_type_ideaspark = true;
//bool card_type_ideaspark = false;

#define FASTLED_FORCE_SOFTWARE_SPI
//#define FASTLED_FORCE_SOFTWARE_PINS
#define FASTLED_INTERNAL
#include <FastLED.h>
FASTLED_USING_NAMESPACE

// define pinnumber used on MCU
#define TRIGGER_PIN 0  // pin 0 (GPIO0(18)/D3) WiFi Manager TRIGGEN_PIN button, press for clock mode or demo mode, hold for 5 seconds for reset settings.
#define DATA_PIN 4     // pin 4 (GPIO4(19)/D2)  Data pin for LED-strip
// pin 16 (GPIO16(4)/D0)
// pin 5 (GPIO5(20)/D1)
// pin 2 (GPIO2(17)/D4) ikke ok
// pin 13 (GPIO13(7)/D7)
// pin 15 (GPIO15(16)/D8)ikke ok

#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 282  // Number of LEDs in strip
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 120

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = false;  // change to true to use non blocking

WiFiManager wm;                     // global wm instance
WiFiManagerParameter custom_field;  // global param ( for non blocking w params )

//wm.setDebugOutput(false);

#include "Simpletimer.h"  // SimpelTimer by Natan Lisowski
Simpletimer timer1{};

uint32_t clockMinuteColour = 0x800000;  // pure red
uint32_t clockHourColour = 0x008000;    // pure green
uint32_t clockTempColour = 0x000080;    // pure blue
uint32_t clockDateColour = 0x808000;    // pure yellow
uint32_t clockBlackColour = 0x000000;   // pure black

int clockFaceBrightness = 0;

// clock_mode or demo_mode, default clock_mode
bool clock_mode = true;

int minute_t;  // variables holds the time
int hour_t;
int second_t;

int date_t;  //  variables holds the date
int month_t;
int year_t;

unsigned long unix_epoch;

// Temperature can be read as a rounded integer value, or a floating point
// float is more precise but takes a bunch of memory and flash space
byte MyIntegerTemperature;
float MyFloatTemperature;

#include "Arduino.h"

// pin 16 (GPIO16(4)/D0)
// pin 5 (GPIO5(20)/D1)
// pin 13 (GPIO13(7)/D7)
#define PLAYER_BUSY_PIN 16  // MP3 player busy, true play
#define PLAYER_RX 5         //TX on MP3 player
#define PLAYER_TX 13        //RX on MP3 player

int volumVal = 10;  // variable to store the volume value for the mp3 player(0-30)
bool playSound = true;

int audioFileOffset = 0;  // variable to select pair of audiofiles. 0=filenumbers for 0001.mp3, 1000:filenumbers from 1001.mp3



//--------------------
//https://github.com/PowerBroker2/DFPlayerMini_Fast
#include <DFPlayerMini_Fast.h>

#if !defined(UBRR1H)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(PLAYER_RX, PLAYER_TX);  // RX, TX
#endif

DFPlayerMini_Fast myMP3;
//----------------------

// Define the GPIO pin number for the pushbutton
//const int buttonPin = 0;  // Assuming GPIO 0
// Variables to keep track of the button press
unsigned long buttonPressedTime = 0;
unsigned long buttonReleasedTime = 0;
bool isButtonPressed = false;
bool printMess1 = false;
bool printMess2 = false;

// Mode variable
int mode = 0;

// Variable for mode 2
int variableMode2 = 0;
unsigned long pressDuration = 0;

#include <EEPROM.h>
//adresse stored volume value
int eeprom_adresse = 0;

bool RTC_found = true;  // RTC modul present?

String demoEffect = "";

//----------------------------------------------------------------

void checkButton() {

  int mode2_time = 4000;   //time enter volume change mode
  int mode2_wait = 5000;   //time with no press, leaving volume mode
  int mode3_time = 10000;  //time enter restart/erase mode

  // Read the state of the pushbutton
  int buttonState = digitalRead(TRIGGER_PIN);

  // Check if the button is pressed
  if (buttonState == LOW && !isButtonPressed) {
    // Button is pressed
    isButtonPressed = true;
    buttonPressedTime = millis();
  }

  if ((millis() - buttonPressedTime >= mode2_time or buttonPressedTime < mode3_time) && isButtonPressed && !printMess1) {
    Serial.print(mode2_time / 1000);
    Serial.println(" sec. Enter Toggle Change.");
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Toggle");
    display.println("Volume");
    display.println("Change");
    display.println(volumVal);
    display.display();
    delay(1000);
    printMess1 = true;
    mode = 2;
  }
  if (millis() - buttonPressedTime >= mode3_time && isButtonPressed && !printMess2) {
    Serial.print(mode3_time / 1000);
    Serial.println(" sec. Reset Setup.");
    printMess2 = true;
  }

  // Check if the button was released
  if (buttonState == HIGH && isButtonPressed) {
    // Button was released
    isButtonPressed = false;
    printMess1 = false;
    printMess2 = false;
    buttonReleasedTime = millis();
    pressDuration = buttonReleasedTime - buttonPressedTime;

    // Short press
    if (pressDuration < mode2_time) {  // Less than 5 seconds
      if (mode == 0 || mode == 1) {
        mode = 1 - mode;  // Toggle between mode 0 and 1
        if (mode == 0) {
          Serial.println("Clock Mode");
        } else {
          Serial.println("Demo Mode");
        }

      } else if (mode == 2) {
        // Toggle between 6 different values of a variable 0-30 Volume, 40-50 audioOffsett
        variableMode2 = (variableMode2 + 10) % 60;
        //  Serial.print("variableMode2: ");
        //  Serial.println(variableMode2);

        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        if (variableMode2 < 40) {
          volumVal = variableMode2;
          Serial.print("Volume: ");
          Serial.println(volumVal);

          display.print("Volume: ");
          display.println(volumVal);
          display.setTextSize(1);
          display.print(mode2_wait / 1000);
          display.println(" sec wait\nsaves volume.");
        } else {
          if (variableMode2 == 40) {
            audioFileOffset = 0;
            Serial.println("AudioFiles Selected: Bell");

            display.print("AudioFiles Selected: Bell-");
            display.println(audioFileOffset);
            display.setTextSize(1);
            display.print(mode2_wait / 1000);
            display.println(" sec wait\nsaves Bell.");
          } else {
            audioFileOffset = 1000;
            Serial.println("AudioFiles Selected: KoKo");

            display.print("AudioFiles Selected: KoKo-");
            display.println(audioFileOffset);
            display.setTextSize(1);
            display.print(mode2_wait / 1000);
            display.println(" sec wait\nsaves KoKo.");
          }
          Serial.print("audioFileOffset: ");
          Serial.println(audioFileOffset);
        }
        display.display();

        // Reset to Mode 0 after mode3_time seconds of inactivity is handled below
      }
    } else if (pressDuration >= mode2_time && pressDuration < mode3_time) {  // Between mode2_time and mode3_time seconds
      mode = 2;
      if (variableMode2 < 40) {
        Serial.print("Volume: ");
        Serial.println(volumVal);
      } else {
        Serial.print("audioFileOffset: ");
        Serial.println(audioFileOffset);
      }
    } else if (pressDuration >= mode3_time) {  // More than mode3_time seconds
      mode = 3;
      executeMode3Function();
    }
  }

  // Additional logic for mode 2 to auto-return to mode 0 after 5 seconds of no press
  if (mode == 2 && millis() - buttonReleasedTime > mode2_wait && !isButtonPressed) {
    mode = 0;

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);

    if (variableMode2 < 40) {
      EEPROM.put(eeprom_adresse, volumVal);
      EEPROM.commit();
      Serial.print("Volume stored in EEPROM: ");
      Serial.println(volumVal);

      display.print("Volume: ");
      display.println(volumVal);

    } else {
      EEPROM.put(eeprom_adresse + 4, audioFileOffset);
      EEPROM.commit();
      Serial.print("AudioFile offset stored in EEPROM: ");
      Serial.println(audioFileOffset);

      display.print("audioFileOffset: ");
      display.println(audioFileOffset);
    }

    display.println("saved.");
    display.display();

    Serial.print("No button press detected in Mode 2 for ");
    Serial.print(mode2_wait / 1000);
    Serial.println(" seconds - Returning to Mode 0(Clock Mode)");

    if (playSound && volumVal > 0) {
      if (variableMode2 < 40) {
        delay(1000);
        Serial.print("Setting volume to ");
        Serial.println(volumVal);
        myMP3.volume(volumVal);
      }

      myMP3.playFromMP3Folder(audioFileOffset + 1);  // play track 1
    }
    delay(2000);
  }
}

void executeMode3Function() {
  // Placeholder for the function's action

  Serial.println("Erasing Config, restarting");

  display.clearDisplay();
  display.setTextSize(1);  // Normal 1:1 pixel scale
  display.setCursor(0, 0);
  display.println("Erasing Config.");
  display.println("Restarting.");
  display.display();
  delay(3000);

  wm.resetSettings();
  ESP.restart();
}

void setup_DFPlayerMini() {

#if !defined(UBRR1H)
  mySerial.begin(9600);
  myMP3.begin(mySerial, false);  //debug off
#else
  Serial1.begin(9600);
  myMP3.begin(Serial1, false);
#endif

  delay(1000);

  Serial.print(F("Volume value read: "));
  Serial.println(volumVal);

  if (variableMode2 < 40) { myMP3.volume(volumVal); }

  Serial.print("audioFileOffset read: ");
  Serial.println(audioFileOffset);

  Serial.println();

  myMP3.playFromMP3Folder(audioFileOffset + 1);
}

//------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;  // wait until Arduino Serial Monitor opens

  delay(1000);
  Serial.println("\n");
  Serial.print(PROG_NAME);
  Serial.print(" ");
  Serial.println(BY_NAME);
  Serial.print(F("V."));
  Serial.print(PROG_VERSION);
  Serial.print(F(", compiled: "));
  Serial.println(compiledOn);
  Serial.println(" ");

  Serial.setDebugOutput(true);

  EEPROM.begin(20);  // Allocate 20 bytes for EEPROM
  //get stored volume value
  int volumStored = EEPROM.get(eeprom_adresse, volumStored);
  if (volumStored < 0 or volumStored > 30) {
    volumStored = 10;
    EEPROM.put(eeprom_adresse, volumStored);
    EEPROM.commit();
    Serial.print("Init volume stored in EEPROM: ");
  } else {
    volumStored = EEPROM.get(eeprom_adresse, volumStored);
    Serial.print("Stored volume read from EEPROM: ");
  }
  volumVal = volumStored;
  Serial.println(volumStored);

  // get stored sound type
  audioFileOffset = EEPROM.get(eeprom_adresse + 4, audioFileOffset);
  if (audioFileOffset != 0 and audioFileOffset != 1000) {
    audioFileOffset = 0;
    EEPROM.put(eeprom_adresse + 4, audioFileOffset);
    EEPROM.commit();
    Serial.print("Init AudioFile stored in EEPROM: ");
  } else {
    audioFileOffset = EEPROM.get(eeprom_adresse + 4, audioFileOffset);
    Serial.print("Stored AudioFile read from EEPROM: ");
  }
  Serial.println(audioFileOffset);

  // get stored temperature units
  temp_celcius = EEPROM.get(eeprom_adresse + 8, temp_celcius);
  if (temp_celcius != 0 and temp_celcius != 1) {
    temp_celcius = 1;
    EEPROM.put(eeprom_adresse + 8, temp_celcius);
    EEPROM.commit();
    Serial.print("Init Temp Units stored in EEPROM: ");
  } else {
    temp_celcius = EEPROM.get(eeprom_adresse + 8, temp_celcius);
    Serial.print("Stored Temp Units read from EEPROM: ");
  }
  Serial.println(temp_celcius);


  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.clear();  //clear LED-strip
  FastLED.show();
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  //network trigger button
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  if (card_type_ideaspark) {
    // ideaspark ESP8266 0.96" OLED Module VR:2.1
    // D6(GPIO12): SCA
    // D5(GPIO14): SDL
    pin_sca = 12;
    pin_sdl = 14;
    Serial.print(F("card_type_ideaspark selected, pin_sca: "));
  } else {
    // HW-364A ESP8266 0.96" OLED Module V2.1.0
    // D6(GPIO14): SDA
    // D5(GPIO12): SCL
    pin_sca = 14;
    pin_sdl = 12;
    Serial.print(F("card_type_ideaspark NOT selected, pin_sca: "));
  }
  Serial.print(pin_sca);
  Serial.print(", pin_sdl: ");
  Serial.println(pin_sdl);
  Serial.println(" ");

  Wire.begin(pin_sca, pin_sdl);  // Set SCA and SDL pin numbers on display

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display_welcome();  // display welcome message on OLED display

  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP

  // wm.resetSettings(); // wipe settings

  if (wm_nonblocking) wm.setConfigPortalBlocking(false);

  //wm.setSaveParamsCallback(saveParamCallback);
  //Info about WiFiManager: https://dronebotworkshop.com/wifimanager/

  // custom menu via array or vector
  //
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"};
  // wm.setMenu(menu,6);
  //std::vector<const char*> menu = { "wifi", "info", "param", "sep", "restart", "exit" };
  std::vector<const char*> menu = { "wifi", "sep", "restart", "erase", "exit" };
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  //set static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  // wm.setShowStaticFields(true); // force show static ip fields
  // wm.setShowDnsFields(true);    // force show dns field always

  wm.setConnectTimeout(20);       // how long to try to connect for before continuing
  wm.setConfigPortalTimeout(40);  // auto close configportal after n seconds
                                  // wm.setCaptivePortalEnable(false); // disable captive portal redirection
                                  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  wm.setShowInfoErase(false);  // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons

  wm.setBreakAfterConfig(true);  // always exit configportal even if wifi save fails

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnectClock", "password");  // password protected ap

  if (WiFi.status() != WL_CONNECTED) {
    display.setCursor(0, 0);
    display.clearDisplay();
    display.setTextSize(1);
    display.println("Connect on phone");
    display.println("AutoConnectClock");
    display.println("PW: password");
    display.display();

    wifiInfo();

    delay(2000);
  }

  RTC.begin();  // Initialize DS3231 RTC module

  RTC_found = true;
  if (!RTC.begin()) {
    Serial.println("Couldn't find RTC");
    // Serial.flush();

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.clearDisplay();
    display.println("Couldn't\nfind RTC");
    display.display();
    delay(3000);
    RTC_found = false;

    // while (1) delay(10);
  }

  timeClient.begin();  // Start NTP client

  String ssidstr = (String)wm.getWiFiSSID();
  if (WiFi.status() == WL_CONNECTED) {
    //if (ssidstr.length() > 0) {
    Serial.println("Adjust DateTime from net");
    timeClient.update();                     // Retrieve current epoch time from NTP server
    unix_epoch = timeClient.getEpochTime();  // Get epoch time
    if (RTC_found) {
      RTC.adjust(DateTime(unix_epoch));  //get time if network set
    }

    printf("Date & Time from unix_epoch: %4d-%02d-%02d %02d:%02d:%02d\n", year(unix_epoch), month(unix_epoch), day(unix_epoch), hour(unix_epoch), minute(unix_epoch), second(unix_epoch));

    wifiInfoDisplay();

    delay(2000);
  }

  Serial.print("Time NET: ");
  Serial.println(timeClient.getFormattedTime());

  if (RTC_found) {
    DateTime now = RTC.now();  // Get initial time from RTC
    Serial.print("Time RTC: ");
    Serial.printf("%02d", now.hour(), DEC);
    Serial.print(':');
    Serial.printf("%02d", now.minute(), DEC);
    Serial.print(":");
    Serial.printf("%02d", now.second(), DEC);
    //MyIntHour = RTC.getHour();

    Serial.print("Date RTC: ");
    Serial.printf("%02d", now.day(), DEC);
    Serial.print(':');
    Serial.printf("%02d", now.month(), DEC);
    Serial.print(":");
    Serial.println(now.year(), DEC);

    // Ask the clock for the data.
    MyIntegerTemperature = RTC.getTemperature();
    MyFloatTemperature = RTC.getTemperature();
    if (!temp_celcius) {
      MyFloatTemperature = MyFloatTemperature * 1.8 + 32;  //convert temp celcius to fahrenheit
      MyIntegerTemperature = (int)round(MyFloatTemperature);
      Serial.print("Temperature Fahrenheit:   ");
    } else {
      Serial.print("Temperature Celcius:   ");
    }
    Serial.println(MyFloatTemperature);
  }

  if (playSound && volumVal > 0) {

    setup_DFPlayerMini();

    myMP3.playFromMP3Folder(audioFileOffset + 1);
  }
}

// List of demo patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { confetti, rainbow, rainbowWithGlitter, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0;  // Index number of which pattern is current
uint8_t gHue = 0;                   // rotating "base color" used by many of the patterns

void playBell() {
  //Serial.println("Enter Play");
  //Serial.println(digitalRead(PLAYER_BUSY_PIN));
  //play every full hour
  // hour_t = 2;
  // minute_t = 0;
  // second_t = 0;
  if (hour_t >= 8 && hour_t < 24) {
    int hour_local = hour_t;
    if (hour_local > 12) { hour_local = hour_local - 12; }
    if (minute_t == 0 && second_t < 2) {

      if (digitalRead(PLAYER_BUSY_PIN)) {  // play if ready(1)
                                           // Serial.print("Play hour ");
                                           // Serial.println(hour_local);
        myMP3.playFromMP3Folder(audioFileOffset + hour_local);
      }
      /* else {
        Serial.print("Player not ready, hour: ");
        Serial.println(hour_local);
      }
      */
    }

    //play every 15 min
    if ((minute_t == 30 or minute_t == 15 or minute_t == 45) && second_t < 2 && digitalRead(PLAYER_BUSY_PIN)) {
      //if (minute_t == 30 && second_t < 2 && digitalRead(PLAYER_BUSY_PIN)) {
      myMP3.playFromMP3Folder(audioFileOffset + 1);
    }
  }
}


void wifiInfoDisplay() {
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.clearDisplay();
  // can contain gargbage on esp32 if wifi is not ready yet
  display.println("Connected.");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("SSID: " + (String)wm.getWiFiSSID());
  display.println("HOSTNAME: " + (String)WiFi.getHostname());
  display.print("Volume set: ");
  display.println(volumVal);
  display.display();
  delay(3000);
}

void display_welcome(void) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(PROG_NAME);
  display.setTextSize(1);
  display.print(BY_NAME);
  display.setCursor(0, 25);
  display.print(F("V."));
  display.println(PROG_VERSION);
  display.println(F("Compiled: "));
  display.println(compiledOn);
  display.println("SSID: " + (String)wm.getWiFiSSID());
  String ssidstr = (String)wm.getWiFiSSID();
  display.display();
  delay(3000);

  if (ssidstr.length() <= 0) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("NO Network \nconnected.\nUse Phone and\nselect network\nAutoConnectClock\nPW: password\nto set NETWORK.");
    display.display();
    delay(5000);
  }
}

void wifiInfo() {
  // can contain gargbage on esp32 if wifi is not ready yet
  Serial.println("[WIFI] SAVED: " + (String)(wm.getWiFiIsSaved() ? "YES" : "NO"));
  Serial.println("[WIFI] SSID: " + (String)wm.getWiFiSSID());
  Serial.println("[WIFI] PASS: " + (String)wm.getWiFiPass());
  Serial.println("[WIFI] HOSTNAME: " + (String)WiFi.getHostname());
}

void checkButton_old() {
  // check for button press
  if (digitalRead(TRIGGER_PIN) == LOW) {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);

    if (digitalRead(TRIGGER_PIN) == LOW) {

      Serial.println("Button Pressed");  // short press, toggle clock_mode or demo_mode

      display.clearDisplay();
      display.setTextSize(1);  // Normal 1:1 pixel scale
      display.setCursor(0, 0);
      display.println("Button Pressed");
      display.println(" ");
      display.println("Toggle");
      display.println("Demo Mode");
      display.println("or Clock Mode.");
      display.println("Hold longer than");
      display.println("5 sec RESET network.");
      display.display();
      clock_mode = !clock_mode;  //toggle clock_mode or demo_mode

      FastLED.clear();  //clear LED-strip

      // still holding button for 5000 ms, reset settings, code not ideal for production
      delay(5000);  // reset delay hold
      if (digitalRead(TRIGGER_PIN) == LOW) {
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");

        display.clearDisplay();
        display.setTextSize(1);  // Normal 1:1 pixel scale
        display.setCursor(0, 0);
        display.println("Button Held");
        display.println(" ");
        display.println("Erasing Config.");
        display.println("Restarting.");
        display.display();
        delay(3000);

        wm.resetSettings();
        ESP.restart();
      }
    }
  }
}

String getParam(String name) {
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void loop() {

  if (wm_nonblocking) wm.process();  // avoid delays() in loop when non-blocking and other long running code

  checkButton();

  if (mode != 2) {  //mode 2, change volume, dont update clock or display
    if (mode == 0) {
      clock_mode = true;
    } else {
      clock_mode = false;
    };

    if (clock_mode) {
      demoEffect = "";
      if (timer1.timer(1000)) {  //change LED-strip every second
        check_light();           //check light in room, set brighness on LED-strip
                                 //Serial.println(timeClient.getSeconds() % 10);  // pick first digit
        getTime();

        if (timeClient.getSeconds() % 16 > 6) {  // display temperature for 4 seconds
          if (RTC_found) {
            if (timeClient.getSeconds() % 16 > 11) {  // display date for 5 seconds
              displayDates();
            } else {
              displayGrads();  // display grads if RTC and temperature is avalable
            }
          }
        } else {
          displayTheTime();
        }
      }
    } else {
      // run ledstrip demo
      if (mode != 2) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println(demoEffect);
        display.setCursor(0, 20);
        display.println("LED-strip");
        display.println("DEMO Mode");
        display.display();
      }

      FastLED.setBrightness(255);

      loop_demoreel();
    }
  }

  if (playSound && volumVal > 0) {
    playBell();
  }
}

void getTime() {

  //timeClient.update();
  //Check if on network

  minute_t = timeClient.getMinutes();
  hour_t = timeClient.getHours();
  second_t = timeClient.getSeconds();

  date_t = day(unix_epoch);
  month_t = month(unix_epoch);
  year_t = year(unix_epoch);

  //Check if time is set from internet, should be the same on the RTC-module
  /*if (hour_t != hour() && minute_t != minute()) {
  Serial.println("Time set from rtc");
   unsigned long currentTime = now();
     Serial.println(currentTime);
    //Get time from RTC module.
    hour_t = hour();
    minute_t = minute();
    second_t = second();
  }
  */

  display.clearDisplay();
  display.setCursor(4, 0);
  display.setTextSize(2);
  display.printf("%02d-%02d-%4d", day(unix_epoch), month(unix_epoch), year(unix_epoch));
  display.setTextSize(4);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(3, 16);

  display.printf("%02d", hour_t);  // Start at top-left corner

  if (second_t % 2 == 0) {  //change second tick every odd second
    display.print(":");     // display :
    fill_solid(&(leds[280]), 2 /*number of leds*/, CRGB(clockMinuteColour));
  } else {
    display.print(" ");  // reset :
    fill_solid(&(leds[280]), 2 /*number of leds*/, CRGB(clockHourColour));
  }

  FastLED.show();

  display.printf("%02d", minute_t);

  display.setTextSize(2);

  if (RTC_found) {
    MyFloatTemperature = RTC.getTemperature();
    if (!temp_celcius) {
      MyFloatTemperature = MyFloatTemperature * 1.8 + 32;  //convert temp celcius to fahrenheit
    }

    display.setTextSize(1);
    display.setCursor(0, 52);  //print a degree sign (°)
    display.print("Temp: ");
    display.setTextSize(2);
    display.setCursor(32, 48);
    display.print(MyFloatTemperature, 2);
    display.print(" ");
    display.write(247);
    if (temp_celcius) {
      display.print("C");
    } else {
      display.print("F");
    }
  }

  if (mode != 2) {  // if mode=2, volume setting, dont update display
    display.display();
  }

  display.setTextSize(1);
}

void check_light() {

  int lightSensorValue;

  lightSensorValue = analogRead(A0);  //get light level

  //set the brightness based on ambiant light levels
  clockFaceBrightness = map(lightSensorValue, 100, 1023, 7, 255);

  if (clockFaceBrightness <= 7) { clockFaceBrightness = 7; }  // minimum brighness value
                                                              /*
  Serial.print("lightSensorValue = ");
  Serial.print(lightSensorValue);
  Serial.print(", clockFaceBrightness = ");
  Serial.println(clockFaceBrightness);
*/
  FastLED.setBrightness(clockFaceBrightness);                 // Set brightness value of the LEDs
  FastLED.show();
}

void displayTheTime() {

  // FastLED.clear();  //clear the clock face
  fill_solid(&(leds[0]), 280 /*number of leds*/, CRGB(clockBlackColour));

  //timeClient.update();

  /* minute_t = timeClient.getMinutes();
  hour_t = timeClient.getHours();
  second_t = timeClient.getSeconds();
*/
  int firstMinuteDigit = minute_t % 10;  //work out the value of the first digit and then display it
  displayNumber(firstMinuteDigit, 0, clockMinuteColour);

  int secondMinuteDigit = floor(minute_t / 10);  //work out the value for the second digit and then display it
  displayNumber(secondMinuteDigit, 70, clockMinuteColour);

  int firstHourDigit = hour_t;  //work out the value for the third digit and then display it
  /*if (firstHourDigit > 12) {
    firstHourDigit = firstHourDigit - 12;
  }
  */

  // Comment out the following three lines if you want midnight to be shown as 12:00 instead of 0:00
  //  if (firstHourDigit == 0){
  //    firstHourDigit = 12;
  //  }

  firstHourDigit = firstHourDigit % 10;
  displayNumber(firstHourDigit, 140, clockHourColour);

  int secondHourDigit = floor(hour_t / 10);  //work out the value for the fourth digit and then display it

  // Comment out the following three lines if you want midnight to be shwon as 12:00 instead of 0:00
  //  if (secondHourDigit == 0){
  //    secondHourDigit = 12;
  //  }

  /*if (secondHourDigit > 12) {
    secondHourDigit = secondHourDigit - 12;
  }
  */

  displayNumber(secondHourDigit, 210, clockHourColour);
  FastLED.show();
  //delay(1000);
}

void displayDates() {

  FastLED.clear();

  // Serial.print("date_t: ");
  // Serial.println(date_t);

  int firstDatesDigit = date_t;
  firstDatesDigit = firstDatesDigit % 10;
  displayNumber(firstDatesDigit, 140, clockDateColour);
  // Serial.print("firstDatesDigit: ");
  // Serial.println(firstDatesDigit);

  int secondDatesDigit = date_t;
  secondDatesDigit = floor(date_t / 10);
  displayNumber(secondDatesDigit, 210, clockDateColour);
  // Serial.print("secondDatesDigit: ");
  // Serial.println(secondDatesDigit);

  // Serial.print("month_t: ");
  // Serial.println(month_t);

  int thirdDatesDigit = month_t;
  thirdDatesDigit = floor(month_t / 10);
  displayNumber(thirdDatesDigit, 70, clockDateColour);
  // Serial.print("thirdDatesDigit: ");
  // Serial.println(thirdDatesDigit);

  int fourthDatesDigit = month_t;
  fourthDatesDigit = fourthDatesDigit % 10;
  displayNumber(fourthDatesDigit, 0, clockDateColour);
  // Serial.print("fourthDatesDigit: ");
  // Serial.println(fourthDatesDigit);

  second_t = timeClient.getSeconds();

  if (second_t % 2 == 0) {  //change second tick every odd second
    display.print(":");     // display :
    fill_solid(&(leds[280]), 2 /*number of leds*/, CRGB(clockMinuteColour));
  } else {
    fill_solid(&(leds[280]), 2 /*number of leds*/, CRGB(clockHourColour));
    display.print(" ");  // space :
  }

  FastLED.show();
}

void displayGrads() {

  FastLED.clear();

  int firstGradsDigit = MyIntegerTemperature;
  firstGradsDigit = firstGradsDigit % 10;
  displayNumber(firstGradsDigit, 140, clockTempColour);

  int secondGradsDigit = MyIntegerTemperature;
  secondGradsDigit = floor(MyIntegerTemperature / 10);
  displayNumber(secondGradsDigit, 210, clockTempColour);

  if (temp_celcius) {
    displayNumber(10, 0, clockTempColour);  //Display C
  } else {
    displayNumber(11, 0, clockTempColour);  //Display F
  }

  displayNumber(12, 70, clockTempColour);  //Display degree sign

  second_t = timeClient.getSeconds();

  if (second_t % 2 == 0) {  //change second tick every odd second
    display.print(":");     // display :
    fill_solid(&(leds[280]), 2 /*number of leds*/, CRGB(clockMinuteColour));
  } else {
    fill_solid(&(leds[280]), 2 /*number of leds*/, CRGB(clockHourColour));
    display.print(" ");  // space :
  }

  FastLED.show();
}


void displayNumber(int digitToDisplay, int offsetBy, uint32_t colourToUse) {
  switch (digitToDisplay) {
    case 0:
      digitZero(offsetBy, colourToUse);
      break;
    case 1:
      digitOne(offsetBy, colourToUse);
      break;
    case 2:
      digitTwo(offsetBy, colourToUse);
      break;
    case 3:
      digitThree(offsetBy, colourToUse);
      break;
    case 4:
      digitFour(offsetBy, colourToUse);
      break;
    case 5:
      digitFive(offsetBy, colourToUse);
      break;
    case 6:
      digitSix(offsetBy, colourToUse);
      break;
    case 7:
      digitSeven(offsetBy, colourToUse);
      break;
    case 8:
      digitEight(offsetBy, colourToUse);
      break;
    case 9:
      digitNine(offsetBy, colourToUse);
      break;
    case 10:
      digitCelcius(offsetBy, colourToUse);
      break;
    case 11:
      digitFahrenheit(offsetBy, colourToUse);
      break;
    case 12:
      digitGrads(offsetBy, colourToUse);
      break;
    default:
      break;
  }
}

// 10 leds in each 7-segment
//

void digitZero(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 30, CRGB(colour));
  fill_solid(&(leds[(40 + offset)]), 30, CRGB(colour));
}

void digitOne(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 10, CRGB(colour));
  fill_solid(&(leds[(40 + offset)]), 10, CRGB(colour));
}

void digitTwo(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 20, CRGB(colour));
  fill_solid(&(leds[(30 + offset)]), 10, CRGB(colour));
  fill_solid(&(leds[(50 + offset)]), 20, CRGB(colour));
}

void digitThree(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 20, CRGB(colour));
  fill_solid(&(leds[(30 + offset)]), 30, CRGB(colour));
}

void digitFour(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 10, CRGB(colour));
  fill_solid(&(leds[(20 + offset)]), 30, CRGB(colour));
}

void digitFive(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 50, CRGB(colour));
}

void digitSix(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 60, CRGB(colour));
}

void digitSeven(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 20, CRGB(colour));
  fill_solid(&(leds[(40 + offset)]), 10, CRGB(colour));
}

void digitEight(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 70, CRGB(colour));
}

void digitNine(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 60, CRGB(colour));
}

void digitCelcius(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 20, CRGB(colour)); 
  fill_solid(&(leds[(50 + offset)]), 20, CRGB(colour));
}

void digitFahrenheit(int offset, uint32_t colour) {
  fill_solid(&(leds[(10 + offset)]), 30, CRGB(colour)); 
  fill_solid(&(leds[(60 + offset)]), 10, CRGB(colour));
}

void digitGrads(int offset, uint32_t colour) {
  fill_solid(&(leds[(0 + offset)]), 40, CRGB(colour));
}

// This code is from the FastLED example, DemoReel100

void loop_demoreel() {
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS(20) {
    gHue++;
  }  // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS(10) {
    nextPattern();
  }  // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern() {

  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void rainbow() {
  demoEffect = "Rainbow";
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() {

  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  demoEffect = "Glitter";
  addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter) {
  if (random8() < chanceOfGlitter) {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

void confetti() {
  demoEffect = "Confetti";
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
  demoEffect = "Sinelon";
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm() {
  demoEffect = "BPM 62";
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) {  //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  demoEffect = "Juggle";
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
