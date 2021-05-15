#include <stdio.h>
#include <EEPROM.h>
#include <Arduino.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "env.cpp"
#include "config.h"
#include "pubsub/pubsub.h"

#define OUTLET_0_PIN D2
#define OUTLET_1_PIN D3
// #define DEBUG true //if enabled this increases the latency

const int EEPROM_ADDR = 0;

WiFiManager wm;
WiFiClient espClient;
PubSubClient client(espClient);
Config config;

// Handle data definition
const size_t jsonReceiveDataCapacity = JSON_OBJECT_SIZE(8) + JSON_ARRAY_SIZE(4) + 60;

void updateStripConfig(Config *config)
{
    EEPROM.put(EEPROM_ADDR, *config);
    EEPROM.commit();
}

void handlePreCommands(DynamicJsonDocument *doc, DynamicJsonDocument *outputDoc)
{
}

void handleCommands(DynamicJsonDocument *doc, DynamicJsonDocument *outputDoc)
{
    switch ((*doc)["c"].as<int>())
    {
    case 0:
        // toggle outlet0State
        config.outlet0State = !config.outlet0State;
        break;
    case 1:
        config.outlet1State = !config.outlet1State;
        break;
    case 2:
        // shutdown
        config.outlet0State = false;
        config.outlet1State = false;
        break;
    case 3:
        // turn on
        config.outlet0State = true;
        config.outlet1State = true;
        break;
    case 4:
        if ((*doc)["o"].as<int>() == 0)
        {
            config.outlet0State = (*doc)["s"].as<bool>();
        }
        else
        {
            config.outlet1State = (*doc)["s"].as<bool>();
        }
        break;
    case 99:
        // reset config
        Config default_config;
        EEPROM.put(EEPROM_ADDR, default_config);
        EEPROM.commit();
    }
}

void handlePostCommands(DynamicJsonDocument *doc, DynamicJsonDocument *outputDoc)
{
    digitalWrite(OUTLET_0_PIN, config.outlet0State ? HIGH : LOW);
    digitalWrite(OUTLET_0_PIN, config.outlet1State ? HIGH : LOW);
    (*outputDoc)["s0"] = config.outlet0State;
    (*outputDoc)["s1"] = config.outlet1State;
    (*outputDoc)["id"] = config.topicName;
}

void callback(char *topic, byte *payload, unsigned int length)
{

    // LOG("Message arrived in topic: " + (String)topic);
    DynamicJsonDocument doc(jsonReceiveDataCapacity);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        LOG(error.c_str());
    }
    else
    {
        DynamicJsonDocument outputDoc(JSON_OBJECT_SIZE(8));
        String output;

        // LOG("################");
        // serializeJson(doc, Serial);
        // LOG("\n----------------");

        // process request
        handlePreCommands(&doc, &outputDoc);
        handleCommands(&doc, &outputDoc);
        handlePostCommands(&doc, &outputDoc);

        // write response
        serializeJson(outputDoc, output);
        LOG(output);
        publish(&client, &config, const_cast<char *>(output.c_str()));
    }
}

void connectToBroker()
{
    if (!client.connected())
    {
        while (!client.connected())
        {
            LOG("Connecting to MQTT...");
            //if (client.connect(String("esp8266-client-" + (String)config.topicName).c_str()))
            String clientId = "ESP8266Client-";
            clientId += String(random(0xffff), HEX);
            if (client.connect(clientId.c_str()))
            {
                subscribe(&client, &config);
                String output;
                DynamicJsonDocument doc(jsonReceiveDataCapacity);
                doc["id"] = config.topicName;
                serializeJson(doc, output);
                publish(&client, &config, const_cast<char *>(output.c_str()));
                LOG("connected");
            }
            else
            {

                LOG("failed with state ");
                LOG(client.state());
                delay(1000);
            }
        }
    }
}

void saveWifiCallback()
{
    LOG("[CALLBACK] saveCallback fired");
    config.enablePortal = false;
    EEPROM.put(EEPROM_ADDR, config);
    EEPROM.commit();
}

void wifiInfo(WiFiManager *wm)
{
    WiFi.printDiag(Serial);
    LOG("SAVED: " + (String)wm->getWiFiIsSaved() ? "YES" : "NO");
    LOG("SSID: " + (String)wm->getWiFiSSID());
    LOG("PASS: " + (String)wm->getWiFiPass());
}

bool configWiFi(Config *config, WiFiManager *wm)
{
    WiFi.mode(WIFI_STA);                // explicitly set mode, esp defaults to STA+AP
    WiFi.setSleepMode(WIFI_NONE_SLEEP); // disable sleep, can improve ap stability
    WiFi.hostname(String("esp8266-" + (String)config->topicName + ".local").c_str());
    wm->setHostname(String("esp8266-" + (String)config->topicName + ".local").c_str());
    wm->setClass("invert");
    wm->setCountry("US");
    wm->setConfigPortalTimeout(120);
    wm->setConnectTimeout(60);
    wm->setBreakAfterConfig(true);
    wm->setSaveConfigCallback(saveWifiCallback);
    wm->setCleanConnect(true);
    return !wm->autoConnect() || config->enablePortal;
}

void configOTA(Config *config)
{
    LOG("Configuring OTA");
    ArduinoOTA.setHostname(String("esp8266-" + (String)config->topicName).c_str());
    ArduinoOTA.setPassword(config->otaPassword);

    ArduinoOTA.onStart([]()
                       { LOG("Start"); });
    ArduinoOTA.onEnd([]()
                     { LOG("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
                           Serial.printf("Error[%u]: ", error);
                           if (error == OTA_AUTH_ERROR)
                               LOG("Auth Failed");
                           else if (error == OTA_BEGIN_ERROR)
                               LOG("Begin Failed");
                           else if (error == OTA_CONNECT_ERROR)
                               LOG("Connect Failed");
                           else if (error == OTA_RECEIVE_ERROR)
                               LOG("Receive Failed");
                           else if (error == OTA_END_ERROR)
                               LOG("End Failed");
                       });
    ArduinoOTA.begin();
    LOG("OTA ready");
}

void configClient(PubSubClient *client, Config *config)
{
    LOG("Configuring client");
    client->setServer(config->host, config->port);
    client->setCallback(callback);
}

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(4096);

    pinMode(OUTLET_0_PIN, OUTPUT);
    pinMode(OUTLET_1_PIN, OUTPUT);

    delay(500);
    if (DEBUG)
        logConfig(&config);

    bool shouldEnablePortal = configWiFi(&config, &wm);
    if (shouldEnablePortal)
    {
        LOG("CONFIG ENABLED");
        wm.setConfigPortalTimeout(180);
        wm.startConfigPortal(String("WM_ConnectAP_" + (String)config.topicName).c_str());
    }
    else
    {
        configClient(&client, &config);
        delay(500);
        configOTA(&config);
        delay(250);
        connectToBroker();
        delay(250);
    }
    if (DEBUG)
        wifiInfo(&wm);
    delay(500);
}

void loop()
{
    ArduinoOTA.handle();
    client.loop();
    connectToBroker();
}