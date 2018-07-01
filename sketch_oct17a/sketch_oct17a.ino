#define trigPin 13
#define echoPin 12
#define led 11
#define empty_led 10

int lastDistance = 0;
int thisDistance = 0;
int dm = 200;

void setup() {
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led, OUTPUT);
  pinMode(empty_led, OUTPUT);
}

void loop() {
  long duration, distance;

  lastDistance = thisDistance;
  
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;

  thisDistance = distance;
  
  if (distance <  dm && !outlierCheck(lastDistance, thisDistance)) {  // This is where the LED On/Off happens
    digitalWrite(led,HIGH); // When the Red condition is met, the Green LED should turn off
    digitalWrite(empty_led,LOW);

}
  else if(!outlierCheck(lastDistance, thisDistance)) {
    digitalWrite(led,LOW);
    digitalWrite(empty_led,HIGH);

  }
  if(outlierCheck(lastDistance, thisDistance)){
    Serial.println("outlier");
  }
  else if (distance >= dm || distance <= 0){
    Serial.println(distance);
    Serial.println("Out of range");
  }
  else {
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(500);
} 

bool outlierCheck(int lastDistance, int thisDistance) {

  if ((lastDistance - thisDistance ) > 20 || (thisDistance > dm && lastDistance < dm)){
    return true;
  }
  else {
      return false;
  }
}

