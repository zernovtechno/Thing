TFT_eSPI tft = TFT_eSPI();  // Объект дисплея
AsyncWebServer server(80);  // Веб-сервер (веб-дисплей)



class ThingClass {
public:
  //Добавленный HTML
  String AddedHTML;
  //Частота связи по UART
  String SerialFrq = "115200";
  //Текущие дата и время
  String DateTime = "Unknown datetime";
  //Режим отладки (вывод инфо в UART)
  bool DebugMode = true;
  bool CoolDown;
  bool GoBackValue;
  bool Clicked = false;
  int ClickedNumber;
  // Нажать на кнопку в меню (Номер кнопки)
  void ClickOnButtonByNumber(int Number) {
    if (!Clicked) {
      Clicked = true;
      ClickedNumber = Number;
    }
  }
  // Вернуться на главный экран
  void GoBack() {
    if (!GoBackValue) {
      GoBackValue = true;
    }
  }
  // Функция для вывода отладки в Serial
  void DoLog(String text) {
    String LogText = "[THING DEBUG] ";
    LogText += "[" + DateTime + "] ";
    LogText += text;
    if (DebugMode) {
      Serial.println(LogText); // Напечатать сконфигурированную строку
    } 
  }
  // Анимация лисички в шляпе на старте.
  void FoxAnimation(String Text, String UpperText = "", uint16_t UpperTextColor = TFT_BLACK) {
    DoLog("Running splash-screen with text \"" + Text + "\" and UpperText - \"" + UpperText + "\"");
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
  // Функция для преобразования RGB 565 в HEX
  String rgb565ToHex(uint16_t rgb565) {
    // Извлечение компонентов RGB из RGB 565
    uint8_t r = (rgb565 >> 11) & 0x1F;  // 5 бит для красного
    uint8_t g = (rgb565 >> 5) & 0x3F;   // 6 бит для зеленого
    uint8_t b = rgb565 & 0x1F;          // 5 бит для синего

    // Преобразование в 8-битные значения
    r = (r * 255) / 31;  // Преобразование 5 бит в 8 бит
    g = (g * 255) / 63;  // Преобразование 6 бит в 8 бит
    b = (b * 255) / 31;  // Преобразование 5 бит в 8 бит

    // Формирование HEX-строки
    char hexColor[8];
    sprintf(hexColor, "#%02X%02X%02X", r, g, b);
    return String(hexColor);
  }
  //Нарисовать пиксель на экране и на WEB-дисплее
  void drawPixel(int x, int y, uint16_t color) {
    tft.drawPixel(x, y, color);
    AddedHTML += "<div style=\"position: absolute; left: " + String(x) + "px; top: " + String(y) + "px; width: 1px; height: 1px; background-color: " + rgb565ToHex(color) + ";\"></div>\n";
  }
  //Нарисовать линию на экране и на WEB-дисплее
  void drawLine(int x1, int y1, int x2, int y2, uint16_t color) {
    tft.drawLine(x1, y1, x2, y2, color);
    AddedHTML += "<div style=\"position: absolute; left: " + String(x1) + "px; top: " + String(y1) + "px; width: " + String(x2 - x1 + 1) + "px; height: " + String(y2 - y1 + 1) + "px; background-color: " + rgb565ToHex(color) + ";\"></div>\n";
  }
  //Нарисовать прямоугольник на экране и на WEB-дисплее
  void drawRect(int x, int y, int width, int height, uint16_t color) {
    tft.drawRect(x, y, width, height, color);
    AddedHTML += "<div style=\"position: absolute; left: " + String(x) + "px; top: " + String(y) + "px; width: " + String(width) + "px; height: " + String(height) + "px; border: 1px solid " + rgb565ToHex(color) + "; background-color: transparent;\"></div>\n";
  }
  //Нарисовать заполненный прямоугольник на экране и на WEB-дисплее
  void fillRect(int x, int y, int width, int height, uint16_t color) {
    tft.fillRect(x, y, width, height, color);
    AddedHTML += "<div style=\"position: absolute; left: " + String(x) + "px; top: " + String(y) + "px; width: " + String(width) + "px; height: " + String(height) + "px; background-color: " + rgb565ToHex(color) + ";\"></div>\n";
  }
  //Нарисовать строку на экране и на WEB-дисплее
  void drawString(String text, int x, int y, int size) {
    tft.drawString(text, x, y, size);
    AddedHTML += "<div class=\"simple-text\" style=\"position: absolute; left: " + String(x) + "px; top: " + String(y) + "px; font-size: " + String(size * 5) + "px; color: white;\">" + text + "</div>\n";
  }
  //Нарисовать центрированную по координатам строку на экране и на WEB-дисплее
  void drawCentreString(String text, int x, int y, int size) {
    tft.drawCentreString(text, x, y, size);
    AddedHTML += "<div class=\"simple-text\" style=\"position: absolute; transform: translate(-50%, 0%); left: " + String(x) + "px; top: " + String(y) + "px; font-size: " + String(size * 5) + "px; color: white;\">" + text + "</div>\n";
  }
};
ThingClass Thing;



// Структура "кнопки". Содержит позицию, размер, текст на самой кнопке, а также ссылку на действие, воспроизводимое по нажатию.
struct Button {
  int location_X;       // Положение X (левый верхний угол)
  int location_Y;       // Положение Y (левый верхний угол)
  int Size_Width;       // Ширина
  int Size_Height;      // Длина
  String Label;         // Надпись
  int Label_Font_Size;  // Размер шрифта (Внимание: Выбирать подбором)

  void (*Action)();  // Ссылка на функцию

  uint16_t BackColor;
  uint16_t ForeColor;
  void Draw(uint16_t _BackColor, uint16_t _ForeColor, bool Fill = false) {
    BackColor = _BackColor;
    ForeColor = _ForeColor;
    tft.fillRect(
      location_X,
      location_Y,
      Size_Width,
      Size_Height,
      BackColor);
    tft.drawRect(
      location_X,
      location_Y,
      Size_Width,
      Size_Height,
      ForeColor);
    tft.setTextColor(ForeColor);
    tft.drawCentreString(
      Label,
      location_X + Size_Width / 2,
      location_Y + Size_Height / 4,
      Label_Font_Size);
    tft.setTextColor(TFT_WHITE);
  }

  String DrawHTML(int i) {
    if (Label == "<-")
      return "<a href=\"/back\" class=\"button simple-button\" style=\"top: " + String(location_Y) + "; left: " + String(location_X) + "px; width: " + String(Size_Width) + "px; height: " + String(Size_Height) + "px; background-color: black; color: white;\">" + Label + "</a>\n";
    else
      return "<a href=\"/?button=" + String(i) + "\" class=\"simple-button\" style=\"top: " + String(location_Y) + "; left: " + String(location_X) + "px; width: " + String(Size_Width) + "px; height: " + String(Size_Height) + "px; background-color: black; color: white;\">" + Label + "</a>\n";
  }
};



// База - родительский класс для всех меню.
class Menu {
public:  // Модификатор доступа
  virtual String Title() {
    return "Thing.";
  };                  // Текст слева вверху
  Button buttons[1];  // Массив с кнопками
  virtual Button* getButtons() {
    return buttons;
  }  // Виртуальная функция для получения значения
  virtual int getButtonsLength() {
    return 1;
  }                             // Количество кнопок
  virtual void CustomDraw() {}  // Количество кнопок
  virtual void MenuLoop(){};    // Цикл, вызываемый для актуального (загруженного) меню.

  virtual String HTML() {
    String HTMLSTR = "<html>\n\
    <head><meta charset=\"UTF-8\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <script>\n\
    setInterval(() => {fetch(\'/\').then(response => response.text()).then(html => {document.body.innerHTML = html;}).catch(error => console.error('Error updating page:', error))},500); \n\
    </script>\n\
    <title>" + Title()
                     + "</title>\n\
    <style>\n\
        body {\n\
            background-color: black;\n\
            position: relative;\n\
			      margin: 0; \n\
			      padding: 0; \n\
			      border: 0;\n\
        }\n\
        .header {\n\
            background-color: white;\n\
            position: absolute;\n\
			      color: white;\n\
        }\n\
        .simple-button {\n\
            border: 1px solid white;\n\
            text-decoration: none;\n\
            display: flex;\n\
            justify-content: center;\n\
            cursor: pointer;\n\
            position: absolute;\n\
			      align-items: center;\n\
        }\n\
		    .simple-text {\n\
            position: absolute;\n\
        }\n\
		    .base64-image {\n\
            image-rendering: pixelated;\n\
            position: absolute;\n\
            left: 270px;\n\
        }	\n\
    </style>\n\
  </head>\n\
  <body>\n\
	<div class=\"header\" style=\"top: 40px; left: 10px; background-color: white; width: 250px; height: 1px;\"></div>\n\
	<div class=\"simple-text\" style=\"top: 10px; left: 10px; width: 200px; height: 40px; font-size: 25px; color: white;\">"
                     + Title() + "</div>\n\
  <img class=\"base64-image\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAAAtCAYAAAAeA21aAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAMeSURBVGhD7Zk/ctUwEMb9khYqLuDchMxQMvSUNOEOOQBUHIA0lFyAoeUCXIGxD0CKTOgf+qRdv5Us648lPRfPv5kdyYktW6vdb2W/bmcbemVHsk25onZHMCj7ZbpNwPgwjoDW98uGH6oVPHFpLe8XhFPAzcnaOWqN179UnTtjW3PxGjBzwPBeJeRbOjCU5qh1vWf8TZk5AOEJEyB8YWuxrveMvykHavGAWojcvDw8UKcATBgrP/FK3e6T0b3DQT/CqOwGnXNz8Rowj4C/qnlUC/LlFofd+Gzs9oc+tFcygynsaeV1FCg8EUAnJFMUOXMHHFWlggPuT+PCATffTb+4dH21K6vHAbmll+ewCr8DJB+Lxvcz14CJFAfLBSFWa8iuAdTOU4A0QPdbwBrwedT6wHuD1BKJKAAiEvhBsyJhOQIw8VaTB874mHjq5IHnfHjUeDWDeQREctC3L8CDhKqDm7PyfIyXe32ELNHaNYDaYARgBXgfwLnHDMPQjePYfXhnNMNdSV45vg7nI/T7B3M+RxSHM66X9wP9i6779poOIqj7Ia9gJGJhkiMAD+VOHvR9r23p/+7f+XwX9zw+1vbPOCjFFFla4MuXIx5QrxRw9gG1NQDE/p+LM15QE3YNoFaiQ0ih3+GH36pBuRLvBj4o/BZxr+PzsVK+HI+NF4PvF9OEpfA4iaLn3aAmKWWwBEpZOMA7gWB+ELYmgIBDfBoRovjlKkLMAbsGUBvC1gQOVbGNlSD3YLKOS9xQL831GDVSAJw0Yd3r6kTrkHepogHUJtNS1HLZNSBCqAyanB8G9NOh6tA6t1NZmwL2PiCHFp/QCljjgHndz6XhxikFR4SDK3LxGsAO4JAvWHYBf/Xl3wDOCFb+zc8p5KNhKCMATsgTvBBwAn34PCcI/z9PpksWpL4IujQWRRI5SdYN930AtT50/KqKYCVydnVARYDx7wwVkO8aqu/meTTsJSnhYuXAqpSoXBZzylyMNReX7xMkCRrhTBhglat4dNcAanOANkzfB4pZ+K4gub7qRlXaXBHJyvXawAkQg3NZox1V1/0HQtRnmAgP6AoAAAAASUVORK5CYII=\" />\n";
    for (int i = 1; i <= getButtonsLength(); i++) {
      HTMLSTR += buttons[i].DrawHTML(i);
    }
    HTMLSTR += Thing.AddedHTML + "</body>\n\
  </html>";  // HTML заканчивается
    return HTMLSTR;
  };

  // Функция рисования меню. По умолчанию отрисовывает все кнопки в нём.
  void Draw() {
    Thing.AddedHTML = "";
    tft.fillRect(10, 10, 200, 39, TFT_BLACK);
    tft.drawString(Title(), 10, 10, 4);
    tft.drawLine(10, 40, 240, 40, TFT_WHITE);
    tft.fillRect(0, 50, 320, 190, TFT_BLACK);
    for (int i = 0; i <= getButtonsLength(); i++) {
      buttons[i].Draw(TFT_BLACK, TFT_WHITE);
    }

    CustomDraw();
  }
  void Touch(uint16_t TouchX, uint16_t TouchY) {
    for (int i = 0; i < getButtonsLength(); i++) {
      if (getButtons()[i].location_X < TouchX &&                               // Левый край
          getButtons()[i].Size_Width + getButtons()[i].location_X > TouchX &&  // Правый край
          getButtons()[i].location_Y < TouchY &&                               // Верх
          getButtons()[i].Size_Height + getButtons()[i].location_Y > TouchY)   // Низ
      {
        getButtons()[i].Action();  // Если в кнопке - выполняем действие этой кнопки
        if (getButtons()[i].Label == "<-") { Thing.GoBack(); }
        Thing.DoLog("Do something on button " + getButtons()[i].Label + " by clicking at X:" + TouchX + " Y:" + TouchY);
      }
    }
    Thing.CoolDown = true;
  }
};