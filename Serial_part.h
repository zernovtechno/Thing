//////////////////////////////////////////////////////////////////////////////////
//    _________    ___   ___      __________      ___   ___       _______       //
//   /________/\  /__/\ /__/\    /_________/\    /__/\ /__/\     /______/\      //
//   \__    __\/  \  \ \\  \ \   \___    __\/    \  \_\\  \ \    \    __\/__    //
//      \  \ \     \  \/_\  \ \      \  \ \       \   `-\  \ \    \ \ /____/\   //
//       \  \ \     \   ___  \ \     _\  \ \__     \   _    \ \    \ \\_  _\/   //
//        \  \ \     \  \ \\  \ \   /__\  \__/\     \  \`-\  \ \    \ \_\ \ \   //
//         \__\/      \__\/ \__\/   \________\/      \__\/ \__\/     \_____\/   //
//                                                                              // 
//                Serial tool. Connect and listen to serial port.               //
//        Serial утилита. Подключайтесь и слушайте последовательный порт.       //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////


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

        {10, 60, 140, 30, "Run Serial port", 2, []() { 
          //Serial_Menu2.Draw(); Actual_Menu = &Serial_Menu2;
          Keyboard_Menu.Draw(); Actual_Menu = &Keyboard_Menu;
          StrPointer = &Thing.SerialFrq; MenuPointer = &Serial_Menu2;}}

    };
  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  int getButtonsLength() override { return 13; } // 
  void CustomDraw() override {
    UpdateSerialFRQ();
  }
};
Serial_Menu_Type Serial_Menu;