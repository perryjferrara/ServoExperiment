#include <Arduino.h>
#include <Servo.h> // Include the Servo library, needed a PlatformIO library dependency

Servo myServo; // Create a servo object to control a servo motor
// Twelve servo objects can be created on most boards

#define SERVO_PIN 9 // Define the pin connected to the servo motor

int pos = 0; // Variable to store the servo position
const int maxPosition = 180; // Define the maximum position for the servo
const int pinNum = 2; // Define the pin connected to a button or switch
const int pinLEDG = 3; // Define the pin connected to an LED (green)
const int pinLEDW = 4; // Define the pin connected to another LED (white)
const int pinLEDR = 5; // Define the pin connected to another LED (red)
const int pinMeasure = A0; // Define the pin connected to the light sensor
int lowestReading; // Variable to store the local minimum light level
int determinedPosition; // Variable to store the position where minimum light is detected


void setup() {
  pinMode(pinNum, INPUT_PULLUP); // Configure the button pin with an internal pull-up resistor
  pinMode(pinLEDG, OUTPUT); // Configure the greenLED pin as an output
  pinMode(pinLEDW, OUTPUT); // Configure the white LED pin as an output
  pinMode(pinLEDR, OUTPUT); // Configure the red LED pin as an output
  pinMode(pinMeasure, INPUT); // Configure the light sensor pin as an input
  Serial.begin(9600); // Initialize serial communication at 9600 baud rate
  myServo.attach(SERVO_PIN); // Attach the servo to pin 9 on the Arduino board
}

void loop() {
  digitalWrite(pinLEDW, LOW); // Turn off the white LED before scan
  digitalWrite(pinLEDG, HIGH); // LED indicating that circuit is active
  lowestReading = 1024; // Initialize the lowest reading to the maximum possible analog value
  determinedPosition = 0; // Initialize the position where minimum light is detected

  for (pos = 0; pos <= maxPosition; pos += 1) { // Loop from 0 to 180 degrees
    digitalWrite(pinLEDW, HIGH); // Turn on the white LED while the servo is determining max light level
    myServo.write(pos); // Tell the servo to go to the position stored in the 'pos' variable: 0 to 180
    delay(25); // Wait for 0 to 180 position
    int reading = analogRead(pinMeasure); // Read the analog value from pinMeasure
    if (reading < lowestReading) { // Check if the current reading is less than the lowest reading
      lowestReading = reading; // Update the lowest reading
      determinedPosition = pos; // Update the position where lowest light is detected
    }
  }

  myServo.write(determinedPosition); // Tell the servo to go to the position where lowest light is detected
  Serial.println("Local minimum light level: "); // Print the local minimum light level
  Serial.println(lowestReading);
  Serial.println("Position where minimum light is detected: "); // Print the position where minimum light is detected
  Serial.println(determinedPosition);
  digitalWrite(pinLEDW, LOW); // Turn off the white LED indicating the scan is complete
  delay(5000); // Wait for 5 seconds before scanning again
}