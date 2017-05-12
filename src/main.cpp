#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>
#include <EEPROM.h>
#include <Config.h>

// PINS
const int dht11Pin = D4;
const int airSwitchPin = D5;
const int sensorsSwitchPin = D8;
const int multiplexerOne = D1;
const int multiplexerTwo = D2;
const int multiplexerTree = D3;

const int analogPinAir = 0;
const int analogPinGround1 = 1;
const int analogPinGround2 = 1;

// END PINS

// DATA
byte temperature = 0;
byte humidity = 0;

WiFiClient espClient;
PubSubClient client(espClient);
SimpleDHT11 dht11;

void sleep(){
        Sprint("going to sleep. Seconds: ");
        Sprintln(sleepSeconds);

        ESP.deepSleep(sleepSeconds * 1000000);
}

void mqttConnect(){
        client.setServer(mqttServer, mqttPort);

        int retries = 0;
        while (!client.connected()) {
                if (retries == connectionRetries) {
                        Sprintln("");
                        Sprintln("max retries reached connecting to mqtt server");
                        sleep();
                }

                Sprintln("attempting mqtt connection...");

                if (!client.connect(mqttClientId,ioUser,ioPassword)) {
                        Sprintln("error connecting to mqtt server.");
                        delay(5000);
                        retries++;
                }
        }

        client.subscribe(keepOnFeed);

        Sprintln("connected to mqtt server.");
}

void connect() {
        Sprint("connecting to ssid ");
        Sprintln(ssid);
        WiFi.begin(ssid, password);
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED) {
                if (retries == connectionRetries) {
                        Sprintln("");
                        Sprintln("error connecting to wifi");
                        sleep();
                }

                delay(500);
                Sprint(".");
                retries++;
        }

        Sprintln("");
        Sprintln("WiFi connected");
        Sprintln("IP address: ");
        Sprintln(WiFi.localIP());
}

boolean readDht11Sensor(){
        return !dht11.read(dht11Pin, &temperature, &humidity, NULL);
}

boolean publishData(const char* feed, int data) {
        char b[3];
        sprintf(b,"%d",data);

        return client.publish(feed,b);
}

boolean powerOnSensors(){
        pinMode(airSwitchPin,OUTPUT);
        pinMode(sensorsSwitchPin,OUTPUT);

        digitalWrite(sensorsSwitchPin,HIGH);

        int airSensorCount =  EEPROM.read(0);
        boolean airSensorOn;
        if (airSensorCount >= airSensorWindowSkip ) {
                Sprintln("powering on air quality sensor and warming up");
                digitalWrite(airSwitchPin,HIGH);
                delay(airQualitySensorWarmUpTime);
                EEPROM.write(0, 0);
                Sprintln("air quality sensor on");
                airSensorOn = true;
        } else {
                Sprint("air quality sensor off. Counter: ");
                Sprintln(airSensorCount);
                EEPROM.write(0, airSensorCount +1);
                airSensorOn = false;
        }

        EEPROM.commit();

        return airSensorOn;
}

void setupAnalogMultiplexer() {
        pinMode(multiplexerOne, OUTPUT);
        pinMode(multiplexerTwo, OUTPUT);
        pinMode(multiplexerTree, OUTPUT);
}

int readAnalog(int pin) {
        int r0 = bitRead(pin,0);
        int r1 = bitRead(pin,1);
        int r2 = bitRead(pin,2);

        digitalWrite(multiplexerOne, r0);
        digitalWrite(multiplexerTwo, r1);
        digitalWrite(multiplexerTree, r2);

        return analogRead(A0);
}

boolean airSensorOn;
void setup() {
        EEPROM.begin(2);

        if (DEBUG) {
                Serial.begin(9600);
        }

        airSensorOn = powerOnSensors();
        connect();
        mqttConnect();
        setupAnalogMultiplexer();
}

void loop() {
        if (readDht11Sensor()) {
                Sprint("dht data ");
                Sprint((int)temperature);
                Sprint(" ");
                Sprintln((int)humidity);

                publishData(tempFeed,(int)temperature);
                publishData(humFeed,(int)humidity);
        } else {
                Sprintln("error reading data from dht sensor");
        }

        if (airSensorOn) {
                int a = readAnalog(analogPinAir);
                Sprint("reading air quality sensor: ");
                Sprintln(a);
                publishData(airFeed,a);
        }

        int s1 = readAnalog(analogPinGround1);
        int s2 = readAnalog(analogPinGround2);

        Sprint("reading ground humidity sensor data: ");
        Sprint(s1);
        Sprint(", ");
        Sprintln(s2);

        publishData(groundHum1Feed,s1);
        publishData(groundHum2Feed,s2);

        sleep();
}
