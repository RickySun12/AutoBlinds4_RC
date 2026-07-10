#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xD4, 0xE9, 0xF4, 0xC4, 0x41, 0xF8};
enum Opperation {IDLE,OPEN, CLOSE, CLOSE45, OPEN90};
struct Packet {
  uint8_t blindsID;
  Opperation command1;
  Opperation command2;
  Opperation command3;
  Opperation command4;
};

Packet packet;
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Ready. Enter: blindsID cmd1 cmd2 cmd3 cmd4");
  Serial.println("e.g. '1 0 2 1 3' then press Enter");
}

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) return;

    // Parse 5 space-separated integers: blindsID cmd1 cmd2 cmd3 cmd4
    int values[5];
    int idx = 0;
    int start = 0;

    for (int i = 0; i <= line.length() && idx < 5; i++) {
      if (i == line.length() || line.charAt(i) == ' ') {
        if (i > start) {
          values[idx++] = line.substring(start, i).toInt();
        }
        start = i + 1;
      }
    }

    if (idx != 5) {
      Serial.println("Invalid input. Expected 5 values: blindsID cmd1 cmd2 cmd3 cmd4");
      return;
    }

    packet.blindsID = (uint8_t)values[0];
    packet.command1 = (Opperation)values[1];
    packet.command2 = (Opperation)values[2];
    packet.command3 = (Opperation)values[3];
    packet.command4 = (Opperation)values[4];

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&packet, sizeof(packet));

    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
  }
}