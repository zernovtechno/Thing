//////////////////////////////////////////////////////////////////////////////////
//    _________    ___   ___      __________      ___   ___       _______       //
//   /________/\  /__/\ /__/\    /_________/\    /__/\ /__/\     /______/\      //
//   \__    __\/  \  \ \\  \ \   \___    __\/    \  \_\\  \ \    \    __\/__    //
//      \  \ \     \  \/_\  \ \      \  \ \       \   `-\  \ \    \ \ /____/\   //
//       \  \ \     \   ___  \ \     _\  \ \__     \   _    \ \    \ \\_  _\/   //
//        \  \ \     \  \ \\  \ \   /__\  \__/\     \  \`-\  \ \    \ \_\ \ \   //
//         \__\/      \__\/ \__\/   \________\/      \__\/ \__\/     \_____\/   //
//                                                                              // 
//            IR tool. Read, decode, remember and repeat IR signals.            //
//    ИК утилита. Читайте, декодируйте, запоминайте и повторяйте ИК сигналы.    //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////



IRsend irsend(IRSenderPin); // ИК передатчик.
IRrecv irrecv(IRReceiverPin, 1024, 100, false); // ИК приёмник.

decode_results IRResult; // Результат ИК-декодирования

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