#include <MycilaESPConnect.h>
#include <Arduino.h>
#include <MycilaMQTT.h>
//#include <DS18B20.h>
#include <DHT22.h>

DS18B20 ds(2);
Mycila::MQTT mqtt;

String baseTopic = "DS18";

AsyncWebServer server(80);
Mycila::ESPConnect espConnect(server);
uint32_t lastLog = 0;
String hostname = "arduino-1";


float temp=0;



/*void mqtt() {
  unsigned long tempo=millis();
if(millis()-tempo>3000){
  client.subscribe("capteur_maison_on", [] (const String &payload)  {
    Serial.println(payload);

    tempo=millis();
  });

  
  
}

}*/


void DS18(){
  while (ds.selectNext()) {

    unsigned long tempo=0;
  if(millis()-tempo>3000){

    temp=ds.getTempC();
    Serial.println(ds.getTempC());

    tempo=millis();

   }
   
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

    WiFi.mode(WIFI_AP);
    WiFi.begin("Xiaomi_94F3", "0670113235!");

  while (WiFi.status() != WL_CONNECTED) {
    ESP_LOGI("APP", "Connecting to WiFi...");
    delay(1000);
  }

  
  server.on("/clear", HTTP_GET, [&](AsyncWebServerRequest* request) {
    Serial.println("Clearing configuration...");
    espConnect.clearConfiguration();
    request->send(200);
    ESP.restart();
  });

  server.on("/restart", HTTP_GET, [&](AsyncWebServerRequest* request) {
    Serial.println("Restarting...");
    request->send(200);
    ESP.restart();
  });

  // network state listener is required here in async mode
  espConnect.listen([](__unused Mycila::ESPConnect::State previous, Mycila::ESPConnect::State state) {
    JsonDocument doc;
    espConnect.toJson(doc.to<JsonObject>());
    serializeJson(doc, Serial);
    Serial.println();

    switch (state) {
      case Mycila::ESPConnect::State::NETWORK_CONNECTED:
      case Mycila::ESPConnect::State::AP_STARTED:
        // serve your home page here
        server.on("/", HTTP_GET, [&](AsyncWebServerRequest* request) {
          return request->send(200, "text/plain", "Hello World!");
        }).setFilter([](__unused AsyncWebServerRequest* request) { return espConnect.getState() != Mycila::ESPConnect::State::PORTAL_STARTED; });
        server.begin();
        break;

      case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
        server.end();
        break;

      default:
        break;
    }

  });

  espConnect.setAutoRestart(true);
  espConnect.setBlocking(false);

  Serial.println("====> Trying to connect to saved WiFi or will start portal in the background...");

  espConnect.begin(hostname.c_str(), "Captive Portal SSID");

  Serial.println("====> setup() completed...");

  

  mqtt.onConnect([]() {
    ESP_LOGI("APP", "MQTT connected");
  });

  mqtt.subscribe(baseTopic + "/value/set", [](const String& topic, const String& payload) {
    ESP_LOGI("APP", "MQTT message received: %s -> %s", topic.c_str(), payload.c_str());
  });

  Mycila::MQTT::Config config;
  config.server = "192.168.1.51";
  config.port = 1883;
  config.username = "rw";
  config.password = "readwrite";
  config.clientId = "my-app-1234";
  config.willTopic = baseTopic + "/status";

  mqtt.setAsync(true);
  mqtt.begin(config);

  dht.begin();



  //DS18();
}

void loop() {
  espConnect.loop();

   unsigned long tempo=0;
  
  mqtt.publish(baseTopic + String(temp).c_str(), String(millis()));
  delay(1000);
  

}