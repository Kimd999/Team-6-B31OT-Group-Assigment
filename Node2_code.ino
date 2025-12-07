#include <WiFi.h>
#include <DHT.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Arduino.h> 
#include <Adafruit_NeoPixel.h>

#define FIXED_WIFI_CHANNEL 6 

// MAC ADDRESS DEFINITIONS given here are for code reference purpose only; not our real ones.
uint8_t GATEWAY_MAC_ADDR[6] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x01};
uint8_t NODE1_MAC_ADDR[6]   = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x02};
uint8_t NODE2_MAC_ADDR[6]   = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x03};

// SENSOR PACKET STRUCT
typedef struct {
  char nodeID[10];
  float temp;
  float smoke;
  int alert;
  unsigned long ts;
} SensorPacket;

// Node 2 Settings
#define NODE_NAME "node2"
#define TEMP_THRESHOLD_ALERT 30.0
#define SLEEP_TIME_S 15

// DHT11 Sensor
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// NeoPixel
#define LED_PIN 5
#define NUMPIXELS 1
Adafruit_NeoPixel led(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Data Packets
SensorPacket myData;
SensorPacket neighborData;

esp_now_peer_info_t peerInfo;

unsigned long lastAlertSent = 0;
unsigned long lastNeighborAlert = 0;

// ======================================================
// NEW ESP-IDF v5.x CALLBACK (REQUIRED FOR YOUR COMPILER)
// ======================================================
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {

  if (len != sizeof(SensorPacket)) {
    Serial.println("[ERR] Packet size mismatch");
    return;
  }

  memcpy(&neighborData, data, sizeof(neighborData));

  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          info->src_addr[0], info->src_addr[1], info->src_addr[2],
          info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  Serial.printf("📥 [Node2] Received from %s (%s) | alert=%d\n",
                neighborData.nodeID, macStr, neighborData.alert);

  if (neighborData.alert == 1) {
    lastNeighborAlert = millis();
  }
}

// Add Peer
void addPeer(const uint8_t *peer_addr) {
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, peer_addr, 6);
  peerInfo.channel = FIXED_WIFI_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK)
      Serial.println("✔ Peer added");
  else
      Serial.println("❌ Peer add failed");
}

// ESP-NOW Setup + Channel Lock
void initESPNow() {
  WiFi.mode(WIFI_STA);

  WiFi.softAP("SYNC", NULL, FIXED_WIFI_CHANNEL, 0);
  WiFi.softAPdisconnect(true);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW init failed");
    ESP.restart();
  }

  Serial.println("✔ ESP-NOW ready");

  esp_now_register_recv_cb(onDataRecv);

  addPeer(GATEWAY_MAC_ADDR);
  addPeer(NODE1_MAC_ADDR); // Peer is Node1
}

// LED FUNCTIONS
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}

void setBlinkingOrange() {
  if ((millis() % 600) < 300)
    setColor(255, 100, 0);
  else
    setColor(0, 0, 0);
}

// SENSOR + ALERT LOGIC
void sendData() {

  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("⚠ DHT11 read error");
    return;
  }

  bool localAlertFlag = (t > TEMP_THRESHOLD_ALERT);
  bool cooperativeAlertFlag = false;

  if (millis() - lastNeighborAlert < SLEEP_TIME_S * 2000UL) {
    cooperativeAlertFlag = true;
  }

  if (localAlertFlag) {
    setColor(255, 0, 0);
    Serial.println("🔥 [Node2] LOCAL ALERT!");
  }
  else if (cooperativeAlertFlag) {
    setBlinkingOrange();
    Serial.println("⚠️ [Node2] COOPERATIVE ALERT (Node1)");
  }
  else {
    setColor(0, 255, 0);
    Serial.println("🌿 [Node2] OK");
  }

  strcpy(myData.nodeID, NODE_NAME);
  myData.temp = t;
  myData.smoke = 0;
  myData.alert = localAlertFlag ? 1 : 0;
  myData.ts = millis();

  esp_now_send(GATEWAY_MAC_ADDR, (uint8_t*)&myData, sizeof(myData));
  Serial.println("📤 Sent to Gateway");

  if (localAlertFlag && millis() - lastAlertSent > 5000) {
    esp_now_send(NODE1_MAC_ADDR, (uint8_t*)&myData, sizeof(myData));
    Serial.println("🚨 Alert sent to Node1");
    lastAlertSent = millis();
  }
}

// MAIN
void setup() {
  Serial.begin(115200);
  dht.begin();
  led.begin();
  led.show();

  initESPNow(); 
}

void loop() {
  sendData();

  Serial.printf("🌙 [Node2] Sleeping %d sec...\n", SLEEP_TIME_S);
  setColor(0, 0, 0);

  esp_sleep_enable_timer_wakeup(SLEEP_TIME_S * 1000000ULL);
  esp_deep_sleep_start();
}
