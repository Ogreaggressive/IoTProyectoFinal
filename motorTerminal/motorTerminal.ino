#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//MotorPins
const int SPEEDPIN=16;
const int RIGHTDIRPIN=27;
const int LEFTDIRPIN =14;

//curtain Pins
const int curtainSensorPin=18;

const String openCurtainState="CURTAIN_OPENED";
const String closeCurtainState="CURTAIN_CLOSED";

String currentCurtainState=closeCurtainState;


//Wifi Config
const char* WIFI_SSID = "POCO X4 Pro 5G";
const char* WIFI_PASS = "testwemos";
//AWS BrokerConfig
const char* AWS_ENDPOINT = "a13f6domxjrswp-ats.iot.us-east-1.amazonaws.com";
const int AWS_ENDPOINT_PORT = 8883;

//Topics
const char* MQTT_CLIENT_ID = "test_ucb_cosin";

bool startedDevice=true;
long timeToOpenRoller=0;



const int PubGetIndex=1;
const int getShadowInfoIndex=0;

const char* PUBLISH_TOPICS_ARRAY[]={
  "$aws/things/cosin/shadow/get",
  "$aws/things/cosin/shadow/update",
  "$aws/things/cosin/shadow/delete"};

const int desiredIndex=8;
const int getAcceptedIndex=9;
const char* SUBSCRIBE_TOPICS_ARRAY[]={
  "$aws/things/cosin/shadow/delete/accepted",
  "$aws/things/cosin/shadow/delete/rejected",
  "$aws/things/cosin/shadow/get/accepted",
  "$aws/things/cosin/shadow/get/rejected",
  "$aws/things/cosin/shadow/update/accepted",
  "$aws/things/cosin/shadow/update/rejected",
  "$aws/things/cosin/shadow/update/delta",
  "$aws/things/cosin/shadow/update/document",
  "/roller/republish",
  "/roller/republish/get"};


const char AMAZON_ROOT_CA1[] PROGMEM = "----";

const char CERTIFICATE[] PROGMEM = "----";


const char PRIVATE_KEY[] PROGMEM = "----";


void stopMotor()
{
  digitalWrite (RIGHTDIRPIN,LOW ) ;
  digitalWrite (LEFTDIRPIN, LOW);
}
void moveMotorLeft()
{
  digitalWrite (RIGHTDIRPIN,LOW ) ;
  digitalWrite (LEFTDIRPIN, HIGH);
}
void moveMotorRight()
{
  digitalWrite (LEFTDIRPIN, LOW);
  digitalWrite (RIGHTDIRPIN,HIGH ) ;
}

void closeCurtain()
{
    moveMotorLeft();
    unsigned long startTime = millis();
    while(digitalRead(curtainSensorPin)!=0){}
    unsigned long endTime = millis();
    stopMotor();
    timeToOpenRoller=endTime-startTime;
    currentCurtainState=closeCurtainState;
    sendJSONWithCurrentState(true,true);

}

void openCurtain()
{
    moveMotorRight();
    unsigned long startTime = millis();
    while(timeToOpenRoller>millis()-startTime){}
    stopMotor();
    
    currentCurtainState=openCurtainState;
    sendJSONWithCurrentState(true,false);
}


WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
DynamicJsonDocument inputDoc(1024);
// PubSubClient callback function
void changeCurtainState(String desiredState)
{
    if(desiredState!=currentCurtainState)
    {
        if(desiredState==openCurtainState)
        {
          Serial.println("abriendo");
            openCurtain();
        }
        else if(desiredState==closeCurtainState)
        {
          Serial.println("cerrando");
            closeCurtain();
        }
    }
}
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) 
  {
    message += String((char) payload[i]);
  }
  Serial.print("llego un mensaje:");
  Serial.println(String(topic));
  
  if (String(topic) == SUBSCRIBE_TOPICS_ARRAY[desiredIndex]) 
  {
    Serial.println("Message from topic " + String(topic) + " : " + message);
    DeserializationError err = deserializeJson(inputDoc, payload);  
    if(!err)
    {
        String desiredState = String(inputDoc["curtainState"].as<char*>());
        changeCurtainState(desiredState);
    }
  }
  if (String(topic) == SUBSCRIBE_TOPICS_ARRAY[getAcceptedIndex]) 
  {
    Serial.println("devolvio get");
    Serial.println("Message from topic " + String(topic) + " : " + message);
    DeserializationError err = deserializeJson(inputDoc, payload);  
    if(!err)
    {
        timeToOpenRoller =float(inputDoc["timeToOpenRoller"]);
        
        currentCurtainState=String(inputDoc["curtainState"].as<char*>());
        String desiredState = String(inputDoc["curtainStateDesired"].as<char*>());
        Serial.println(timeToOpenRoller);
        Serial.println(currentCurtainState);
        Serial.println(desiredState);
        changeCurtainState(desiredState);
    }
  }
}




void SubscribeToTopics()
{
  for(int i=0;i<10;i++)
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
    
    Serial.begin(115200);
    //MotorPinsSetup
    pinMode (SPEEDPIN, OUTPUT);
    pinMode (LEFTDIRPIN, OUTPUT);
    pinMode (RIGHTDIRPIN, OUTPUT);
    digitalWrite (SPEEDPIN,HIGH) ;
    stopMotor();
    //PhotoElectricSensorSetup
    pinMode (curtainSensorPin, INPUT);

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



StaticJsonDocument<JSON_OBJECT_SIZE(4)> outputDoc;
char outputBuffer[128];
void sendJSONWithCurrentState(bool sendCurtainState,bool sendTimeToOpenRoller)
{
    outputDoc["state"]["reported"]["curtainState"] = currentCurtainState.c_str();
    Serial.println("mandar tiempo:"+sendTimeToOpenRoller );
    if(sendTimeToOpenRoller)
    {
        outputDoc["state"]["reported"]["timeToOpenRoller"] = timeToOpenRoller;
    }
    serializeJson(outputDoc, outputBuffer);
    Serial.println("Intenta enviar un estado");
    Serial.println(outputBuffer);
    mqttClient.publish(PUBLISH_TOPICS_ARRAY[PubGetIndex], outputBuffer);
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
void loop() {
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
    mqttClient.loop();
    delay(20);
    if(startedDevice)
    {
        sendJSONToAskForShadowInfo();
        startedDevice=false;
      
    }
  }
}
