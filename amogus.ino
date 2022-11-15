int SensorPIR = 2;
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
  Serial.println (cou);
   if (val == 0) {
     cou = cou + 1; //
   }
   if (val == 1) {
     cou = 0; //
   }
   if (cou < 25) {
    digitalWrite (buzzer, HIGH);
    Serial.println ("Buzzer OFF");
   }
   if (cou > 25) {
    digitalWrite (buzzer, LOW);
    Serial.println("Buzzer ON");
   }
}