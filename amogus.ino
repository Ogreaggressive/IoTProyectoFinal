int SensorPIR = 26;
int buzzer = 13;
int val = 0;
int cou = 0;
void setup ( ) {
  pinMode (SensorPIR, INPUT); // sets the pin as output
  pinMode (buzzer, OUTPUT); // sets the pin as output
  Serial.begin (9600);
 }
void loop() {
  val = digitalRead (SensorPIR);
  Serial.print ("val = ");
  Serial.println (val);
  if(val==0)
  {
    Serial.println("hay algo en medio");
  }
  else
  {
    Serial.println("no hay algo en medio");
  }
  delay(1000);
}