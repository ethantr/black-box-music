#define sensorPin A0 
const int trigPin = 11;
const int echoPin = 10;
const int ledEnable = 12;
const int ledEnable2 = 9;
const int ledEnable3 = 8;
 // Variable to store the distance value
float noise = 0; // Variable to store the noise value
float pressure = 0; // Variable to store the pressure value

void setup() {
  pinMode(trigPin, OUTPUT);  
	pinMode(echoPin, INPUT);  
  pinMode(ledEnable, OUTPUT);
  pinMode(ledEnable2, OUTPUT); 
  pinMode(ledEnable3, OUTPUT);
  Serial.begin(9600); 
}

float readDistance() {

  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2);  
  digitalWrite(trigPin, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(trigPin, LOW);  

  float duration = pulseIn(echoPin, HIGH);  
  float distance = (duration*.0343)/2; 
  return distance;
}

void loop() {

  float distance = readDistance();
  noise = analogRead(sensorPin);
  delay(25);
  pressure = (noise * 5.0) / 1023.0; // Convert to voltage
  pressure = (pressure - 0.5) * 100; // Convert to pressure in kPa

  if (noise > 50) { 
    digitalWrite(ledEnable, HIGH); 
    digitalWrite(ledEnable2, HIGH); 
    digitalWrite(ledEnable3, HIGH);
    //delay(25);
  } else { 
    digitalWrite(ledEnable, LOW);
    digitalWrite(ledEnable2, LOW); 
    digitalWrite(ledEnable3, LOW);
  }



  // Print the values to the Serial Monitor
  Serial.print("D:");
  Serial.print(distance);
  Serial.print(",P:");
  Serial.print(pressure);
  Serial.print(",N:");
  Serial.println(noise);  

  delayMicroseconds(10);
}