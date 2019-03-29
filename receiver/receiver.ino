#include <xCore.h>
#include <xVersion.h>
#include "xOD01.h"
#include <xOC03.h>
#include <xSL06.h>

#include <MQTT.h>
#include <CloudIoTCore.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include <stdlib.h>
#include <stdio.h>

xOD01 screen;
xOC03 relay;
xSL06 lightSensor;

WiFiClientSecure *netClient;
MQTTClient *mqttClient;
CloudIoTCoreDevice *device;
unsigned long iss = 0;
String jwt;

void setup() {
    Wire.begin();
    screen.begin();
    relay.begin();
    lightSensor.begin();
    lightSensor.setProximityGain(PGAIN_2X);
    lightSensor.enableProximitySensor(false);

    delay(5000);
    screen.println("Sensors ready!");

    initWifi();
    syncTime();
    initCloud();
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
    mqttClient->onMessage(messageReceived);
    screen.println("MQTT started");
}

void messageReceived(String &topic, String &payload) {
    screen.println(".");
    screen.println(payload);
    int i = atoi(payload.c_str());
    if (i < 50) {
        relay.write(HIGH);
    } else if (i > 100) {
        relay.write(LOW);
    }
}

int isOn = 0;
void loop() {
    mqttClient->loop();
    delay(10);
    screen.print(".");
    
    if (!mqttClient->connected()) {
        if (WiFi.status() != WL_CONNECTED) {
            initWifi();
        }
        mqttConnect();
    }

    uint8_t proximity = 0;
    lightSensor.getProximity(proximity);
    if (proximity > 100) {
        //detected something close
        if (isOn == 0) {
            screen.println("ON!");
            relay.write(HIGH);
            isOn = 1;
        } else {
            screen.println("OFF!");
            relay.write(LOW);
            isOn = 0;
        }
    }
    delay(500);
}

void mqttConnect() {
    while (!mqttClient->connect(device->getClientId().c_str(), "unused", getJwt().c_str(), false)) {
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