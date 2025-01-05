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