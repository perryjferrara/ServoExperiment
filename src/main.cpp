#include <Arduino.h>
#include <Servo.h> // Include the Servo library, needed a PlatformIO library dependency

Servo myServo; // Create a servo object to control a servo motor
// Twelve servo objects can be created on most boards

#define SERVO_PIN 9 // Define the pin connected to the servo motor

int pos = 0;                          // Variable to store the servo position
const int maxPosition = 180;          // Define the maximum position for the servo
const int interruptPin = 2;           // Define the pin connected to a button or switch
const int pinLEDG = 3;                // Define the pin connected to an LED (green)
const int pinLEDW = 4;                // Define the pin connected to another LED (white)
const int pinLEDR = 5;                // Define the pin connected to another LED (red)
const int pinMeasure = A0;            // Define the pin connected to the light sensor
int lowestReading;                    // Variable to store the local minimum light level
int determinedPosition;               // Variable to store the position where minimum light is detected
volatile bool resetRequested = false; // Variable to indicate if a reset is requested
int loopCount = 1;                    // Variable to store the number of times the loop has executed
int cancelledLoops = 0;               // Variable to store the number of cancelled loops
int scanPeriod = 1000;                // Define the period between scans (in milliseconds)
int rescanThreshold = 100;            // Define the threshold for triggering a rescan
int rescanCount = 0;                  // Variable to store the number of rescans performed

void reset()
{                        // Reset function implementation
  resetRequested = true; // Set the reset flag to true
}

bool handleReset()
{ // Handle reset function implementation
  if (!resetRequested)
  {               // Check if a reset is requested
    return false; // Return false if no reset is requested
  }

  digitalWrite(pinLEDW, LOW);  // Turn off the white LED for reset
  digitalWrite(pinLEDG, LOW);  // Turn off the green LED for reset
  digitalWrite(pinLEDR, HIGH); // Turn on the red LED indicating a reset is requested

  Serial.println(" ");                                           // Print a blank line
  Serial.println("Button pressed, user has requested a rescan"); // Print a message indicating the scan was cancelled
  delay(2000);                                                   // Wait for 2 seconds

  resetRequested = false;     // Clear after the pause to absorb button bounce during that time.
  digitalWrite(pinLEDR, LOW); // Turn off the red LED
  cancelledLoops++;           // Increment the cancelled loops count

  Serial.println("---------------------------------------------------------------------------------"); // Print a separator line
  return true;                                                                                         // Return true if a reset was handled
}

void setup()
{
  pinMode(interruptPin, INPUT_PULLUP);                                                                                                           // Configure the button pin with an internal pull-up resistor
  pinMode(pinLEDG, OUTPUT);                                                                                                                      // Configure the greenLED pin as an output
  pinMode(pinLEDW, OUTPUT);                                                                                                                      // Configure the white LED pin as an output
  pinMode(pinLEDR, OUTPUT);                                                                                                                      // Configure the red LED pin as an output
  pinMode(pinMeasure, INPUT);                                                                                                                    // Configure the light sensor pin as an input
  Serial.begin(9600);                                                                                                                            // Initialize serial communication at 9600 baud rate
  myServo.attach(SERVO_PIN);                                                                                                                     // Attach the servo to pin 9 on the Arduino board
  attachInterrupt(digitalPinToInterrupt(interruptPin), reset, FALLING);                                                                          // Attach the interrupt to reset, falling edge to detect button press
  Serial.println("---------------------------------------------------------------------------------");                                           // Print a separator line
  Serial.println("Program uploaded to Arduino board: Servo Experiment");                                                                         // Print a message indicating the program has been uploaded
  Serial.println(" ");                                                                                                                           // Print a blank line for readability
  Serial.println("This program is designed to control a servo motor based on light sensor readings.");                                           // Print a message describing the program
  Serial.println("- A green LED illuminates as long as the system is active.");                                                                  // Print a message describing the LED behavior
  Serial.println("- A white LED will illuminate while the system is scanning, and blink when an automatic rescan is initiated.");                // Print a message describing the LED behavior
  Serial.println("- A red LED will illuminate when the system is manually reset. To reset the system, press the button on the mini breadboard."); // Print a message describing the LED behavior
  Serial.println(" ");                                                                                                                           // Print a blank line for readability
  Serial.println("Please input a value for the period between scans (in milliseconds):");                                                        // Print a message prompting the user to input a value
  while (true)                                                                                                                                   // Wait for serial input
  {
    if (Serial.available() == 0) // Check if there is no available serial data
    {
      delay(100); // Delay for 100 milliseconds
    }
    long inputP = Serial.parseInt(); // Read the input from the serial port
    if (inputP > 0)                  // Check if the input is a positive value
    {
      scanPeriod = inputP; // Set the scan period to the input value
      break;               // Break out of the loop
    }
  }
  Serial.println("Scan period set to " + String(scanPeriod) + " ms");                                                  // Print a message indicating the scan period
  Serial.println(" ");                                                                                                 // Print a blank line for readability
  Serial.println("Please input a value for the rescan threshold (+/- ___ from the lowest reading initiates rescan):"); // Print a message prompting the user to input a value
  while (true)                                                                                                         // Wait for serial input
  {
    if (Serial.available() == 0) // Check if there is no available serial data
    {
      delay(100); // Delay for 100 milliseconds
    }
    long inputT = Serial.parseInt(); // Read the input from the serial port
    if (inputT > 0)                  // Check if the input is a positive value
    {
      rescanThreshold = inputT; // Set the rescan threshold to the input value
      break;                    // Break out of the loop
    }
  }
  Serial.println("Rescan threshold set to " + String(rescanThreshold) + " units");                     // Print a message indicating the rescan threshold
  Serial.println("---------------------------------------------------------------------------------"); // Print a separator line
}

void loop()
{
  digitalWrite(pinLEDG, HIGH); // LED indicating that circuit is active

  Serial.println("Loop iteration #" + String(loopCount) + " | No. of cancelled loops: " + String(cancelledLoops) + " | No. of automatic rescans: " + String(rescanCount)); // Print the loop iteration number, the number of cancelled loops, and the number of automatic rescans
  Serial.println(" ");                                                                                                                                                     // Print a blank line for readability
  digitalWrite(pinLEDW, LOW);                                                                                                                                              // Turn off the white LED before scan
  digitalWrite(pinLEDR, LOW);                                                                                                                                              // Turn off the red LED after potential button press

  lowestReading = 1024;   // Initialize the lowest reading to the maximum possible analog value
  determinedPosition = 0; // Initialize the position where minimum light is detected

  for (pos = 0; pos <= maxPosition; pos += 1)
  {                              // Loop from 0 to 180 degrees
    digitalWrite(pinLEDW, HIGH); // Turn on the white LED while the servo is determining max light level
    myServo.write(pos);          // Tell the servo to go to the position stored in the 'pos' variable: 0 to 180
    delay(25);                   // Wait for 0 to 180 position

    if (handleReset())
    {         // Check if a reset was requested
      return; // Don't measure or continue a cancelled scan.
    }

    int reading = analogRead(pinMeasure); // Read the analog value from pinMeasure
    if (reading < lowestReading)
    {                           // Check if the current reading is less than the lowest reading
      lowestReading = reading;  // Update the lowest reading
      determinedPosition = pos; // Update the position where lowest light is detected
    }
  }

  if (handleReset())
  {         // Check if a reset was requested
    return; // Return if a reset was handled
  }

  myServo.write(determinedPosition);                                                           // Tell the servo to go to the position where lowest light is detected
  digitalWrite(pinLEDW, LOW);                                                                  // Turn off the white LED indicating the scan is complete
  Serial.println("Local maximum light level:   " + String(lowestReading));                     // Print the local maximum light level
  Serial.println("Position where maximum light is detected:   " + String(determinedPosition)); // Print the position where maximum light is detected
  loopCount++;                                                                                 // Increment the loop count

  unsigned long waitStart = millis(); // Wait for the scan period

  while (millis() - waitStart < scanPeriod)
  { // Check if the scan period has passed
    if (handleReset())
    { // Check if a reset was requested
      return;
    }
    delay(1); // Delay for 1 millisecond
    if (abs(analogRead(pinMeasure) - lowestReading) > rescanThreshold)
    {                                                                                                                          // White LED blinks twice to indicate a rescan is needed
      Serial.println(" ");                                                                                                     // Print a blank line
      Serial.println("Pin measure exceeded threshold, rescan needed: " + String(abs(analogRead(pinMeasure) - lowestReading))); // Print a message indicating a rescan is needed
      digitalWrite(pinLEDW, HIGH);                                                                                             // Turn on the white LED to indicate a rescan is needed
      delay(500);                                                                                                              // Delay for 100 milliseconds
      digitalWrite(pinLEDW, LOW);                                                                                              // Turn off the white LED after the rescan is complete
      delay(500);                                                                                                              // Delay for 100 milliseconds
      digitalWrite(pinLEDW, HIGH);                                                                                             // Turn on the white LED to indicate a rescan is needed
      delay(500);                                                                                                              // Delay for 100 milliseconds
      digitalWrite(pinLEDW, LOW);                                                                                              // Turn off the white LED after the rescan is complete
      rescanCount++;                                                                                                           // Increment the rescan count
      delay(2000);                                                                                                             // Delay for 2000 milliseconds
      break;                                                                                                                   // Exit the while loop
    }
  }
  Serial.println("---------------------------------------------------------------------------------"); // Print a separator line
}