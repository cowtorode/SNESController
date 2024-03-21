/*
 * File: adapter.cpp
 * Author: Cory Torode
 * Creation Date: 03/18/2024
 * Description: Arduino SNES Controller adapter, reads the SNES Controller and forwards it to the driver.
 * (Made for the Arduino Nano)
 */

#define CLOCK_PIN 2    // Clock pin of the SNES controller connected to Arduino pin 2 (D2)
#define LATCH_PIN 3    // Latch pin of the SNES controller connected to Arduino pin 3 (D3)
#define DATA_PIN 4     // Data pin of the SNES controller connected to Arduino pin 4 (D4)

void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    pinMode(CLOCK_PIN, OUTPUT);  // Set clock pin as output
    pinMode(LATCH_PIN, OUTPUT);  // Set latch pin as output
    pinMode(DATA_PIN, INPUT);    // Set data pin as input
}

void loop() {
    uint16_t input = readSNESController();
    // wait 16 ms (16.67 ms) between inputs, the SNES was 60 Hz.
    delay(16);

    // Send serial data to USB port
    // Send the least significant byte first, ...
    Serial.write(input);
    // and then send the most significant byte
    Serial.write(input >> 8);
}

/**
 * @return 16 bit value representing the IC cache in the SNES controller, where bit 0 is the first bit selected out from the IC cache
 */
uint16_t readSNESController() {
    uint16_t input = 0;

    // Save button inputs in the controller
    digitalWrite(LATCH_PIN, HIGH);
    delayMicroseconds(12);
    digitalWrite(LATCH_PIN, LOW);
    delayMicroseconds(6);

    // Read button inputs
    // SNES Controller protocol only modifies 12 bits, so we don't need to iterate through the entire 16 bits.
    for (int i = 0; i < 16; i++) {
        // Store each bit (in reverse order)
        input |= digitalRead(DATA_PIN) << i; // 1111111111111111
        // Get next bit
        digitalWrite(CLOCK_PIN, HIGH);
        delayMicroseconds(6);
        digitalWrite(CLOCK_PIN, LOW);
        delayMicroseconds(6);
    }

    return input;
}
