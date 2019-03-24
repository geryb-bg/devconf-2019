#include <xCore.h>
#include <xVersion.h>
#include "xOD01.h"
#include <xSW02.h>
#include <xSL06.h>
#include <xSH01.h>

#include <WiFi.h>
#include <MQTT.h>
#include <CloudIoTCore.h>
#include <WiFiClientSecure.h>

#include "config.h"

xOD01 OD01;
xSW02 SW02;
xSL06 SL06;
xSH01 SH01;

WiFiClientSecure *netClient;
MQTTClient *mqttClient;
CloudIoTCoreDevice *device;
unsigned long iss = 0;
String jwt;

void setup() {
  Wire.begin();
  OD01.begin();
  SW02.begin();
  SL06.begin();
  SL06.enableLightSensor(false);
  SH01.begin();

  delay(5000);
  OD01.println("Sensors ready!");

  initWifi();
  syncTime();
  initCloud();
}

void initWifi() {
  OD01.println("Connecting to:");
  OD01.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    OD01.println("Retry in 5 secs");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(5000);
  }
  OD01.println("Connected to WiFi");
}

void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 1510644967) {
    delay(10);
  }
}

void initCloud() {
  device = new CloudIoTCoreDevice(project_id, location, registry_id, device_id, private_key_str);
  netClient = new WiFiClientSecure();
  mqttClient = new MQTTClient(512);
  mqttClient->begin("mqtt.googleapis.com", 8883, *netClient);
  OD01.println("MQTT started");
}

int lightOn = 0;
int fanOn = 0;
void loop() {
  uint16_t light = readLight();
  float temp = readTemperature();

  if (!mqttClient->connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      initWifi();
    }
    mqttConnect();
  }

  if (lightOn == 0 && light < 30) {
    String message = "{\"light\": true}";
    OD01.println(message);
    mqttClient->publish(device->getEventsTopic(), message);
    lightOn = 1;
  }
  
  if (lightOn == 1 && light > 100) {
    String message = "{\"light\": false}";
    OD01.println(message);
    mqttClient->publish(device->getEventsTopic(), message);
    lightOn = 0;
  }
  
  if (fanOn == 0 && temp > 30) {
    String message = "{\"fan\": true}";
    OD01.println(message);
    mqttClient->publish(device->getEventsTopic(), message);
    fanOn = 1;
  }
  
  if (fanOn == 1 && temp < 20) {
    String message = "{\"fan\": false}";
    OD01.println(message);
    mqttClient->publish(device->getEventsTopic(), message);
    fanOn = 0;
  }

  SH01.poll();
  if (SH01.touchDetected()) {
    if (SH01.circleTouched()) {
      //print ambient light
      OD01.print("Light: ");
      OD01.print(light);
      OD01.println(" LUX");
    } else if (SH01.squareTouched()) {
      //print temp
      OD01.print("Temperature: ");
      OD01.print(temp);
      OD01.println(" C");
    }
  }
  delay(1000);
}

uint16_t readLight() {
  uint16_t ambientLight = 0;
  SL06.getAmbientLight(ambientLight);
  return ambientLight;
}

float readTemperature() {
  SW02.poll();
  return SW02.getTempC();
}

void mqttConnect() {
  while (!mqttClient->connect(device->getClientId().c_str(), "unused", getJwt().c_str(), false)) {
    delay(1000);
  }
  OD01.println("Connected to MQTT");
  mqttClient->subscribe(device->getConfigTopic());
  mqttClient->subscribe(device->getCommandsTopic());
}

String getJwt() {
  if (iss == 0 || time(nullptr) - iss > 3600) {
    iss = time(nullptr);
    jwt = device->createJWT(iss);
  }
  return jwt;
}