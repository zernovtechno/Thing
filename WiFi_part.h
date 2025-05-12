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
#include "esp_wifi.h"

String toString(IPAddress ip) { // IP v4 only
  String ips;
  ips.reserve(16);
  ips = ip[0];  ips += ':';
  ips += ip[1]; ips += ':';
  ips += ip[2]; ips += ':';
  ips += ip[3];
  return ips;
}

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
WiFiNetwork WiFiNetworks[50];

bool ReScan = true;
bool DrawButtons = false;
int NetworksCount = 0;
int SelectedNetwork;
int SelectedArray;
bool DoJamm = false;

uint8_t noisePacket[] = {
  0x80, 0x00, // Frame Control (beacon frame)
  0x00, 0x00, // Duration
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination (broadcast)
  0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, // Source (example MAC)
  0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, // BSSID (example MAC)
  0x00, 0x00, // Sequence number
  // Additional payload to make packet larger
  0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x69, 0x46, 0x69, // "Hello WiFi"
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, // Padding
  0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21  // More padding
};

const size_t packetLength = sizeof(noisePacket); // Ensure the packet size matches the buffer size

void sendNoisePacket() {
  if (packetLength < 20) { // Minimum valid length for 802.11 frames
    Thing.DoLog("Error: Packet too short");
    return;
  }

  // Send raw packet to generate noise
  esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, noisePacket, packetLength, false);
  if (result == ESP_OK) {
    Thing.DoLog("Sent noise packet");
  } else {
    Thing.DoLog("Failed to send packet, error code: ");
  }
}

void StartJamm() {
    WiFi.mode(WIFI_STA); // Set Wi-Fi mode to Station (client)
    esp_wifi_set_promiscuous(true); // Enable promiscuous mode to send raw packets
    esp_wifi_set_channel(WiFiNetworks[SelectedNetwork].Channel, WIFI_SECOND_CHAN_NONE);
    DoJamm = true;
  }

  void StopJamm() {
    WiFi.mode(WIFI_STA); // Set Wi-Fi mode to Station (client)
    esp_wifi_set_promiscuous(false); // Enable promiscuous mode to send raw packets
    esp_wifi_set_channel(WiFiNetworks[SelectedNetwork].Channel, WIFI_SECOND_CHAN_NONE);
    DoJamm = false;
  }

class WIFI_Choose_Type : public Menu {
  public:
  String Title() override { return "Wi-Fi.";}
  Button buttons[3] = {
        {10, 200, 40, 30, "<-", 2, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.

        {40, 100, 100, 30, "Connect", 2, []() { MenuPointer = Actual_Menu; Keyboard_Menu.Draw(); Actual_Menu = &Keyboard_Menu; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {180, 100, 100, 30, "Jamm", 2, []() {  if (DoJamm) StopJamm(); else StartJamm(); Actual_Menu->Draw(); }}, //Пример кнопок. Есть параметры и лямбда-функция.
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() override {
    if (TextChanged) {
      TextChanged = false;
      WiFi.begin(WiFiNetworks[SelectedNetwork].SSID, Text);
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Thing.DoLog("Connecting to WiFi..");
      }
      Thing.drawCentreString("Connected. IP: " + toString(WiFi.localIP()), 160, 150, 2);
      if (Thing.DebugMode) Serial.println(WiFi.localIP());
    }
    if (DoJamm) sendNoisePacket();
  }
  void CustomDraw() override {
    Thing.drawLine(10, 200, 310, 200, TFT_WHITE);
    Thing.drawCentreString(WiFiNetworks[SelectedNetwork].SSID + " " 
            + WiFiNetworks[SelectedNetwork].RSSI + " " 
            + WiFiNetworks[SelectedNetwork].Channel + " " 
            + WiFiNetworks[SelectedNetwork].StringEncryptionType, 160, 60, 2);
    if (DoJamm) Thing.drawCentreString("Jamming...", 160, 150, 2);
  }
  int getButtonsLength() override { return 3; } // 
};
WIFI_Choose_Type WIFI_Choose;

int DisplayedScreen = 0;

class WIFI_Menu_Type : public Menu {
  public:
  String Title() override { return "Wi-Fi.";}
  Button buttons[9] = {
        {10, 200, 40, 30, "<-", 2, []() {  DisplayedScreen = 0; DrawButtons = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {130, 200, 60, 30, "ReScan", 2, []() { ReScan = true; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {270, 200, 40, 30, "->", 2, []() { if (DisplayedScreen < 10) { DisplayedScreen++; DrawButtons = true; } }},

        {10, 70, 300, 20, "SSID | RSSI | CHANNEL | ENCRYPTION", 1, []() { }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 90, 300, 20, " ", 1, []() { SelectedNetwork = DisplayedScreen*5+0; WIFI_Choose.Draw(); Actual_Menu = &WIFI_Choose; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 110, 300, 20, " ", 1, []() { SelectedNetwork = DisplayedScreen*5+1; WIFI_Choose.Draw(); Actual_Menu = &WIFI_Choose; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 130, 300, 20, " ", 1, []() { SelectedNetwork = DisplayedScreen*5+2; WIFI_Choose.Draw(); Actual_Menu = &WIFI_Choose; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 150, 300, 20, " ", 1, []() { SelectedNetwork = DisplayedScreen*5+3; WIFI_Choose.Draw(); Actual_Menu = &WIFI_Choose; }}, //Пример кнопок. Есть параметры и лямбда-функция.
        {10, 170, 300, 20, " ", 1, []() { SelectedNetwork = DisplayedScreen*5+4; WIFI_Choose.Draw(); Actual_Menu = &WIFI_Choose; }} //Пример кнопок. Есть параметры и лямбда-функция.
    };

  Button* getButtons() override { return buttons; } // 2^16 способов отстрелить себе конечность
  void MenuLoop() override {
    if (ReScan) {
      Thing.DoLog("Go Scanning WiFi");
      NetworksCount = WiFi.scanNetworks();
      Thing.DoLog("Scanned. Found " + String(NetworksCount));
      if (NetworksCount != 0) {
        for (int i = 0; i < NetworksCount; ++i) {
            Thing.DoLog("Remembering. Iteraion " + String(i));
            WiFiNetworks[i].SSID = WiFi.SSID(i).c_str();
            WiFiNetworks[i].RSSI = WiFi.RSSI(i);
            WiFiNetworks[i].Channel = WiFi.channel(i);
            WiFiNetworks[i].FillEncryptionType(WiFi.encryptionType(i));
        }
      }
      Thing.DoLog("Deleting");
      WiFi.scanDelete();
      ReScan = false;
      DrawButtons = true;
    }
    if (DrawButtons) {
        for (int i = 0; i < 5; i++) {
          buttons[i+4].Label = WiFiNetworks[DisplayedScreen*5 + i].SSID + " " 
            + WiFiNetworks[DisplayedScreen*5 + i].RSSI + " " 
            + WiFiNetworks[DisplayedScreen*5 + i].Channel + " " 
            + WiFiNetworks[DisplayedScreen*5 + i].StringEncryptionType;
        }
        DrawButtons = false;
        Draw();
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
  int getButtonsLength() override { return 9; } // 
};
WIFI_Menu_Type WIFI_Menu;