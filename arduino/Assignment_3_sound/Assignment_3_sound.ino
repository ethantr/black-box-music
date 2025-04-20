#define sensorPin A0 
const int trigPin = 11;
const int echoPin = 10;
float distance = 0.0; // Variable to store the distance value
int noise = 0; // Variable to store the noise value
int pressure = 0; // Variable to store the pressure value

void setup() {
  pinMode(trigPin, OUTPUT);  
	pinMode(echoPin, INPUT);  
  Serial.begin(9600); 
}

float readDistance() {

  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2);  
  digitalWrite(trigPin, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(trigPin, LOW);  

  float duration = pulseIn(echoPin, HIGH);  
  distance = (duration*.0343)/2; 
  
  return distance;
}

void loop() {

  distance = readDistance();
  noise = analogRead(sensorPin);
  pressure = (noise * 5.0) / 1023.0; // Convert to voltage
  pressure = (pressure - 0.5) * 100; // Convert to pressure in kPa

  // Print the values to the Serial Monitor
  Serial.print("D:");
  Serial.print(distance);
  Serial.print(",P:");
  Serial.print(pressure);
  Serial.print(",N:");
  Serial.println(noise);  

  delayMicroseconds(10);
}