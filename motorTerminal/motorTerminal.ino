#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
//#define rollerID roller1
String rollerID="rollerbathroom";
//MotorPins
const int SPEEDPIN=25;
const int RIGHTDIRPIN=14;
const int LEFTDIRPIN =27;

//curtain Pins
const int curtainSensorPin=18;

const String openCurtainState="CURTAIN_OPENED";
const String closeCurtainState="CURTAIN_CLOSED";

String currentCurtainState=openCurtainState;


//AWS BrokerConfig
const char* AWS_ENDPOINT = "a13f6domxjrswp-ats.iot.us-east-1.amazonaws.com";
const int AWS_ENDPOINT_PORT = 8883;

//Topics
const char* MQTT_CLIENT_ID = "test_ucb_cosin";

bool startedDevice=true;
long timeToOpenRoller=0;



const int requestShadowTopicIndex=0;
const int reportStateTopicIndex=1;

const String PUBLISH_TOPICS_ARRAY[]={
  "$aws/things/cosin/shadow/get",
  "$aws/things/cosin/shadow/update"};

const int getShadowStateTopicIndex=0;
const int desiredGetStateTopicIndex=1;
const int listenForUpdateTopicIndex=2;

const String SUBSCRIBE_TOPICS_ARRAY[]={
  "roller/get/reported/state",
  "roller/get/desired/state",
  "roller/update/state"};

const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAKKouzlg0AVFfs+CvkRzlUF3oD6zMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMjEwMjAwMDUz
MDBaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCkSiaWd8zlSZD4RCVR
vUQO4LrMZYwmC0XwSq5fKaaCcPsTVebf5BnsPL0gXdUDlydPA+706I84625zLZ+R
uHhJEcuoeM81Ppyg2pLUwBO+e3e66keIBT2W22ih9SleHmrPv+AFr83dVC2Kf0vI
b5jiYuaKbQVzHdIahi7lfr7jmf0Syq3l2837Ox7oELkS6a+nA2FyLuxuRpSavwKA
I1DJezDNsJdjI2HUbMs5NbJfesX4Ju1ns1iTtBALttK5H1WRRqJEkhA/gqjK8hB6
+OpH5W1VB8BwT9cilfaBSm94+zrSOFwqjWP2nnFnug1WX9XBQ/+dN2qEE1JkAc5x
+GHpAgMBAAGjYDBeMB8GA1UdIwQYMBaAFEwjhveYXR8RdQiW/QMKgRwqUDflMB0G
A1UdDgQWBBQNYWIJ7imPmxrTHUadsxlL7cDGpzAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAYyzBfd1cL3UjwLaPpDLtlIqf
F3f6bsRYnq53Xr4/jHb07Tg3qcrQW4hniGHro7OZQ2RCDaxcQas+zbAL5hX3C+Xq
BboV52DIQZwd02cy8yQ3dnRuB52MG0+KQIRc4FD2H4yMAICWtOmqZbczAbzUwoku
32G2oy+A2X8EoBXbzE5mE4eYZPbuNzB1Hkie43Q127PHvSK7Us5kXb3KGr2KrgiI
tErqPBHJ3pUahDFs7epJdOAmv/t3YdmmAEF3ZNgcRQQqNX5MopRVsmyttFuEGi1d
l96LwXCerqLesq15bayySLVttB82biAgcJIkq/tbzNi3QLGYeNttyytS6aRXzQ==
-----END CERTIFICATE-----)KEY";


const char PRIVATE_KEY[] PROGMEM = R"KEY(-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEApEomlnfM5UmQ+EQlUb1EDuC6zGWMJgtF8EquXymmgnD7E1Xm
3+QZ7Dy9IF3VA5cnTwPu9OiPOOtucy2fkbh4SRHLqHjPNT6coNqS1MATvnt3uupH
iAU9lttoofUpXh5qz7/gBa/N3VQtin9LyG+Y4mLmim0Fcx3SGoYu5X6+45n9Esqt
5dvN+zse6BC5EumvpwNhci7sbkaUmr8CgCNQyXswzbCXYyNh1GzLOTWyX3rF+Cbt
Z7NYk7QQC7bSuR9VkUaiRJIQP4KoyvIQevjqR+VtVQfAcE/XIpX2gUpvePs60jhc
Ko1j9p5xZ7oNVl/VwUP/nTdqhBNSZAHOcfhh6QIDAQABAoIBABLcgHmd32QjggU+
rZooxHuAyXFV5zUm7iycJlC+k2M+XVTm886YfXFlt8DThPUYkWa86N4tJORAAzot
7rfoGccpx9vbos5WFd1RurG6K4oe2qwW4Yd4Mo3zOpv9bXZkv5aCTNHaUSQr2d6g
pq6oQUEcYaNayNlifxCUfyXR21B92i3f/gI9e7+k63e/70189RsOAGSTtX8rjtlu
MKQucIXwWc1tBM3jSotZiLDrEFg+D49IXPuQI8ZsOs4Yqe9RcwIBTZ+oEyeXwUV6
f7sVVIiPqa7MN6GBxdRnfSebeUKhDVRDDdFtIHfy/rQEN7LSZSV8CUopafZKbudp
Yo4cZEECgYEA17HWmRh50E1x65osp1/mVz22fJatM6WpdAC9ku902KaKAqLTKMRo
sTh1i6YugDdaq+Ti1PB7MKlOtQG+85zxZI+XmgqMR5PGDxup/uOajzLbdx1RUD8c
zy1MJfSfQm7DpGt+22v86lvdaYk06aLNy2c4Tv3JQqBpOvxTtBzNHO0CgYEAwv1B
hl41SH6JiW5wSDngjVpcJ92uSpUiBrWZHUgJW9C1lAtUk/KyyBB2f5fMxFlycqEV
If68DqZ7okZp66G2Xk/LFLcJ6EHA7RIYqmATprAvCkbBPlJcBOPSrQ8Ha7WfKMw9
lUdg6ZOPIPHh98ZH3kKYup6Fj3hFEPIU7Qd+NW0CgYEAnmAVW3puTzXeVQlyij9P
SXWl3dthN9AHGYdFq3Mpz76RDZhzBbcZzC7RmIWgFUMPz7GToJknSza19RBgHk55
rMYGofPmxtPJlORSMTb8EPNd1BtxzkNZKwJurFvu6H+eJcdMcV7caVSkwc4eTQLe
quCSpuWP9t0EB2ypJVIUhY0CgYB3400xd6UdQKgB9wRUdDmLz74PyO5L7yvvoiJV
baBLcqTiNKE5IiUoe27Qfp4pL7H9pusebshj3ZrwqIihl1lQ3ZDI2M3fnuYnHVRL
FS2f9H3HvL8+OVdYrlcvjqkKYa5NYF6Q9UUx+Ectg2tjc+dmUd4kCCmoj9MvWxjg
sBW6dQKBgQC1aLzcQnoNNExZL6jJtS23PV9VhVf17uHUhpLizysG+8K3HJr9HXjO
C8a2NULbkVLsfVBXqmitXOT4LTNSn+O9FsgZ7FSV0kHDjkcP7P2Brwe7mh2+oFGZ
4EIss0Kv/x89GF5JWLu7wOv5PnerF3X8CbVB4o3nfxcZQf7KgOVUEw==
-----END RSA PRIVATE KEY-----)KEY";


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
  Serial.println("entro");
    if(desiredState!=currentCurtainState )
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
  
  if (String(topic) == SUBSCRIBE_TOPICS_ARRAY[listenForUpdateTopicIndex] or String(topic) == SUBSCRIBE_TOPICS_ARRAY[desiredGetStateTopicIndex]) 
  {
    Serial.println("Message from topic " + String(topic) + " : " + message);
    DeserializationError err = deserializeJson(inputDoc, payload);  
    if(!err)
    {
        String desiredState = String(inputDoc["rollers"][rollerID]["curtainState"].as<char*>());
        Serial.println(desiredState);
        if(desiredState!="")
        {
          changeCurtainState(desiredState);
        }
    }
  }
  
  if (String(topic) == SUBSCRIBE_TOPICS_ARRAY[getShadowStateTopicIndex]) 
  {
    Serial.println("Message from topic " + String(topic) + " : " + message);
    DeserializationError err = deserializeJson(inputDoc, payload);  
    if(!err)
    {
      String curtainState=String(inputDoc["rollers"][rollerID]["curtainState"].as<char*>());
        
        if(curtainState=="")
        {
          currentCurtainState=openCurtainState;
          timeToOpenRoller=0;
        }
        else
        {
          currentCurtainState=curtainState;
          timeToOpenRoller =float(inputDoc["rollers"][rollerID]["timeToOpenRoller"]);
        }
        Serial.println(timeToOpenRoller);
        Serial.println(currentCurtainState);
    }
    
  }
}




void SubscribeToTopics()
{
  int size = sizeof(SUBSCRIBE_TOPICS_ARRAY)/sizeof(SUBSCRIBE_TOPICS_ARRAY[0]);
  for(int i=0;i<size;i++)
  {
    mqttClient.subscribe(SUBSCRIBE_TOPICS_ARRAY[i].c_str());
    Serial.println("Subscribed to " + SUBSCRIBE_TOPICS_ARRAY[i]);
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

    WiFiManager wm;
    bool res;
    res = wm.autoConnect("AutoConnectAP"); 
    if(!res) {
        Serial.println("Wifi Couldnt Connect");
    } 
    else {  
        Serial.println("Wifi Connected");
    }


    wifiClient.setCACert(AMAZON_ROOT_CA1);
    wifiClient.setCertificate(CERTIFICATE);
    wifiClient.setPrivateKey(PRIVATE_KEY);
    mqttClient.setServer(AWS_ENDPOINT, AWS_ENDPOINT_PORT);
    mqttClient.setCallback(callback);
    mqttClient.setBufferSize(1024);
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

unsigned char counter = 0;



StaticJsonDocument<JSON_OBJECT_SIZE(7)> outputDoc;
char outputBuffer[128];
void sendJSONWithCurrentState(bool sendCurtainState,bool sendTimeToOpenRoller)
{
    outputDoc["state"]["reported"]["rollers"][rollerID]["curtainState"] = currentCurtainState.c_str();
    Serial.println("mandar tiempo:"+sendTimeToOpenRoller );
    if(sendTimeToOpenRoller)
    {
        outputDoc["state"]["reported"]["rollers"][rollerID]["timeToOpenRoller"] = timeToOpenRoller;
    }
    serializeJson(outputDoc, outputBuffer);
    Serial.println("Intenta enviar un estado");
    Serial.println(outputBuffer);
    mqttClient.publish(PUBLISH_TOPICS_ARRAY[reportStateTopicIndex].c_str(), outputBuffer);
}
StaticJsonDocument<JSON_OBJECT_SIZE(1)> outputDocGetState;
char outputBufferGetState[128];
void sendJSONToAskForShadowInfo()
{
    outputDocGetState["id"] = rollerID.c_str();
    serializeJson(outputDocGetState, outputBufferGetState);
    Serial.println(outputBufferGetState);
    mqttClient.publish(PUBLISH_TOPICS_ARRAY[requestShadowTopicIndex].c_str(), outputBufferGetState);
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
