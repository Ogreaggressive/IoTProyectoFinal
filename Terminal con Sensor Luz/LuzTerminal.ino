#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

int LIGHTRESISTANCE = 35;


const int THRESHOLD=2500;
const String openCurtainState="CURTAIN_OPENED";
const String closeCurtainState="CURTAIN_CLOSED";

String lastCurtainStateSent=openCurtainState;

//Wifi Config
const char* WIFI_SSID = "POCO X4 Pro 5G";
const char* WIFI_PASS = "testwemos";
//AWS BrokerConfig
const char* AWS_ENDPOINT = "a13f6domxjrswp-ats.iot.us-east-1.amazonaws.com";
const int AWS_ENDPOINT_PORT = 8883;

//Topics
const char* MQTT_CLIENT_ID = "terminalLight";
bool detectLight=true;
bool startedDevice=true;
bool Sleep=false;
const int PubGetIndex=1;
const int getShadowInfoIndex=0;
unsigned long previusMillis;
unsigned long sleepPreviusMillis;
const char* PUBLISH_TOPICS_ARRAY[]={
  "$aws/things/cosin/shadow/get",
  "$aws/things/cosin/shadow/update",
  "$aws/things/cosin/shadow/delete"};

const int lightTopicIndex=0;
const char* SUBSCRIBE_TOPICS_ARRAY[]={
  "/roller/light"};


const char AMAZON_ROOT_CA1[] PROGMEM = "----";

const char CERTIFICATE[] PROGMEM = "----";


const char PRIVATE_KEY[] PROGMEM = "----";


String getCurtainStatusBasedOnLight()
{
  int lightInput = analogRead(LIGHTRESISTANCE);
  Serial.print("Detected Light:");
  Serial.println(lightInput);
  if (lightInput>THRESHOLD) //esta oscuro
  {
    return closeCurtainState;
  }
  else //esta claro
  {
    return openCurtainState;
  }
}


WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
DynamicJsonDocument inputDoc(1024);
// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) 
  {
    message += String((char) payload[i]);
  }
  if (String(topic) == SUBSCRIBE_TOPICS_ARRAY[lightTopicIndex]) 
  {
    Serial.println("Message from topic " + String(topic) + " : " + message);
    DeserializationError err = deserializeJson(inputDoc, payload);  
    if(!err)
    {
        detectLight =inputDoc["detectLight"];
        sendJSONWithAutomaticReported(detectLight);
        Serial.println("Automatic mode:"+String(detectLight));
        
    }
  }
}




void SubscribeToTopics()
{
  for(int i=0;i<1;i++)
  {
    mqttClient.subscribe(SUBSCRIBE_TOPICS_ARRAY[i]);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPICS_ARRAY[i]));
  }
}
boolean mqttClientConnect() {
  Serial.println("Connecting to " + String(AWS_ENDPOINT));
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("Connected to " + String(AWS_ENDPOINT));
    SubscribeToTopics();
  } else {
    Serial.println("Can't connect to " + String(AWS_ENDPOINT));
  }
  return mqttClient.connected();
}

void setup() {
    pinMode (LIGHTRESISTANCE, INPUT);
    Serial.begin(115200);
    Serial.println("Connecting to " + String(WIFI_SSID));
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) 
    {
        Serial.println("Can't connect to " + String(WIFI_SSID));
        while (1) delay(200);
    }
    Serial.println("Connected to " + String(WIFI_SSID));

    wifiClient.setCACert(AMAZON_ROOT_CA1);
    wifiClient.setCertificate(CERTIFICATE);
    wifiClient.setPrivateKey(PRIVATE_KEY);
    mqttClient.setServer(AWS_ENDPOINT, AWS_ENDPOINT_PORT);
    mqttClient.setCallback(callback);
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

unsigned char counter = 0;



StaticJsonDocument<JSON_OBJECT_SIZE(3)> outputDocState;
char outputBufferState[128];
void sendJSONWithCurrentState(String curtainState)
{
    outputDocState["state"]["desired"]["curtainState"] = curtainState.c_str();
    
    serializeJson(outputDocState, outputBufferState);
    Serial.println(outputBufferState);
    mqttClient.publish(PUBLISH_TOPICS_ARRAY[PubGetIndex], outputBufferState);
}
StaticJsonDocument<JSON_OBJECT_SIZE(3)> outputDocMode;
char outputBufferMode[128];
void sendJSONWithAutomaticReported(bool state)
{
    outputDocMode["state"]["reported"]["detectLight"] = state;

    serializeJson(outputDocMode, outputBufferMode);
    mqttClient.publish(PUBLISH_TOPICS_ARRAY[PubGetIndex], outputBufferMode);
}

StaticJsonDocument<JSON_OBJECT_SIZE(1)> outputDocGetState;
char outputBufferGetState[128];
void sendJSONToAskForShadowInfo()
{
    outputDocGetState["get"] = 1;
    serializeJson(outputDocGetState, outputBufferGetState);
    Serial.println(outputBufferGetState);
    mqttClient.publish(PUBLISH_TOPICS_ARRAY[getShadowInfoIndex], outputBufferGetState);
}

void loop() 
{
  unsigned long now = millis();
  if (!mqttClient.connected()) 
  {
    if (now - previousConnectMillis >= 2000) 
    {
      previousConnectMillis = now;
      if (mqttClientConnect()) previousConnectMillis = 0;
      else delay(1000);
    }
  } 
  else 
  { 
    if(startedDevice)
    {
      sendJSONToAskForShadowInfo();
      startedDevice=false;
      previusMillis=now;
    }
    delay(20);
    mqttClient.loop();
    //300000
    if (Sleep and now - sleepPreviusMillis >=10000 )
    {
        Sleep=false;
        previusMillis=now;
    }
    if(detectLight and Sleep==false and now - previusMillis >= 1000)
    {
      String desiredStatus=getCurtainStatusBasedOnLight();
      if(desiredStatus!=lastCurtainStateSent)
      {
        Serial.println("nuevo status");
        sendJSONWithCurrentState(desiredStatus);
        lastCurtainStateSent=desiredStatus;
        Sleep=true;
        sleepPreviusMillis=now;
        
      }
      previusMillis=now;
    }
    
  }
}
    
