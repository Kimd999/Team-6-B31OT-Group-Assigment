#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

// -------- LED -----------
#define LED_PIN 5
#define NUMPIXELS 1
Adafruit_NeoPixel led(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// -------- WiFi ----------
const char* ssid     = "Test";
const char* password = "123456789";

// -------- HiveMQ Cloud TLS ----------
const char* mqttServer = "2bcacdc6c78e4b73a575478294c5953b.s1.eu.hivemq.cloud";
const int mqttPort     = 8883;
const char* mqttUser   = "Ananthapadmanabhan_Manoj";
const char* mqttPass   = "Padmanabham@23";

// -------- Topics ----------
const char* dataSubTopic = "greenhouse/+/data";   // node1, node2...
const char* cmdTopic     = "greenhouse/gateway/cmd";
const char* statusTopic  = "greenhouse/gateway/status";
const char* alertTopic   = "greenhouse/alerts";   // GLOBAL alert channel

// -------- TLS Client --------
WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

// -------- LED helper --------
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}

// -------- WiFi Connect --------
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// -------- MQTT Connect --------
void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to HiveMQ Cloud... ");

    String clientId = "ESP32_Gateway_";
    clientId += WiFi.macAddress();
    clientId.replace(":", "");

    if (mqtt.connect(clientId.c_str(), mqttUser, mqttPass)) {
      Serial.println("CONNECTED ✔");

      mqtt.subscribe(dataSubTopic);   // receive all node data
      mqtt.subscribe(cmdTopic);       // LED control
      mqtt.publish(statusTopic, "gateway-online");

    } else {
      Serial.print("FAILED → state ");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

// -------- MQTT CALLBACK --------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.println("---- MQTT ----");
  Serial.print("Topic: "); Serial.println(topic);
  Serial.print("Message: "); Serial.println(msg);

  // ----- 1. Gateway LED control -----
  if (String(topic) == cmdTopic) {
    StaticJsonDocument<200> doc;
    if (!deserializeJson(doc, msg)) {
      int r = doc["red"] | 0;
      int g = doc["green"] | 0;
      int b = doc["blue"] | 0;
      setColor(r, g, b);
      Serial.println("Gateway LED updated ✔");
    }
  }

  // ----- 2. GLOBAL ALERT SYNC FOR NODES -----
  if (String(topic).startsWith("greenhouse/node")) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, msg) != DeserializationError::Ok) return;

    String status = doc["status"];

    if (status == "Hot") {
      mqtt.publish(alertTopic, "HOT");
      Serial.println("🔥 GLOBAL ALERT: HOT");
    }
    else if (status == "Cold") {
      mqtt.publish(alertTopic, "COLD");
      Serial.println("❄ GLOBAL ALERT: COLD");
    }
    else {
      mqtt.publish(alertTopic, "NORMAL");
      Serial.println("🌿 GLOBAL ALERT: NORMAL");
    }
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  led.begin();
  led.clear();
  led.show();
  setColor(0, 0, 80);  // startup glow

  connectWiFi();

  secureClient.setInsecure();   // TLS without certificate

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqttCallback);
}

// -------- LOOP --------
void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();
}
