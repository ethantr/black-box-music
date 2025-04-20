#define sensorPin A0 
const int trigPin = 11;
const int echoPin = 10;

float duration, distance;  

void setup() {
  pinMode(trigPin, OUTPUT);  
	pinMode(echoPin, INPUT);  
  Serial.begin(9600); 
}

void loop() {
  int x = analogRead(sensorPin);
  // Serial.println(x);

  if(x > 50){
    Serial.println("note_on, 60");
    delay(500);
    Serial.println("note_off,60");
  }

  digitalWrite(trigPin, LOW);  
	delayMicroseconds(2);  
	digitalWrite(trigPin, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(trigPin, LOW);  

  duration = pulseIn(echoPin, HIGH);  
  distance = (duration*.0343)/2; 

  Serial.print("Distance: ");  
	Serial.println(distance);  


  delay(50);
}