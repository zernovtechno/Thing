//////////////////////////////////////////////////////////////////////////////////
//    _________    ___   ___      __________      ___   ___       _______       //
//   /________/\  /__/\ /__/\    /_________/\    /__/\ /__/\     /______/\      //
//   \__    __\/  \  \ \\  \ \   \___    __\/    \  \_\\  \ \    \    __\/__    //
//      \  \ \     \  \/_\  \ \      \  \ \       \   `-\  \ \    \ \ /____/\   //
//       \  \ \     \   ___  \ \     _\  \ \__     \   _    \ \    \ \\_  _\/   //
//        \  \ \     \  \ \\  \ \   /__\  \__/\     \  \`-\  \ \    \ \_\ \ \   //
//         \__\/      \__\/ \__\/   \________\/      \__\/ \__\/     \_____\/   //
//                                                                              // 
//               WiFi tool. Scan and connect to wifi networks.                  //
//              WiFi утилита. Сканируйте и подключайтесь к WIFI.                //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

struct WiFiNetwork {
    String SSID;
    int RSSI;
    int Channel;
    String StringEncryptionType;
    byte ByteEncryptionType;

    void FillEncryptionType(byte encryptionType) {
      ByteEncryptionType = encryptionType;
      switch (encryptionType)
            {
            case WIFI_AUTH_OPEN:
                StringEncryptionType = "open";
                break;
            case WIFI_AUTH_WEP:
                StringEncryptionType = "WEP";
                break;
            case WIFI_AUTH_WPA_PSK:
                StringEncryptionType = "WPA";
                break;
            case WIFI_AUTH_WPA2_PSK:
                StringEncryptionType = "WPA2";
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                StringEncryptionType = "WPA+WPA2";
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                StringEncryptionType = "WPA2-EAP";
                break;
            case WIFI_AUTH_WPA3_PSK:
                StringEncryptionType = "WPA3";
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                StringEncryptionType = "WPA2+WPA3";
                break;
            case WIFI_AUTH_WAPI_PSK:
                StringEncryptionType = "WAPI";
                break;
            default:
                StringEncryptionType = "unknown";
            }
    }
  };
WiFiNetwork WiFiNetworks[10];

bool ReScan = true;
int NetworksCount = 0;

class WIFI_Menu_Type : public Menu {
  public:
  String Title() override { return "Wi-Fi.";}
  Button buttons[8] = {
        {10, 200, 40, 30, "<-", 2, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {140, 200, 40, 30, "ReScan", 2, []() { ReScan = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.

        {10, 70, 300, 20, "SSID | RSSI | CHANNEL | ENCRYPTION", 1, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 90, 300, 20, " ", 1, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 110, 300, 20, " ", 1, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 130, 300, 20, " ", 1, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 150, 300, 20, " ", 1, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 170, 300, 20, " ", 1, []() { }} //Пример кнопок. Есть параметры и лямбда-функция.
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() override {
    if (ReScan) {
      Thing.DoLog("Go Scanning WiFi");
      NetworksCount = WiFi.scanNetworks();
      Thing.DoLog("Scanned. Found " + String(NetworksCount));
      if (NetworksCount != 0) {
        for (int i = 0; i < 5; ++i) {
            Thing.DoLog("Remembering. Iteraion " + String(i));
            WiFiNetworks[i].SSID = WiFi.SSID(i).c_str();
            WiFiNetworks[i].RSSI = WiFi.RSSI(i);
            WiFiNetworks[i].Channel = WiFi.channel(i);
            WiFiNetworks[i].FillEncryptionType(WiFi.encryptionType(i));
            buttons[i+3].Label = WiFiNetworks[i].SSID + " " 
            + WiFiNetworks[i].RSSI + " " 
            + WiFiNetworks[i].Channel + " " 
            + WiFiNetworks[i].StringEncryptionType;
        }
      }
      Thing.DoLog("Deleting");
      WiFi.scanDelete();
      Draw();
      ReScan = false;
    }
  }
  void CustomDraw() override {
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    if (NetworksCount == 0) {
          Thing.drawString("No networks found", 10, 50, 2);
    } else {
        Thing.drawString("Found "+String(NetworksCount)+" networks", 10, 50, 2);
    }
  }
  int getButtonsLength() override { return 8; } // 
};
WIFI_Menu_Type WIFI_Menu;