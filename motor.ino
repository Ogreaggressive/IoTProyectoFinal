int speedPin = 16;
int rightPin = 27;
int leftPin = 14;
void stopMotor()
{
  digitalWrite (rightPin,LOW ) ;
  digitalWrite (leftPin, LOW);
}
void moveMotorLeft()
{
  digitalWrite (rightPin,LOW ) ;
  digitalWrite (leftPin, HIGH);
}
void moveMotorRight()
{
  digitalWrite (leftPin, LOW);
  digitalWrite (rightPin,HIGH ) ;
}
void setup () {
  pinMode (speedPin, OUTPUT);
  pinMode (leftPin, OUTPUT);
  pinMode (rightPin, OUTPUT);
  digitalWrite (speedPin,HIGH) ;


 }
void loop () {
   moveMotorRight();
   delay(3000);
   stopMotor();
   delay(3000);
}