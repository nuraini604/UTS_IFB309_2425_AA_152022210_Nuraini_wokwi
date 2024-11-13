#include "DHTesp.h"
#include "PubSubClient.h"
#include "WiFi.h"

#define DHTPIN 18           
#define DHTTYPE DHT22       
#define LED_GREEN 5         
#define LED_YELLOW 2        
#define LED_RED 12          
#define RELAY_PIN 17        
#define BUZZER_PIN 19       

DHTesp dhtSensor;

const char* ssid = "Wokwi-GUEST";
const char* pass = "";    
const char* mqtt_server = "broker.hivemq.com"; 

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void setup() {
  Serial.begin(115200);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setup_wifi();
  
  mqtt.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!");
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float temperature = data.temperature;
  float humidity = data.humidity;

  bool pumpStatus = false;
  if (humidity < 50) {
    digitalWrite(RELAY_PIN, HIGH); 
    pumpStatus = true;
  } else {
    digitalWrite(RELAY_PIN, LOW);  
    pumpStatus = false;
  }

  if (temperature > 35) {
    digitalWrite(LED_RED, HIGH);    
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER_PIN, HIGH); 
  } else if (temperature >= 30 && temperature <= 35) {
    digitalWrite(LED_YELLOW, HIGH); 
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER_PIN, LOW); 
  } else {
    digitalWrite(LED_GREEN, HIGH);  
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(BUZZER_PIN, LOW);  
  }

if (!isnan(temperature) && !isnan(humidity)) {
  char payload[50]; 
  snprintf(payload, sizeof(payload), "{\"temperature\":%.2f,\"humidity\":%.2f,\"pump\":\"%s\"}", temperature, humidity, pumpStatus ? "ON" : "OFF");

  mqtt.publish("UTS_IOT_Nuraini", payload);

  Serial.print("Data sent to MQTT: ");
  Serial.println(payload);
}

  
  delay(2000); 
}
