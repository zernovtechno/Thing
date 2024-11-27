// Структура "кнопки". Содержит позицию, размер, текст на самой кнопке, а также ссылку на действие, воспроизводимое по нажатию.
struct Button {
  int location_X; // Положение X (левый верхний угол)
  int location_Y; // Положение Y (левый верхний угол)
  int Size_Width; // Ширина
  int Size_Height; // Длина
  String Label; // Надпись
  int Label_Font_Size; // Размер шрифта (Внимание: Выбирать подбором)

  void (*Action)(); // Ссылка на функцию

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
        BackColor
      );
      tft.drawRect(
        location_X, 
        location_Y, 
        Size_Width, 
        Size_Height, 
        ForeColor
      );
      tft.setTextColor(ForeColor);
      tft.drawCentreString(
        Label,
        location_X + Size_Width / 2,
        location_Y + Size_Height / 4,
        Label_Font_Size
      );
      tft.setTextColor(TFT_WHITE);
  }

  String DrawHTML(int i) {
    if (Label == "<-")
          return "<a href=\"/back\" class=\"button simple-button\" style=\"top: "+String(location_Y)+"; left: "+String(location_X)+"px; width: "+String(Size_Width)+"px; height: "+String(Size_Height)+"px; background-color: black; color: white;\">"+Label+"</a>\n";
    else
          return "<a href=\"/"+String(i)+"\" class=\"simple-button\" style=\"top: "+String(location_Y)+"; left: "+String(location_X)+"px; width: "+String(Size_Width)+"px; height: "+String(Size_Height)+"px; background-color: black; color: white;\">"+Label+"</a>\n";
  }
};

// База - родительский класс для всех меню. 
class Menu {
  public: // Модификатор доступа
  virtual String Title() { return "Thing.";}; // Текст слева вверху
  Button buttons[1]; // Массив с кнопками
  virtual Button* getButtons() { return buttons; } // Виртуальная функция для получения значения
  virtual int getButtonsLength() { return 1; } // Количество кнопок
  virtual void CustomDraw() { } // Количество кнопок
  virtual void MenuLoop() {}; // Цикл, вызываемый для актуального (загруженного) меню.
 
  virtual String HTML() {
    String HTMLSTR = "<html>\n\
    <head><meta charset=\"UTF-8\">\n\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
    <title>Buttons</title>\n\
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
            border: 3px solid white;\n\
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
            position: absolute;\n\
            left: 270px;\n\
        }	\n\
    </style>\n\
  </head>\n\
  <body>\n\
	<div class=\"header\" style=\"top: 40px; left: 10px; background-color: white; width: 250px; height: 3px;\"></div>\n\
	<div class=\"simple-text\" style=\"top: 10px; left: 10px; width: 200px; height: 40px; font-size: 25px; color: white;\">"+Title()+"</div>\n\
  <img class=\"base64-image\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAAAtCAYAAAAeA21aAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAMeSURBVGhD7Zk/ctUwEMb9khYqLuDchMxQMvSUNOEOOQBUHIA0lFyAoeUCXIGxD0CKTOgf+qRdv5Us648lPRfPv5kdyYktW6vdb2W/bmcbemVHsk25onZHMCj7ZbpNwPgwjoDW98uGH6oVPHFpLe8XhFPAzcnaOWqN179UnTtjW3PxGjBzwPBeJeRbOjCU5qh1vWf8TZk5AOEJEyB8YWuxrveMvykHavGAWojcvDw8UKcATBgrP/FK3e6T0b3DQT/CqOwGnXNz8Rowj4C/qnlUC/LlFofd+Gzs9oc+tFcygynsaeV1FCg8EUAnJFMUOXMHHFWlggPuT+PCATffTb+4dH21K6vHAbmll+ewCr8DJB+Lxvcz14CJFAfLBSFWa8iuAdTOU4A0QPdbwBrwedT6wHuD1BKJKAAiEvhBsyJhOQIw8VaTB874mHjq5IHnfHjUeDWDeQREctC3L8CDhKqDm7PyfIyXe32ELNHaNYDaYARgBXgfwLnHDMPQjePYfXhnNMNdSV45vg7nI/T7B3M+RxSHM66X9wP9i6779poOIqj7Ia9gJGJhkiMAD+VOHvR9r23p/+7f+XwX9zw+1vbPOCjFFFla4MuXIx5QrxRw9gG1NQDE/p+LM15QE3YNoFaiQ0ih3+GH36pBuRLvBj4o/BZxr+PzsVK+HI+NF4PvF9OEpfA4iaLn3aAmKWWwBEpZOMA7gWB+ELYmgIBDfBoRovjlKkLMAbsGUBvC1gQOVbGNlSD3YLKOS9xQL831GDVSAJw0Yd3r6kTrkHepogHUJtNS1HLZNSBCqAyanB8G9NOh6tA6t1NZmwL2PiCHFp/QCljjgHndz6XhxikFR4SDK3LxGsAO4JAvWHYBf/Xl3wDOCFb+zc8p5KNhKCMATsgTvBBwAn34PCcI/z9PpksWpL4IujQWRRI5SdYN930AtT50/KqKYCVydnVARYDx7wwVkO8aqu/meTTsJSnhYuXAqpSoXBZzylyMNReX7xMkCRrhTBhglat4dNcAanOANkzfB4pZ+K4gub7qRlXaXBHJyvXawAkQg3NZox1V1/0HQtRnmAgP6AoAAAAASUVORK5CYII=\" />\n";
  for (int i = 1; i <= getButtonsLength(); i++) {
      HTMLSTR += buttons[i].DrawHTML(i);
  }
  HTMLSTR += "</body>\n\
  </html>"; // HTML заканчивается
  return HTMLSTR;
  };

  // Функция рисования меню. По умолчанию отрисовывает все кнопки в нём.
  void Draw() {
    tft.fillRect(10, 10, 200, 39, TFT_BLACK);
    tft.drawString(Title(), 10, 10, 4);
    tft.drawLine(10, 40, 240, 40, TFT_WHITE);
    tft.fillRect(0, 50, 320, 190, TFT_BLACK);
    for (int i = 0; i <= getButtonsLength(); i++) {
      buttons[i].Draw(TFT_BLACK, TFT_WHITE);
    }

    CustomDraw();
  }

};