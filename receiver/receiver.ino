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

xOD01 OD01;
xOC03 OC03;
xSL06 SL06;

WiFiClientSecure *netClient;
MQTTClient *mqttClient;
CloudIoTCoreDevice *device;
unsigned long iss = 0;
String jwt;

void setup() {
    Wire.begin();
    OD01.begin();
    OC03.begin();
    SL06.begin();
    SL06.setProximityGain(PGAIN_2X);
    SL06.enableProximitySensor(false);

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
    mqttClient->onMessage(messageReceived);
    OD01.println("MQTT started");
}

void messageReceived(String &topic, String &payload) {
    OD01.println(".");
    OD01.println(payload);
    int i = atoi(payload.c_str());
    if (i < 50) {
        OC03.write(HIGH);
    } else if (i > 100) {
        OC03.write(LOW);
    }
}

int isOn = 0;
void loop() {
    mqttClient->loop();
    delay(10);
    OD01.print(".");

    if (!mqttClient->connected()) {
        if (WiFi.status() != WL_CONNECTED) {
            initWifi();
        }
        mqttConnect();
    }

    uint8_t proximity = 0;
    SL06.getProximity(proximity);
    if (proximity > 100) {
        //detected something close
        if (isOn == 0) {
            OD01.println("ON!");
            OC03.write(HIGH);
            isOn = 1;
        } else {
            OD01.println("OFF!");
            OC03.write(LOW);
            isOn = 0;
        }
    }
    delay(500);
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