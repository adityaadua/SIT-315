#include <avr/sleep.h>

const int motionSensor = 2;              // the pin that the motion sensor is attached to
const int ambientSensor = 3;             // the pin that the ambient light sensor is attached to
const int tiltSensor = 4;                // the pin that the tilt sensor is attached to
const int ledMotion = 5;                 // the pin for indicating motion
const int buzzer = 7;                    // the pin for the buzzer
const int ledTilt = 8;                   // the pin for indicating tilt
const int ledTimer = 9;                  // the pin for indicating timer

int motionState = 0;
int ambientState = 0;
int tiltState = 1;

volatile bool timerFlag = false;

void setup() {
  Serial.begin(9600);                    // initialize serial communication
  pinMode(motionSensor, INPUT);          // initialize motion sensor pin as input
  pinMode(ambientSensor, INPUT);         // initialize ambient light sensor pin as input
  pinMode(ledMotion, OUTPUT);            // initialize motion LED pin as output
  pinMode(buzzer, OUTPUT);               // initialize buzzer pin as output
  pinMode(ledTilt, OUTPUT);              // initialize tilt LED pin as output
  pinMode(ledTimer, OUTPUT);             // initialize timer LED pin as output

  // Configure PCINT for tilt sensor (pin 4)
  PCICR |= (1 << PCIE2);          // Enable PCINT for port D (pins 0-7)
  PCMSK2 |= (1 << PCINT20);       // Enable interrupt on pin 4 (PCINT20)

  // Configure Timer1 for interrupt every 1 second
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, prescaler 1024
  OCR1A = 15624;  // Timer compare value for 1 second interrupt
  TIMSK1 |= (1 << OCIE1A);
}

void loop() {
  if (timerFlag) {
    digitalWrite(ledTimer, !digitalRead(ledTimer)); // Toggle the timer LED
    timerFlag = false;  // Reset the flag
    sleep_cpu();  // Enter sleep mode until next interrupt
  }
}

// Motion sensor interrupt service routine
void motionInterrupt() {
  motionState = digitalRead(motionSensor);
  digitalWrite(ledMotion, motionState);
  if (motionState == HIGH) {
    Serial.println("Motion Detected! - Turned RED led");
  }
}

// Ambient light sensor interrupt service routine
void ambientInterrupt() {
  ambientState = digitalRead(ambientSensor);
  digitalWrite(buzzer, ambientState);
  if (ambientState == HIGH) {
    Serial.println("High Ambient Light Detected! - Turned Buzzer");
  }
}

// Tilt sensor PCINT interrupt service routine
ISR(PCINT2_vect) {
  if (digitalRead(tiltSensor) == HIGH) {
    tiltState = HIGH;
    digitalWrite(ledTilt, HIGH);
    Serial.println("Tilt Detected! - Turned Green led");
  } else {
    tiltState = LOW;
    digitalWrite(ledTilt, LOW);
  }
}

// Timer1 compare match interrupt service routine
ISR(TIMER1_COMPA_vect) {
  timerFlag = true; // Set the flag indicating 1-second interval
}
