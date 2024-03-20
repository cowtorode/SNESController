
// SNES Controller Read Example for Arduino Nano
// Reads button states from a SNES controller

#define CLOCK_PIN 2    // Clock pin of the SNES controller connected to Arduino pin 2 (D2)
#define LATCH_PIN 3    // Latch pin of the SNES controller connected to Arduino pin 3 (D3)
#define DATA_PIN 4     // Data pin of the SNES controller connected to Arduino pin 4 (D4)

void setup() {
    Serial.begin(9600);   // Initialize serial communication
    pinMode(CLOCK_PIN, OUTPUT);  // Set clock pin as output
    pinMode(LATCH_PIN, OUTPUT);  // Set latch pin as output
    pinMode(DATA_PIN, INPUT);    // Set data pin as input
}

void loop() {
    // Read button states from SNES controller
    uint16_t input = readSNESController();

    // Print button states to Serial Monitor
    // Serial.print("Button States: ");
    // Serial.println(input, BIN);  // Print in binary format ( for some reason this prints in 32 bits )

    // send serial data to usb port
    // send least significant byte first
    Serial.write(input);
    // and then send most significant byte
    Serial.write(input >> 8);

    // delay(100);  // Delay if to avoid flooding the Serial Monitor
}

/**
 * @return 16 bit value representing the IC cache in the SNES controller, where bit 0 is the first bit selected out from the IC cache
 */
uint16_t readSNESController() {
    uint16_t input = 0;

    digitalWrite(LATCH_PIN, HIGH);
    delayMicroseconds(12);
    digitalWrite(LATCH_PIN, LOW);
    delayMicroseconds(6);

    for (int i = 0; i < 16; i++) {
        // store each bit ( in reverse order )
        input |= digitalRead(DATA_PIN) << i;
        // get next bit
        digitalWrite(CLOCK_PIN, HIGH);
        delayMicroseconds(6);
        digitalWrite(CLOCK_PIN, LOW);
        delayMicroseconds(6);
    }

    return input;
}