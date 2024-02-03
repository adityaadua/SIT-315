const int pir_pin = 2;
const int led_pin = 3;
const int light_sensor_pin = A0;
const int buzzer_pin = 7;

volatile bool motionDetected = false;

void setup()
{
  pinMode(pir_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(light_sensor_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(pir_pin), pirInterrupt, CHANGE);
}

void loop()
{
  delay(1000);

  // Check if the LED should be turned on based on PIR sensor
  if (motionDetected)
  {
    digitalWrite(led_pin, HIGH); // Turn on the LED when motion is detected
    Serial.println("LED turned on");
  }
  else
  {
    digitalWrite(led_pin, LOW); // Turn off the LED when no motion is detected
    Serial.println("LED turned off");
  }

  int lightValue = analogRead(light_sensor_pin);

  // Check if the buzzer should be turned on based on light sensor value
  if (lightValue > 500)
  {
    digitalWrite(buzzer_pin, HIGH); // Turn on the buzzer when light is detected
    Serial.println("Buzzing");
  }
  else
  {
    digitalWrite(buzzer_pin, LOW); // Turn off the buzzer when no light is detected
    Serial.println("No Buzz");
  }
}

void pirInterrupt()
{
  if (digitalRead(pir_pin) == HIGH)
  {
    motionDetected = true;
  }
  else
  {
    motionDetected = false;
  }
}
