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
#include "FS.h" // Общий класс файловая система
#include <LittleFS.h> // Безопасная и быстрая файловая система для сохранений
#define FORMAT_LITTLEFS_IF_FAILED true // Форматировать сохранения при ошибке
//Частота связи по UART
String SerialFrq = "115200";
//Текущие дата и время
String DateTime = "Unknown datetime";
//Режим отладки (вывод инфо в UART)
bool DebugMode = true;
bool CoolDown;

const int IRReceiverPin = 27;
const int IRSenderPin = 16;

const char* ssid = "";
const char* password =  "";
TFT_eSPI tft = TFT_eSPI(); // Объект дисплея 

AsyncWebServer server(80); // Веб-сервер (веб-дисплей)

IRsend irsend(IRSenderPin); // ИК передатчик.
IRrecv irrecv(IRReceiverPin, 1024, 100, false); // ИК приёмник.

decode_results IRResult; // Результат ИК-декодирования

#include "Resources/Animations/FoxyOnCorner.h" // Включаем анимацию лисы в углу в проект
#include "Resources/Animations/FoxyOnStart.h" // Включаем анимацию лисы в шляпе

#include "InterfaceBase.h"
Menu* Actual_Menu;

void DoLog(String text) {
  if (DebugMode) {
    String LogText = "[DEBUG] ";
    LogText += "[" + DateTime + "] ";
    LogText += text;
    Serial.println(LogText);
  }
}

// Анимация лисички в шляпе на старте.
void FoxAnimation(String Text, String UpperText = "", uint16_t UpperTextColor = TFT_BLACK) {
  DoLog("Running splash-screen with text \"" + Text + "\" and UpperText - \"" + UpperText + "\"" );
  tft.fillScreen(TFT_WHITE);
  for (int i = 0; i < 5; i++) {
    tft.pushImage(30, 50, 191, 144, FoxyOnStart[i]);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(Text, 110, 70, 4);
    tft.setTextColor(UpperTextColor, TFT_WHITE);
    tft.drawCentreString(UpperText, 160, 20, 4);
    delay(700);
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
}

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
    if (DebugMode) Serial.printf("Writing file: %s\r\n", "IRData");

    fs::File file = fs.open("/IRData", FILE_WRITE);
    if (!file) {
      if (DebugMode) DoLog("IR FS - failed to open file for writing");
      return;
    }
    if(file.write((uint8_t*)&data, sizeof(data.raw_array) + sizeof(data.length))) { // Записываем uint16_t)
      if (DebugMode) DoLog("IR FS - file written");
    }
    else {
      if (DebugMode) DoLog("IR FS - write failed");
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
      DoLog("IR FS- failed to open file for reading");
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
      Serial.println(String(IRResult.value, HEX));
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
   // if (DebugMode) Serial.println(IRData[IndexOfActive_Button].raw_array);
    if (DebugMode) Serial.println(IRData[IndexOfActive_Button].length);
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
      tft.drawCentreString(IRData[Index].RawString, 165, 50, 4);
      tft.drawCentreString(IRData[Index].Address + " " + IRData[Index].Command, 165, 70, 2);
      tft.drawCentreString(IRData[Index].Protocol, 165, 83, 2);
      tft.drawCentreString(DateTime, 165, 93, 2);
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
    tft.drawLine(10, 200, 310, 200, TFT_WHITE);
    irrecv.enableIRIn();  // Start up the IR receiver.
  }
  int getButtonsLength() override { return 9; } // 
};
IR_Menu_Type IR_Menu;



class Serial_Menu_Type2 : public Menu {
  public:
  String Title() override { return "Serial.";}
  Button buttons[1] = {
        {10, 200, 40, 30, "<-", 2, []() { if (!DebugMode) Serial.end(); }}, //Пример кнопок. Есть параметры и лямбда-функция.

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
    tft.drawLine(10, 200, 310, 200, TFT_WHITE);
    delay(500);
    Serial.end();
    delay(500);
    Serial.begin(SerialFrq.toInt()); 
    tft.drawCentreString(SerialFrq, 160, 210, 2);
    tft.setCursor(10,50);
  }
};
Serial_Menu_Type2 Serial_Menu2;



class Serial_Menu_Type : public Menu {
  public:
  String Title() override { return "Serial.";}
  Button buttons[13] = {
        {10, 200, 40, 30, "<-", 2, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.

        {160, 140, 30, 30, "1", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "1"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}},
        {200, 140, 30, 30, "2", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "2"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}}, 
        {240, 140, 30, 30, "3", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "3"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}}, 
        {160, 100, 30, 30, "4", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "4"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}},
        {200, 100, 30, 30, "5", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "5"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}},  
        {240, 100, 30, 30, "6", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "6"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}}, 
        {160, 60, 30, 30, "7", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "7"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}},  
        {200, 60, 30, 30, "8", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "8"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}},  
        {240, 60, 30, 30, "9", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "9"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}},

        {280, 60, 30, 30, "<=", 2, []() {if (SerialFrq.length() > 0) SerialFrq.remove(SerialFrq.length() -1); tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}}, 
        {280, 100, 30, 70, "0", 2, []() {if (SerialFrq.length() < 8) SerialFrq += "0"; tft.fillRect(0, 90, 150, 90, TFT_BLACK); tft.drawCentreString(SerialFrq, 80, 100, 4);}},

        {10, 60, 140, 30, "Run Serial port", 2, []() { Serial_Menu2.Draw(); Actual_Menu = &Serial_Menu2;}}

    };
  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  int getButtonsLength() override { return 13; } // 
  void CustomDraw() override {
    tft.drawLine(10, 200, 310, 200, TFT_WHITE);
    tft.fillRect(0, 90, 150, 90, TFT_BLACK);
    tft.drawCentreString(SerialFrq, 80, 100, 4);
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
    tft.drawLine(10, 200, 310, 200, TFT_WHITE);
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
            delay(10);
        }
    }
    tft.println("");

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
  }
  int getButtonsLength() override { return 1; } // 
};
WIFI_Menu_Type WIFI_Menu;

// Пример наследуемого класса: класс главного меню. Имеет кучу кнопок, и ничего более.
class Main_Menu_Type : public Menu {
  public:
  Button buttons[9] = {
        {20, 60, 80, 40, "433Mhz", 2, []() { FoxAnimation("Be careful!", "Actions are limited by law!", TFT_RED); }}, //Пример кнопок. Есть параметры и лямбда-функция.
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

// Обработчик тач-скрина. Получает X и Y нажатия и актуальное меню. Сам ищет среди кнопок совпадения.
void TouchRenderer(Menu* _Menu, uint16_t TouchX, uint16_t TouchY) {
  for (int i = 0; i < _Menu->getButtonsLength(); i++) {
    if (_Menu->getButtons()[i].location_X < TouchX && // Левый край
       _Menu->getButtons()[i].Size_Width + _Menu->getButtons()[i].location_X > TouchX &&  // Правый край
       _Menu->getButtons()[i].location_Y < TouchY && // Верх
       _Menu->getButtons()[i].Size_Height + _Menu->getButtons()[i].location_Y > TouchY) // Низ
      { 
        _Menu->getButtons()[i].Action(); // Если в кнопке - выполняем действие этой кнопки
        if (_Menu->getButtons()[i].Label == "<-") { Main_Menu.Draw(); Actual_Menu = &Main_Menu; }
        DoLog("Do something on button " + _Menu->getButtons()[i].Label + " by clicking at X:" + TouchX + " Y:" + TouchY);
      }
  }
  CoolDown = true;
}
void CreateHTMLFromActual_Menu() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", Actual_Menu->HTML());
  });
    server.on("/1", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[0].Action();
    });
    server.on("/2", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[1].Action();
    });
    server.on("/3", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[2].Action();
    });
    server.on("/4", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[3].Action();
    });
    server.on("/5", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[4].Action();
    });
    server.on("/6", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[5].Action();
    });
    server.on("/7", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[6].Action();
    });
    server.on("/8", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[7].Action();
    });
    server.on("/9", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[8].Action();
    });
    server.on("/10", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[9].Action();
    });
    server.on("/11", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[10].Action();
    });
    server.on("/12", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[11].Action();
    });
    server.on("/13", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[12].Action();
    });
    server.on("/14", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/");
      Actual_Menu->getButtons()[13].Action();
    });
      server.on("/back", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect("/");
        Main_Menu.Draw(); 
        Actual_Menu = &Main_Menu; 
      });
  DoLog("Paint menu "+Actual_Menu->Title()+" on a WEB");
}

void setup() {
  pinMode(IRReceiverPin, INPUT_PULLUP);
  pinMode(IRSenderPin, OUTPUT);

  irsend.begin();       // Start up the IR sender.

  Serial.setTimeout(50);
  if (DebugMode) Serial.begin(SerialFrq.toInt());
  DoLog("Serial began on frequency " + SerialFrq);

  /*WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (DebugMode) DoLog("Connecting to WiFi..");
  }
  if (DebugMode) Serial.println(WiFi.localIP());*/

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    if (DebugMode) DoLog("LittleFS Mount Failed");
    return;
  } else {
    if (DebugMode) DoLog("Little FS Mounted Successfully");
  }

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  FoxAnimation("Welcome!");
  
  uint16_t calData[5] = { 437, 3472, 286, 3526, 3 };
  tft.setTouch(calData);

  Actual_Menu = &Main_Menu;
  Main_Menu.Draw();
  IR_Menu_Resources.setIRMenu(&IR_Menu);

  DoLog(Main_Menu.HTML());
  CreateHTMLFromActual_Menu();
  //server.begin();
}

void loop() {
  uint16_t x = 0, y = 0; // Координаты тача
  if (tft.getTouch(&x, &y) && !CoolDown) { // Если коснулись..
    TouchRenderer(Actual_Menu, x, y); // Обработать нажатие
    CreateHTMLFromActual_Menu();
  }
  FoxyOnCornerLoop();
  Actual_Menu->MenuLoop();
}

// "Пусть кто-то говорит, что ты слаб и бездарен
//  Не слушай и играй на любимой гитаре"
//  - Екатерина Яшникова, "Это возможно".