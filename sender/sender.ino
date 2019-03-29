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

xOD01 screen;
xSW02 tempSensor;
xSL06 lightSensor;
xSH01 touchSensor;

WiFiClientSecure *netClient;
MQTTClient *mqttClient;
CloudIoTCoreDevice *device;
unsigned long iss = 0;
String jwt;

void setup() {
    Wire.begin();
    screen.begin();
    tempSensor.begin();
    lightSensor.begin();
    lightSensor.enableLightSensor(false);
    touchSensor.begin();
    
    delay(5000);
    screen.println("Sensors ready!");

    initWifi();
    syncTime();
}

void initWifi() {
    screen.println("Connecting to:");
    screen.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        screen.println("Retry in 5 secs");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        delay(5000);
    }
    screen.println("Connected to WiFi");
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
    screen.println("MQTT started");
}

void loop() {
    uint16_t light = readLight();
    float temp = readTemperature();

    if (!mqttClient->connected()) {
        if (WiFi.status() != WL_CONNECTED) {
            initWifi();
        }
        mqttConnect();
    }

    String message1 = "{\"light\": " + String(light) + "}";
    mqttClient->publish(device->getEventsTopic(), message1);
    delay(5000);
    String message2 = "{\"temp\": " + String(temp) + "}";
    mqttClient->publish(device->getEventsTopic(), message2);
    screen.println("Messages sent");

    touchSensor.poll();
    if (touchSensor.touchDetected()) {
        if (touchSensor.circleTouched()) {
            //print ambient light
            screen.print("Light: ");
            screen.print(light);
            screen.println(" LUX");
        } else if (touchSensor.squareTouched()) {
            //print temp
            screen.print("Temperature: ");
            screen.print(temp);
            screen.println(" C");
        }
    }
    delay(5000);
}

uint16_t readLight() {
    uint16_t ambientLight = 0;
    lightSensor.getAmbientLight(ambientLight);
    return ambientLight;
}

float readTemperature() {
    tempSensor.poll();
    return tempSensor.getTempC();
}

void mqttConnect() {
    while (!mqttClient->connect(device->getClientId().c_str(),
            "unused", getJwt().c_str(), false)) {
        delay(1000);
    }
    screen.println("Connected to MQTT");
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