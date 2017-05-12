// CONFIGURATION

const char* ssid = "...";
const char* password = "...";
const int connectionRetries = 10;

const char* ioUser = "...";
const char* ioPassword = "...";

const char* mqttClientId = "...";
const char* mqttServer = "...";
const int mqttPort = 1883;

const char* humFeed = ".../f/hum";
const char* tempFeed = ".../f/temp";
const char* groundHum1Feed = ".../f/groundHum1";
const char* groundHum2Feed = ".../f/groundHum2";
const char* airFeed = ".../f/air";

const int sleepSeconds = 10;
const int airQualitySensorWarmUpTime = 60000;
const int airSensorWindowSkip = 6;

// END CONFIGURATION

#define DEBUG false

#ifdef DEBUG
  #define Sprintln(a) (Serial.println(a))
  #define Sprint(a) (Serial.print(a))
#else
  #define Sprintln(a)
  #define Sprint(a)
#endif
