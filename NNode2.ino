#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

// ------------ Node Settings ------------
#define NODE_ID 2
const char* nodeName = "node2";
const char* dataTopic = "greenhouse/node2/data";   // FIXED (unique for node2)

// ------------ Pins ------------
#define DHTPIN     5
#define DHTTYPE    DHT11
#define LED_PIN    4
#define NUMPIXELS  1

DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel led(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ------------ WiFi ------------
const char* ssid     = "Test";        
const char* password = "123456789";   

// ------------ HiveMQ Cloud TLS ------------
const char* mqttServer = "2bcacdc6c78e4b73a575478294c5953b.s1.eu.hivemq.cloud";
const int mqttPort     = 8883;
const char* mqttUser   = "Ananthapadmanabhan_Manoj";
const char* mqttPass   = "Padmanabham@23";

// TLS client
WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

// ------------ Helpers ------------
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  led.setPixelColor(0, led.Color(r, g, b));
  led.show();
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Node connecting to WiFi ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to HiveMQ Cloud... ");

    String clientId = "Node2_";      // FIXED: Node2 ID
    clientId += WiFi.macAddress();
    clientId.replace(":", "");

    if (mqtt.connect(clientId.c_str(), mqttUser, mqttPass)) {
      Serial.println("MQTT connected ✔");
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);     // FIXED BAUD RATE
  dht.begin();
  led.begin();
  led.clear();
  led.show();

  connectWiFi();

  secureClient.setInsecure();   // No certificate needed
  mqtt.setServer(mqttServer, mqttPort);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("⚠ Sensor error");
    delay(2000);
    return;
  }

  String tempLevel;
  if (t < 18) {
    tempLevel = "Cold";
    setColor(0, 0, 255);
  } 
  else if (t > 30) {
    tempLevel = "Hot";
    setColor(255, 0, 0);
  } 
  else {
    tempLevel = "Optimal";
    setColor(0, 255, 0);
  }

  StaticJsonDocument<256> doc;
  doc["node"]        = NODE_ID;
  doc["name"]        = nodeName;
  doc["temperature"] = t;
  doc["humidity"]    = h;
  doc["status"]      = tempLevel;

  char buffer[256];
  size_t len = serializeJson(doc, buffer);

  if (mqtt.publish(dataTopic, buffer, len)) {
    Serial.print("Published: ");
    Serial.println(buffer);
  } else {
    Serial.println("⚠ MQTT publish failed");
  }

  delay(5000);
}
