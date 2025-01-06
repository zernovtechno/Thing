//////////////////////////////////////////////////////////////////////////////////
//    _________    ___   ___      __________      ___   ___       _______       //
//   /________/\  /__/\ /__/\    /_________/\    /__/\ /__/\     /______/\      //
//   \__    __\/  \  \ \\  \ \   \___    __\/    \  \_\\  \ \    \    __\/__    //
//      \  \ \     \  \/_\  \ \      \  \ \       \   `-\  \ \    \ \ /____/\   //
//       \  \ \     \   ___  \ \     _\  \ \__     \   _    \ \    \ \\_  _\/   //
//        \  \ \     \  \ \\  \ \   /__\  \__/\     \  \`-\  \ \    \ \_\ \ \   //
//         \__\/      \__\/ \__\/   \________\/      \__\/ \__\/     \_____\/   //
//                                                                              // 
//       GPIO tool. Control digital ports, and read analog. Oscilloscope.       //
// GPIO утилита. Контроллируйте цифровые пины и читайте аналоговые. Осциллограф.//
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////


bool GPIO25State = false;
bool GPIO26State = false;
bool Update_GPIO = true;
bool Update_ADCms = true;
bool Update_ADCpin = true;
int GraphWide = 75;
int GraphHeight = 170;
int GraphHeightOld = 170;
int OscilloscopicTimer = 500;
int ADCPin = 39;
unsigned long GPIO_ADC_TIMER;

class GPIO_Menu_Type : public Menu {
  public:
  String Title() override { return "GPIO.";}
  Button buttons[9] = {
        {10, 200, 40, 30, "<-", 2, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {70, 50, 50, 30, "Wait", 2, []() { GPIO25State = !GPIO25State; Update_GPIO = true; digitalWrite(25, GPIO25State); }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {70, 90, 50, 30, "Wait", 2, []() { GPIO26State = !GPIO26State; Update_GPIO = true; digitalWrite(26, GPIO26State); }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {15, 135, 50, 15, "500ms", 1, []() { OscilloscopicTimer = 500; Update_ADCms = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {15, 150, 50, 15, "100ms", 1, []() { OscilloscopicTimer = 100; Update_ADCms = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {15, 165, 50, 15, "10ms", 1, []() { OscilloscopicTimer = 10; Update_ADCms = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {220, 100, 30, 30, "39", 2, []() { ADCPin = 39; Update_ADCpin = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {250, 100, 30, 30, "36", 2, []() { ADCPin = 36; Update_ADCpin = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {280, 100, 30, 30, "34", 2, []() { ADCPin = 34; Update_ADCpin = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() override {
    if (Update_GPIO) {
      getButtons()[1].Label = GPIO25State ? "ON" : "OFF";
      getButtons()[2].Label = GPIO26State ? "ON" : "OFF";
      getButtons()[1].Draw(TFT_BLACK, TFT_WHITE);
      getButtons()[2].Draw(TFT_BLACK, TFT_WHITE);
      Update_GPIO = false;
    }
    if (Update_ADCms) {
      getButtons()[3].Draw((OscilloscopicTimer == 500) ? TFT_WHITE : TFT_BLACK, (OscilloscopicTimer == 500) ? TFT_BLACK : TFT_WHITE);
      getButtons()[4].Draw((OscilloscopicTimer == 100) ? TFT_WHITE : TFT_BLACK, (OscilloscopicTimer == 100) ? TFT_BLACK : TFT_WHITE);
      getButtons()[5].Draw((OscilloscopicTimer == 10) ? TFT_WHITE : TFT_BLACK, (OscilloscopicTimer == 10) ? TFT_BLACK : TFT_WHITE);
      Update_ADCms = false;
    }
    if (Update_ADCpin) {
      getButtons()[6].Draw((ADCPin == 39) ? TFT_WHITE : TFT_BLACK, (ADCPin == 39) ? TFT_BLACK : TFT_WHITE);
      getButtons()[7].Draw((ADCPin == 36) ? TFT_WHITE : TFT_BLACK, (ADCPin == 36) ? TFT_BLACK : TFT_WHITE);
      getButtons()[8].Draw((ADCPin == 34) ? TFT_WHITE : TFT_BLACK, (ADCPin == 34) ? TFT_BLACK : TFT_WHITE);
      Update_ADCpin = false;
    }
    if (millis() - GPIO_ADC_TIMER >= OscilloscopicTimer) {
      GPIO_ADC_TIMER = millis();
      if (GraphWide <= 295) GraphWide = GraphWide + 5; 
      else { GraphWide = 75; tft.fillRect(70, 131, 231, 48, TFT_BLACK); }
      GraphHeight = 172 - (analogRead(ADCPin) / 100);
      Thing.drawLine(GraphWide-5, GraphHeightOld, GraphWide, GraphHeight, TFT_RED);
      GraphHeightOld = GraphHeight;
    }
  }
  void CustomDraw() override {
    pinMode(39, INPUT);
    pinMode(36, INPUT);
    pinMode(34, INPUT);
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    Thing.drawString("GPIO 25", 10, 56, 2);
    Thing.drawString("GPIO 26", 10, 96, 2);
    Thing.drawLine(130, 50, 130, 120, TFT_WHITE);
    Thing.drawRect(10, 130, 300, 55, TFT_WHITE);
    Update_GPIO = true;
    Update_ADCms = true;
    Update_ADCpin = true;
  }
  int getButtonsLength() override { return 9; } // 
};
GPIO_Menu_Type GPIO_Menu;