int pir_pin = 2;
int buzzer_pin = 7;
void setup()
{
 pinMode(pir_pin, INPUT);
 pinMode(buzzer_pin, OUTPUT);
 Serial.begin(9600);
}

void loop()
{
  if(digitalRead(pir_pin) == HIGH)
   {
    digitalWrite(buzzer_pin, HIGH);
    Serial.println("Buzzing");
   }
   else{
    digitalWrite(buzzer_pin, LOW);
    Serial.println("No Buzz");
   }
}