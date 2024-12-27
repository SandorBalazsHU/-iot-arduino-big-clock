// Shift register pin definitions
#define DATA_PIN  4
#define CLOCK_PIN 6
#define LATCH_PIN 5

#define DISP_COUNT 6
#define CHAR_COUNT 20

// Character to 7-segment mapping (common cathode)
const uint8_t CHAR_MAP[CHAR_COUNT] = {
  0b00111111, // 0
  0b00110000, // 1
  0b01101101, // 2
  0b01111001, // 3
  0b01110010, // 4
  0b01011011, // 5
  0b01011111, // 6
  0b00110001, // 7
  0b01111111, // 8
  0b01111011, // 9
  0b01110111, // A
  0b01011110, // B
  0b00001111, // C
  0b01111100, // D
  0b01001111, // E
  0b01000111, // F
  0b00000000, // CLEAR
  0b10000000, // ,
  0b01100011, // * = °
  0b01001000  // =
};

// Shift register buffer for 6 digits
uint8_t displayBuffer[DISP_COUNT] = {0};

// Sends the buffer to the shift registers
void updateShiftRegisters(uint8_t buffer[], int size) {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = size - 1; i >= 0; i--) {
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, buffer[i]);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

// Sets a character on a specific display
void setDisplayCharacter(uint8_t buffer[], int digit, char character) {
  uint8_t charIndex;

  // Determine the character index for the 7-segment map
  if (character >= '0' && character <= '9') {
    charIndex = character - '0';
  } else if (character >= 'A' && character <= 'F') {
    charIndex = character - 'A' + 10;
  } else {
    switch (character) {
      case ' ':
        charIndex = 16;
      break;
      case '.':
        charIndex = 17;
      break;
      case '*':
        charIndex = 18;
      break;
      case '=':
        charIndex = 19;
      break;
      default:
        charIndex = 16;
      break;
    }

  }

  // Update the corresponding digit in the buffer
  buffer[digit] = CHAR_MAP[charIndex];
}

void setDisplayDot(uint8_t buffer[], int digit, bool on) {
  if(on){
    bitWrite(buffer[digit],7,1);
  } else {
    bitWrite(buffer[digit],7,0);
  }
}

void intToString(char* str, int i){
  char buf[6];
  sprintf (str, "%03i", i);
}

void dispTest(uint8_t buffer[], int size, int charCount){
  for(int c=0; c<charCount; c++){
    for(int digit=0; digit<size; digit++){
      buffer[digit] = CHAR_MAP[c];
    }
    updateShiftRegisters(buffer, size);
    delay(1000);
  }
}

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Clear the display initially
  for (int i = 0; i < 6; i++) {
    displayBuffer[i] = 0;
  }
  updateShiftRegisters(displayBuffer, DISP_COUNT);
  delay(1000);
  dispTest(displayBuffer, DISP_COUNT, CHAR_COUNT);
}

void loop() {
/*
  // Example usage: Display "A.1" on the first three digits
  setDisplayCharacter(displayBuffer, 0, '0');
  setDisplayCharacter(displayBuffer, 1, '0');

  // Update the shift registers to reflect the new buffer
  updateShiftRegisters(displayBuffer, DISP_COUNT);


  setDisplayDot(displayBuffer, 0, true);
  setDisplayDot(displayBuffer, 1, true);
  updateShiftRegisters(displayBuffer, DISP_COUNT);
  delay(1000); // Delay for visibility
  setDisplayDot(displayBuffer, 0, false);
  setDisplayDot(displayBuffer, 1, false);
  updateShiftRegisters(displayBuffer, DISP_COUNT);
  delay(1000); // Delay for visibility
  */
  delay(1000);
  dispTest(displayBuffer, DISP_COUNT, CHAR_COUNT);
}
