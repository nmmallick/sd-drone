#include "Arduino.h"
#include <SoftwareSerial.h>
// Function Declarations
void printAng();
int testInputs();
void sendBT(int value); // Updated function signature
String RECEIVE(String &rxMsg);

// BT Globals
const byte txp = 8;
const byte rxp = 9;
SoftwareSerial BTSerial(rxp, txp);
bool isBTConnected = false;

// BT Messaging Globals
String rxMsg = "";
String messageToSend = "Hello world!";

// Analog Testing Globals
int thresh = 20;
int pot0_prev = 0;
int pot1_prev = 0;
int pot2_prev = 0;

// setup routine runs on reset:
void setup() {
  pinMode(rxp, INPUT);
  pinMode(txp, OUTPUT);
  Serial.begin(9600);
  BTSerial.begin(9600);
  Serial.println("======First Read========");
  printAng();
  BTSerial.println(messageToSend);
  isBTConnected = true;
}

void loop() {
  // Read the input on analog pin 0:
  int x = testInputs();
  if (x != 0) {
    String xstr = String(x);
    sendBT(x);
  }
  delay(100);
}
void sendBT(int value) {
  if (!isBTConnected) {
    BTSerial.begin(9600); // Open the Bluetooth connection if not already open
    isBTConnected = true;
  }
  
  BTSerial.println("REPLY " + value); // Send the value to Bluetooth
  Serial.print("Value sent: ");
  Serial.println(value); // Update the user in the serial monitor
}

void printAng() {
  int pot0 = analogRead(A0);
  int pot1 = analogRead(A1);
  int pot2 = analogRead(A2);
  Serial.print("pot0: ");
  Serial.println(pot0);
  Serial.print("pot1: ");
  Serial.println(pot1);
  Serial.print("pot2: ");
  Serial.println(pot2);
}

int testInputs() {
  // Start with current values
  int pot0curr = analogRead(A0);
  int pot1curr = analogRead(A1);
  int pot2curr = analogRead(A2);

  // Need absolute value or only counts up
  int diff0 = abs(pot0curr - pot0_prev);
  int diff1 = abs(pot1curr - pot1_prev);
  int diff2 = abs(pot2curr - pot2_prev);

  // Check against threshold
  if (diff0 > thresh || diff1 > thresh || diff2 > thresh) {
    // See debouncing methods
    delay(100);

    // Confirm value change
    int pot0new = analogRead(A0);
    int pot1new = analogRead(A1);
    int pot2new = analogRead(A2);

    int diff0new = abs(pot0new - pot0_prev);
    int diff1new = abs(pot1new - pot1_prev);
    int diff2new = abs(pot2new - pot2_prev);

    // If the new difference is still greater than the threshold, return it
    if (diff0new > thresh) {
      Serial.print("Analog value for pot0 changed to: ");
      Serial.println(pot0new);

      // Update pot0_prev to the new value
      pot0_prev = pot0new;
      return pot0new;
    }

    // Check against the threshold for pot1
    if (diff1new > thresh) {
      Serial.print("Analog value for pot1 changed to: ");
      Serial.println(pot1new);

      // Update pot1_prev to the new value
      pot1_prev = pot1new;
      return pot1new;
    }

    // Check against the threshold for pot2
    if (diff2new > thresh) {
      Serial.print("Analog value for pot2 changed to: ");
      Serial.println(pot2new);

      // Update pot2_prev to the new value
      pot2_prev = pot2new;
      return pot2new;
    }
  }

  // If no value change met the threshold, return 0
  return 0;
}
