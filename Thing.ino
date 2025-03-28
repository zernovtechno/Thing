/*
  #############################################################################
  #  _________    ___   ___      __________      ___   ___       _______      #
  # /________/\  /__/\ /__/\    /_________/\    /__/\ /__/\     /______/\     # 
  # \__    __\/  \  \ \\  \ \   \___    __\/    \  \_\\  \ \    \    __\/__   # 
  #    \  \ \     \  \/_\  \ \      \  \ \       \   `-\  \ \    \ \ /____/\  # 
  #     \  \ \     \   ___  \ \     _\  \ \__     \   _    \ \    \ \\_  _\/  # 
  #      \  \ \     \  \ \\  \ \   /__\  \__/\     \  \`-\  \ \    \ \_\ \ \  # 
  #       \__\/      \__\/ \__\/   \________\/      \__\/ \__\/     \_____\/  # 
  #                                                                           #
  #                   https://github.com/ZernovTechno/Thing                   #
  #                                                                           #
  #############################################################################
                                                    ___     ___    ___    _  _   
                                                   |__ \   / _ \  |__ \  | || |  
  ____   ___   _ __   _ __     ___   __   __          ) | | | | |    ) | | || |_ 
 |_  /  / _ \ | '__| | '_ \   / _ \  \ \ / /         / /  | | | |   / /  |__   _|
  / /  |  __/ | |    | | | | | (_) |  \ V /   _     / /_  | |_| |  / /_     | |  
 /___|  \___| |_|    |_| |_|  \___/    \_/   (_)   |____|  \___/  |____|    |_|  
     https://www.youtube.com/@zernovtech

  Мультифункциональная прошивка для ESP32. Включает основные дистанционные протоколы обмена данными. 
  RFID, NFC, WiFi, 433Mhz, ИК.

  Прошивка начата 30.10.2024 в 14:42:57.

*/

#include <Arduino.h> // Общая библиотека Arduino.
#include <SPI.h> // Библиотека коммуникации по протоколу SPI
#include <TFT_eSPI.h> // Библиотека для дисплея (тач-скрина)
#include <IRsend.h> // Утилиты для ИК-передатчика
#include <IRrecv.h> // Утилиты для ИК-приёмника
#include <IRremoteESP8266.h> // Набор дешифровальщиков и шифровальщиков для ИК-приёмопередатчика
#include <IRutils.h> // Утилиты для ИК-приёмопередатчика
#include "WiFi.h" // Wi-Fi библиотека (ESP32)
#include "ESPAsyncWebServer.h" // Веб сервер (Для веб-дисплея)
#include <ESPmDNS.h>
#include "FS.h" // Общий класс файловая система
#include <LittleFS.h> // Безопасная и быстрая файловая система для сохранений
#define FORMAT_LITTLEFS_IF_FAILED true // Форматировать сохранения при ошибке
#define IRReceiverPin 27 // Пин ИК-приёмника
#define IRSenderPin 16 // Пин ИК-светодиода

#include "Creds.h" // Включаем данные от WiFi

#include "Resources/Animations/FoxyOnStart.h" // Включаем анимацию лисы в шляпе
#include "Thing.h"
#include "Resources/Animations/FoxyOnCorner.h" // Включаем анимацию лисы в углу в проект

#include "Keyboard_part.h" // Включаем модуль клавиатуры
#include "IR_part.h" // Включаем модуль ИК
#include "Serial_part.h" // Включаем модуль Serial
#include "WiFi_part.h" // Включаем модуль WiFi
#include "GPIO_part.h" // Включаем модуль GPIO

// Пример наследуемого класса: класс главного меню. Имеет кучу кнопок, и ничего более.
class Main_Menu_Type : public Menu {
  public:
  Button buttons[9] = {
        {20, 60, 80, 40, "433Mhz", 2, []() { Thing.FoxAnimation("Be careful!", "Actions are limited by law!", TFT_RED); }}, //Пример кнопок. Есть параметры и лямбда-функция.
      	{120, 60, 80, 40, "IR", 2, []() { IR_Menu.Draw(); Actual_Menu = &IR_Menu; }},
        {220, 60, 80, 40, "NFC", 2, []() { Serial.println("Button1;3"); }},

        {20, 120, 80, 40, "WiFi", 2, []() { WIFI_Menu.Draw(); Actual_Menu = &WIFI_Menu;}},
      	{120, 120, 80, 40, "GPIO", 2, []() { GPIO_Menu.Draw(); Actual_Menu = &GPIO_Menu;}},
        {220, 120, 80, 40, "Serial", 2, []() { Serial_Menu.Draw(); Actual_Menu = &Serial_Menu;}},

        {20, 180, 80, 40, "BT", 2, []() { Serial.println("Button2;1"); }},
      	{120, 180, 80, 40, "I2c", 2, []() { Serial.println("Button2;2"); }},
        {220, 180, 80, 40, "Settings", 2, []() { Keyboard_Menu.Draw(); Actual_Menu = &Keyboard_Menu;}},
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  int getButtonsLength() override { return 9; } // 
};

Main_Menu_Type Main_Menu;

void CreateHTMLFromActual_Menu() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      for(int i=0;i<request->params();i++){
        const AsyncWebParameter* p = request->getParam(i);
        if (p->name() == "button") {
          Thing.DoLog("Just got a param. Button id is " + p->value());
          Thing.ClickOnButtonByNumber(p->value().toInt()-1);
        }
      }
      request->send(200, "text/html", Actual_Menu->HTML());
  });
      server.on("/back", HTTP_GET, [](AsyncWebServerRequest *request){
        Thing.GoBack();
        request->redirect("/");
      });
  Thing.DoLog("Paint menu "+Actual_Menu->Title()+" on a WEB");
}

void setup() {
  pinMode(IRReceiverPin, INPUT_PULLUP);
  pinMode(IRSenderPin, OUTPUT);

  irsend.begin();       // Start up the IR sender.

  Serial.setTimeout(50);
  if (Thing.DebugMode) Serial.begin(Thing.SerialFrq.toInt());
  Thing.DoLog("Serial began on frequency " + Thing.SerialFrq);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Thing.DoLog("Connecting to WiFi..");
  }

  if (Thing.DebugMode) Serial.println(WiFi.localIP());

  if (MDNS.begin("Thing")) {
    Thing.DoLog("MDNS successfully started! Get access on http:\\Thing.local");
  }

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Thing.DoLog("LittleFS Mount Failed");
    return;
  } else {
    Thing.DoLog("LittleFS Mounted Successfully");
  }

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  Thing.FoxAnimation("Welcome!");
  
  uint16_t calData[5] = { 437, 3472, 286, 3526, 3 };
  tft.setTouch(calData);

  Actual_Menu = &Main_Menu;
  Main_Menu.Draw();
  IR_Menu_Resources.setIRMenu(&IR_Menu);

  CreateHTMLFromActual_Menu();
  server.begin();
}


void loop() {
  uint16_t x = 0, y = 0; // Координаты тача
  if (tft.getTouch(&x, &y) && !Thing.CoolDown) { // Если коснулись..
    Actual_Menu->Touch(x, y); // Обработать нажатие
  }
  if (Thing.GoBackValue) { Main_Menu.Draw(); Actual_Menu = &Main_Menu; Thing.GoBackValue = false;}
  if (Thing.Clicked) {Actual_Menu->getButtons()[Thing.ClickedNumber].Action(); Thing.Clicked = false; }
  FoxyOnCornerLoop();
  Actual_Menu->MenuLoop();
}

// "Пусть кто-то говорит, что ты слаб и бездарен
//  Не слушай и играй на любимой гитаре"
//  - Екатерина Яшникова, "Это возможно".