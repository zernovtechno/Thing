//////////////////////////////////////////////////////////////////////////////////
//    _________    ___   ___      __________      ___   ___       _______       //
//   /________/\  /__/\ /__/\    /_________/\    /__/\ /__/\     /______/\      //
//   \__    __\/  \  \ \\  \ \   \___    __\/    \  \_\\  \ \    \    __\/__    //
//      \  \ \     \  \/_\  \ \      \  \ \       \   `-\  \ \    \ \ /____/\   //
//       \  \ \     \   ___  \ \     _\  \ \__     \   _    \ \    \ \\_  _\/   //
//        \  \ \     \  \ \\  \ \   /__\  \__/\     \  \`-\  \ \    \ \_\ \ \   //
//         \__\/      \__\/ \__\/   \________\/      \__\/ \__\/     \_____\/   //
//                                                                              // 
//                         Settings. Set up your Thing.                         //
//                       Настройки. Настройте свой Thing.                       //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

class Settings_Menu_Type : public Menu {
  public:
  String Title() override { return "Settings.";}
  Button buttons[3] = { 
    {10, 200, 45, 30, "<-", 2, []() { }}, 
    {100, 50, 50, 30, "Black", 2, []() {  }}, //Пример кнопок. Есть параметры и лямбда-функция.
    {160, 90, 50, 30, "White", 2, []() {  }}, //Пример кнопок. Есть параметры и лямбда-функция.
    
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() override {
  }
  void CustomDraw() override {
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    Thing.drawString("Foreground color", 10, 56, 2);
    Thing.drawString("Background color", 10, 96, 2);
  }
  int getButtonsLength() override { return 3; } // 
};
Settings_Menu_Type Settings_Menu;