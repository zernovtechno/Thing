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

IRsend irsend(IRSenderPin); // ИК передатчик.
IRrecv irrecv(IRReceiverPin, 1024, 100, false); // ИК приёмник.

decode_results IRResult; // Результат ИК-декодирования

#include "Resources/Animations/FoxyOnStart.h" // Включаем анимацию лисы в шляпе
#include "Thing.h"
#include "Resources/Animations/FoxyOnCorner.h" // Включаем анимацию лисы в углу в проект

class IR_Menu_Resources_Type {
  public:
  Button* Active_Button;
  int IndexOfActive_Button;
  Menu* _IRMenu;

  struct IRData_struct {
    uint16_t* raw_array;
    uint16_t length;
    String RawString;
    String Address;
    String Command;
    String Protocol;
    void Fill(decode_results * results) {
      raw_array = resultToRawArray(results);
      length = getCorrectedRawLength(results);
      RawString = String(results->value, HEX);
      Address = String(results->address, HEX);
      Command = String(results->command, HEX);
      Protocol = typeToString(results->decode_type).c_str();
    }
  };
  void saveData(fs::FS &fs,  IRData_struct data) {
    Thing.DoLog("Writing file IRData");

    fs::File file = fs.open("/IRData", FILE_WRITE);
    if (!file) {
      Thing.DoLog("IR FS - failed to open file for writing");
      return;
    }
    if(file.write((uint8_t*)&data, sizeof(data.raw_array) + sizeof(data.length))) { // Записываем uint16_t)
      Thing.DoLog("IR FS - file written");
    }
    else {
      Thing.DoLog("IR FS - write failed");
    }
    file.println(data.RawString);  // Записываем строку
    file.println(data.Address);  // Записываем строку
    file.println(data.Command);  // Записываем строку
    file.println(data.Protocol);  // Записываем строку
    file.close();
  }
  IRData_struct loadData(fs::FS &fs) {
    IRData_struct data;
    fs::File file = fs.open("/IRData");
    if (!file || file.isDirectory()) {
      Thing.DoLog("IR FS- failed to open file for reading");
      return data;
    }

    file.read((uint8_t *)&data, sizeof(data.raw_array) + sizeof(data.length));  // Читаем uint16_t
    data.RawString = file.readStringUntil('\n');                                  // Читаем строку
    data.RawString.remove(data.RawString.length() -1);
    data.Address = file.readStringUntil('\n');
    data.Address.remove(data.Address.length() -1);
    data.Command = file.readStringUntil('\n');
    data.Command.remove(data.Command.length() -1);
    data.Protocol = file.readStringUntil('\n');
    data.Protocol.remove(data.Protocol.length() -1);
    file.close();
    return data;
  }
  String HexToStr(uint16_t hexcode) {
    String str = String(hexcode, HEX);
    str.toUpperCase();
    return str;
  }

  IRData_struct IRData[6];
  
  void ResourcesLoop () {
    if (irrecv.decode(&IRResult)) {
      if (String(IRResult.value, HEX).length() > 2) {
        for (int i = 1; i < 5; i++) {
          IRData[i] = IRData[i+1];
          _IRMenu->getButtons()[i].Label = _IRMenu->getButtons()[i+1].Label;
          _IRMenu->getButtons()[i].Draw(TFT_BLACK, TFT_WHITE);
          Active_Button = NULL;
        }
        IRData[5].Fill(&IRResult);
        _IRMenu->getButtons()[5].Label = IRData[5].RawString;
        _IRMenu->getButtons()[5].Draw(TFT_BLACK, TFT_WHITE);
        }
      irrecv.resume();
    }
  }
  void SendRawData() {
    irsend.sendRaw(IRData[IndexOfActive_Button].raw_array, IRData[IndexOfActive_Button].length, 38000);
  }
  void setIRMenu(Menu* _Menu) {
    _IRMenu = _Menu;
  }
  void setActiveButton (int Index) {
    if (IRData[Index].RawString != "") {
      if (Active_Button) Active_Button->Draw(TFT_BLACK, TFT_WHITE); // Чистим
      Active_Button = &_IRMenu->getButtons()[Index];
      IndexOfActive_Button = Index;
      Active_Button->Draw(TFT_WHITE, TFT_BLACK, true);
      tft.fillRect(110, 50, 110, 120, TFT_BLACK);
      Thing.AddedHTML = "";
      Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
      Thing.drawCentreString(IRData[Index].RawString, 165, 50, 4);
      Thing.drawCentreString(IRData[Index].Address + " " + IRData[Index].Command, 165, 70, 2);
      Thing.drawCentreString(IRData[Index].Protocol, 165, 83, 2);
      Thing.drawCentreString(Thing.DateTime, 165, 93, 2);
    }
  }
  void SaveIRData() {
    saveData(LittleFS, IRData[IndexOfActive_Button]);
  }
  void LoadIRData() {
    for (int i = 1; i < 5; i++) {
          IRData[i] = IRData[i+1];
          _IRMenu->getButtons()[i].Label = _IRMenu->getButtons()[i+1].Label;
          _IRMenu->getButtons()[i].Draw(TFT_BLACK, TFT_WHITE);
          Active_Button = NULL;
        }
        IRData[5] = loadData(LittleFS);
        _IRMenu->getButtons()[5].Label = IRData[5].RawString;
        _IRMenu->getButtons()[5].Draw(TFT_BLACK, TFT_WHITE);
  }
};
IR_Menu_Resources_Type IR_Menu_Resources;


class IR_Menu_Type : public Menu {
  public:
  String Title() override { return "IR.";}
  Button buttons[9] = {
        {10, 200, 40, 30, "<-", 2, []() { irrecv.disableIRIn();}}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 50, 100, 20, "", 1, []() { IR_Menu_Resources.setActiveButton(1); }},
        {10, 80, 100, 20, "", 1, []() { IR_Menu_Resources.setActiveButton(2); }},
        {10, 110, 100, 20, "", 1, []() { IR_Menu_Resources.setActiveButton(3); }},
        {10, 140, 100, 20, "", 1, []() { IR_Menu_Resources.setActiveButton(4); }},
        {10, 170, 100, 20, "", 1, []() { IR_Menu_Resources.setActiveButton(5); }},

        {220, 80, 100, 30, "SAVE", 2, []() { IR_Menu_Resources.SaveIRData();}},
        {220, 120, 100, 30, "EMULATE", 2, []() { IR_Menu_Resources.SendRawData();}},
        {220, 160, 100, 30, "Memory", 2, []() { IR_Menu_Resources.LoadIRData(); }}
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() override {
    IR_Menu_Resources.ResourcesLoop();
  }
  void CustomDraw() override {
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    irrecv.enableIRIn();  // Start up the IR receiver.
  }
  int getButtonsLength() override { return 9; } // 
};
IR_Menu_Type IR_Menu;



class Serial_Menu_Type2 : public Menu {
  public:
  String Title() override { return "Serial.";}
  Button buttons[1] = {
        {10, 200, 40, 30, "<-", 2, []() { if (!Thing.DebugMode) Serial.end(); }}, //Пример кнопок. Есть параметры и лямбда-функция.

    };
  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() {
    if (Serial.available() > 0) {
      tft.setTextColor(TFT_GREEN);
      tft.println("> " + Serial.readString());
      tft.setCursor(10 + tft.getCursorX(), 5 + tft.getCursorY());
      tft.setTextColor(TFT_WHITE);
    }
  }
  int getButtonsLength() override { return 1; } // 
  void CustomDraw() override {
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    delay(500);
    Serial.end();
    delay(500);
    Serial.begin(Thing.SerialFrq.toInt()); 
    Thing.drawCentreString(Thing.SerialFrq, 160, 210, 2);
    tft.setCursor(10,50);
  }
};
Serial_Menu_Type2 Serial_Menu2;

void UpdateSerialFRQ() {
  tft.fillRect(0, 90, 150, 90, TFT_BLACK); 
  Thing.AddedHTML = "";
  Thing.drawCentreString(Thing.SerialFrq, 80, 100, 4);
  Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
}
void AddNumberToSerialFRQ(int Number) {
  if (Thing.SerialFrq.length() < 8) Thing.SerialFrq += Number; 
  UpdateSerialFRQ();
}
void RemoveLastNumberFromSerialFRQ() {
  if (Thing.SerialFrq.length() > 0) Thing.SerialFrq.remove(Thing.SerialFrq.length() -1); 
  UpdateSerialFRQ();

}

class Serial_Menu_Type : public Menu {
  public:
  String Title() override { return "Serial.";}
  Button buttons[13] = {
        {10, 200, 40, 30, "<-", 2, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.

        {160, 140, 30, 30, "1", 2, []() {AddNumberToSerialFRQ(1);}},
        {200, 140, 30, 30, "2", 2, []() {AddNumberToSerialFRQ(2);}}, 
        {240, 140, 30, 30, "3", 2, []() {AddNumberToSerialFRQ(3);}}, 
        {160, 100, 30, 30, "4", 2, []() {AddNumberToSerialFRQ(4);}},
        {200, 100, 30, 30, "5", 2, []() {AddNumberToSerialFRQ(5);}},  
        {240, 100, 30, 30, "6", 2, []() {AddNumberToSerialFRQ(6);}}, 
        {160, 60, 30, 30, "7", 2, []() {AddNumberToSerialFRQ(7);}},  
        {200, 60, 30, 30, "8", 2, []() {AddNumberToSerialFRQ(8);}},  
        {240, 60, 30, 30, "9", 2, []() {AddNumberToSerialFRQ(9);}},

        {280, 60, 30, 30, "<=", 2, []() {RemoveLastNumberFromSerialFRQ();}}, 
        {280, 100, 30, 70, "0", 2, []() {AddNumberToSerialFRQ(0);}},

        {10, 60, 140, 30, "Run Serial port", 2, []() { Serial_Menu2.Draw(); Actual_Menu = &Serial_Menu2;}}

    };
  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  int getButtonsLength() override { return 13; } // 
  void CustomDraw() override {
    UpdateSerialFRQ();
  }
};
Serial_Menu_Type Serial_Menu;

class WIFI_Menu_Type : public Menu {
  public:
  String Title() override { return "Wi-Fi.";}
  Button buttons[1] = {
        {10, 200, 40, 30, "<-", 2, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() override {
  }
  void CustomDraw() override {
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    tft.setCursor(5,50);
    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        tft.print(n);
        tft.println(" networks found");
        tft.println("");
        tft.println(" N| SSID                           |RSSI|CH|Encr");
        tft.println("");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            tft.printf("%2d",i + 1);
            tft.print("|");
            tft.printf("%-32.32s", WiFi.SSID(i).c_str());
            tft.print("|");
            tft.printf("%4d", WiFi.RSSI(i));
            tft.print("|");
            tft.printf("%2d", WiFi.channel(i));
            tft.print("|");
            switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                tft.print("open");
                break;
            case WIFI_AUTH_WEP:
                tft.print("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                tft.print("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                tft.print("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                tft.print("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                tft.print("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                tft.print("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                tft.print("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                tft.print("WAPI");
                break;
            default:
                tft.print("unknown");
            }
            tft.println();
        }
    }
    tft.println("");
    WiFi.scanDelete();
  }
  int getButtonsLength() override { return 1; } // 
};
WIFI_Menu_Type WIFI_Menu;

// Пример наследуемого класса: класс главного меню. Имеет кучу кнопок, и ничего более.
class Main_Menu_Type : public Menu {
  public:
  Button buttons[9] = {
        {20, 60, 80, 40, "433Mhz", 2, []() { Thing.FoxAnimation("Be careful!", "Actions are limited by law!", TFT_RED); }}, //Пример кнопок. Есть параметры и лямбда-функция.
      	{120, 60, 80, 40, "IR", 2, []() { IR_Menu.Draw(); Actual_Menu = &IR_Menu; }},
        {220, 60, 80, 40, "NFC", 2, []() { Serial.println("Button1;3"); }},

        {20, 120, 80, 40, "WiFi", 2, []() { WIFI_Menu.Draw(); Actual_Menu = &WIFI_Menu;}},
      	{120, 120, 80, 40, "GPIO", 2, []() { Serial.println("Button2;2"); }},
        {220, 120, 80, 40, "Serial", 2, []() { Serial_Menu.Draw(); Actual_Menu = &Serial_Menu;}},

        {20, 180, 80, 40, "WiFi", 2, []() { Serial.println("Button2;1"); }},
      	{120, 180, 80, 40, "GPIO", 2, []() { Serial.println("Button2;2"); }},
        {220, 180, 80, 40, "Serial", 2, []() { Serial_Menu.Draw(); Actual_Menu = &Serial_Menu;}},
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
  Thing.DoLog("IP: " + String(WiFi.localIP()));

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