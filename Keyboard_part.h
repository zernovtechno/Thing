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

String EnglishLayout = "1234567890qwertyuiopasdfghjklzxcvbnm";
int SelectedLetter;

String Text = "";

void AddLetterToText(const char* Letter) {
  Text += Letter;
  Thing.AddedHTML = "";
  Thing.fillRect(0, 45, 320, 30, TFT_BLACK);
  Thing.drawCentreString(Text, 160, 60, 2);
  Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
}

void DeleteLetterToText() {
  Text.remove(Text.length() - 1); ;
  Thing.AddedHTML = "";
  Thing.fillRect(0, 45, 320, 30, TFT_BLACK);
  Thing.drawCentreString(Text, 160, 60, 2);
  Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
}

class Keyboard_Menu_Type : public Menu {
  public:
  String Title() override { return "Keyboard.";}
  Button buttons[43] = {
        {10, 200, 40, 30, "<-", 2, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.

        {15, 80, 20, 20, "1", 1, []() {AddLetterToText("1"); }},
        {45, 80, 20, 20, "2", 1, []() {AddLetterToText("2"); }},
        {75, 80, 20, 20, "3", 1, []() {AddLetterToText("3"); }},
        {105, 80, 20, 20, "4", 1, []() {AddLetterToText("4"); }},
        {135, 80, 20, 20, "5", 1, []() {AddLetterToText("5"); }},
        {165, 80, 20, 20, "6", 1, []() {AddLetterToText("6"); }},
        {195, 80, 20, 20, "7", 1, []() {AddLetterToText("7"); }},
        {225, 80, 20, 20, "8", 1, []() {AddLetterToText("8"); }},
        {255, 80, 20, 20, "9", 1, []() {AddLetterToText("9"); }},
        {285, 80, 20, 20, "0", 1, []() {AddLetterToText("0"); }},

        {15, 110, 20, 20, "Q", 1, []() {AddLetterToText("Q"); }},
        {45, 110, 20, 20, "W", 1, []() {AddLetterToText("W"); }},
        {75, 110, 20, 20, "E", 1, []() {AddLetterToText("E"); }},
        {105, 110, 20, 20, "R", 1, []() {AddLetterToText("R"); }},
        {135, 110, 20, 20, "T", 1, []() {AddLetterToText("T"); }},
        {165, 110, 20, 20, "Y", 1, []() {AddLetterToText("Y"); }},
        {195, 110, 20, 20, "U", 1, []() {AddLetterToText("U"); }},
        {225, 110, 20, 20, "I", 1, []() {AddLetterToText("I"); }},
        {255, 110, 20, 20, "O", 1, []() {AddLetterToText("O"); }},
        {285, 110, 20, 20, "P", 1, []() {AddLetterToText("P"); }},

        {30, 140, 20, 20, "A", 1, []() {AddLetterToText("A"); }},
        {60, 140, 20, 20, "S", 1, []() {AddLetterToText("S"); }},
        {90, 140, 20, 20, "D", 1, []() {AddLetterToText("D"); }},
        {120, 140, 20, 20, "F", 1, []() {AddLetterToText("F"); }},
        {150, 140, 20, 20, "G", 1, []() {AddLetterToText("G"); }},
        {180, 140, 20, 20, "H", 1, []() {AddLetterToText("H"); }},
        {210, 140, 20, 20, "J", 1, []() {AddLetterToText("J"); }},
        {240, 140, 20, 20, "K", 1, []() {AddLetterToText("K"); }},
        {270, 140, 20, 20, "L", 1, []() {AddLetterToText("L"); }},

        {50, 170, 20, 20, "Z", 1, []() {AddLetterToText("Z"); }},
        {80, 170, 20, 20, "X", 1, []() {AddLetterToText("X"); }},
        {110, 170, 20, 20, "C", 1, []() {AddLetterToText("C"); }},
        {140, 170, 20, 20, "V", 1, []() {AddLetterToText("V"); }},
        {170, 170, 20, 20, "B", 1, []() {AddLetterToText("B"); }},
        {200, 170, 20, 20, "N", 1, []() {AddLetterToText("N"); }},
        {230, 170, 20, 20, "M", 1, []() {AddLetterToText("M"); }},

        {260, 170, 50, 20, "<=", 1, []() {DeleteLetterToText(); }},
        {120, 200, 80, 30, "", 2, []() {AddLetterToText(" "); }},
        {260, 200, 50, 30, "ENTER", 2, []() {AddLetterToText("/"); }},
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность

  /*void SetLayout(String Layout) {
    int Xoffset = 15;
    int Yoffset = 80;
    int Line = 1;
    for (int i = 1; i < getButtonsLength(); i++) {
      Thing.DoLog(String(i));
      SelectedLetter = i-1;
      buttons[i] = (Button) {Xoffset, Yoffset, 20, 20, String(EnglishLayout[SelectedLetter]), 1, []() { AddLetterToText(EnglishLayout[SelectedLetter]); }};
      Xoffset += 30;
      if (Xoffset >= 300) {
        Line++;
        switch (Line) {
          case 2:
            Xoffset = 15;
            break;
          case 3:
            Xoffset = 30;
            break;
          case 4:
            Xoffset = 50;
            break;
        }
        Yoffset = Yoffset + 30;
      }
    }
  }*/
  void MenuLoop() override {
  }
  void CustomDraw() override {
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    Thing.drawCentreString(Text, 160, 60, 2);
  }
  int getButtonsLength() override { return 43; } // 
};
Keyboard_Menu_Type Keyboard_Menu;