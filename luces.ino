int ldr;
void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT); //led
  pinMode(12, OUTPUT); //led
  pinMode(A0, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  ldr = analogRead(A0);
  if(ldr > 700)
  {
    digitalWrite(13, 1);
    digitalWrite(12, 0);
  }
  else
  {
    digitalWrite(13, 0);
    digitalWrite(12, 1);
  }
  Serial.println(ldr);
  delay(50);
  }
