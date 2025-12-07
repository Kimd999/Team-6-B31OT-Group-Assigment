#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <ArduinoJson.h> 
 
// =============================================================
// MAC ADDRESS DEFINITIONS given here are for code reference purpose only; not our real ones.
// =============================================================
uint8_t GATEWAY_MAC_ADDR[6] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x01};
uint8_t NODE1_MAC_ADDR[6]   = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x02};
uint8_t NODE2_MAC_ADDR[6]   = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x03};         

// =============================================================
// WiFi CREDENTIALS
// =============================================================
const char* ssid     = "Test";
const char* password = "123456789"; 

// =============================================================
// HiveMQ CLOUD TLS SETTINGS
// =============================================================
const char* mqttServer = "2bcacdc6c78e4b73a575478294c5953b.s1.eu.hivemq.cloud";
const int   mqttPort   = 8883;
const char* mqttUser   = "Ananthapadmanabhan_Manoj"; 
const char* mqttPass   = "Padmanabham@23";

#define ALERT_TOPIC   "greenhouse/alerts"
#define STATUS_TOPIC  "greenhouse/gateway/status"

// =============================================================
// MQTT OBJECTS
// =============================================================
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// =============================================================
// DATA STRUCT (MUST match Node1 & Node2)
// =============================================================
typedef struct {
  char nodeID[10];
  float temp;
  float smoke;
  int alert;
  unsigned long ts;    
} SensorPacket;

SensorPacket incoming;

// =============================================================
// PRINT GATEWAY MAC ADDRESS
// =============================================================
void printMAC() {
    Serial.print("Gateway MAC: ");
    Serial.println(WiFi.macAddress());
}

// =============================================================
// MQTT RECONNECT
// =============================================================
void mqttReconnect() {
  while (!mqttClient.connected()) {
      Serial.print("Connecting to MQTT... ");

      String clientId = "ESP32-Gateway-";
      clientId += WiFi.macAddress();
      clientId.replace(":", "");      

      if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPass)) {   
          Serial.println("Connected to HiveMQ Cloud.");
          mqttClient.publish(STATUS_TOPIC, "Gateway online");
      } else {
          Serial.print("Failed (rc=");
          Serial.print(mqttClient.state());
          Serial.println("). Retrying..."); 
          delay(2000);
      }
  }
}

// =============================================================
// ESP-NOW CALLBACK FOR ESP32-C3
// =============================================================
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {

    if (len != sizeof(SensorPacket)) {
        Serial.println("[ERR] Packet size mismatch");
        return;
    }

    memcpy(&incoming, data, sizeof(incoming));

    // Print sender MAC
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            info->src_addr[0], info->src_addr[1], info->src_addr[2],
            info->src_addr[3], info->src_addr[4], info->src_addr[5]);

    Serial.printf("\n[ESP-NOW] From %s (%s)", incoming.nodeID, macStr);
    Serial.printf(" | Temp=%.2f | Smoke=%.2f | Alert=%d | TS=%lu\n",
                  incoming.temp, incoming.smoke, incoming.alert, incoming.ts);

    // ------- Dynamic MQTT Topic -------
    char topic[60];
    sprintf(topic, "greenhouse/%s/data", incoming.nodeID);

    StaticJsonDocument<200> doc;
    doc["node"]  = incoming.nodeID;                                                             
    doc["temp"]  = incoming.temp;
    doc["smoke"] = incoming.smoke;
    doc["alert"] = incoming.alert;
    doc["ts"]    = incoming.ts;

    char jsonOut[200];
    serializeJson(doc, jsonOut);
    mqttClient.publish(topic, jsonOut);

    Serial.printf("[MQTT] Published to %s\n", topic);

    // ------- GLOBAL ALERT FORWARDING -------
    if (incoming.alert == 1) {
        StaticJsonDocument<150> alert;
        alert["source"] = incoming.nodeID;
        alert["status"] = "ALERT";
        alert["ts"]     = incoming.ts;

        char outAlert[150];
        serializeJson(alert, outAlert);

        mqttClient.publish(ALERT_TOPIC, outAlert);
        Serial.println("[MQTT] GLOBAL ALERT forwarded!");
    }
}

// =============================================================
// SETUP
// =============================================================
void setup() {
    Serial.begin(115200);
    delay(300);

    Serial.println("\n==== GATEWAY STARTING ====");

    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(400);
    }
    Serial.println("\nWiFi connected!");

    printMAC();

    // Sync ESP-NOW channel with router
    int channel = WiFi.channel();
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    Serial.printf("ESP-NOW synced to Channel %d\n", channel);

    // Setup MQTT TLS
    wifiClient.setInsecure();
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setKeepAlive(60);
    mqttClient.setBufferSize(512);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW INIT FAILED! Restarting...");
        ESP.restart();
    }

    esp_now_register_recv_cb(onDataRecv);

    Serial.println("ESP-NOW ready.");
}

// =============================================================
// LOOP
// =============================================================
void loop() {
    if (!mqttClient.connected()) {
        mqttReconnect();
    }
    mqttClient.loop();
}
