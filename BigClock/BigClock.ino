#include <string.h> // String manipulációkhoz

// Shift register pin definitions
#define DATA_PIN  4
#define CLOCK_PIN 6
#define LATCH_PIN 5

//Size of the display chain
#define DISP_SIZE 6


// Character to 7-segment mapping (common cathode)
#define CHAR_COUNT 22
const uint8_t charMap[CHAR_COUNT] = {
  0b00111111, // 0 -- HEX CHARS
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
  0b00000000, // CLEAR -- OTHER CHARS AND SYMBOLS
  0b10000000, // ,
  0b01100011, // * = °
  0b01001000, // =
  0b01001100, // c
  0b01100111  // P
};

// Shift register buffer for 6 digits
uint8_t displayBuffer[DISP_SIZE] = {0};

// Sends the buffer to the shift registers
void updateShiftRegisters(uint8_t buffer[], int displaySize) {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = displaySize - 1; i >= 0; i--) {
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, buffer[i]);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

void clearDisplay(uint8_t buffer[], int displaySize){
  for (int i = 0; i < 6; i++) {
    buffer[i] = 0; // Kikapcsoljuk az összes kijelzőt
  }
  updateShiftRegisters(buffer, 6);
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
      case 'c':
        charIndex = 20;
      break;
      case 'P':
        charIndex = 21;
      break;
      default:
        charIndex = 16;
      break;
    }

  }

  // Update the corresponding digit in the buffer
  displayBuffer[digit] = charMap[charIndex];
}

void setDisplayDot(uint8_t buffer[], int digit, bool on) {
  if(on){
    bitWrite(buffer[digit],7,1);
  } else {
    bitWrite(buffer[digit],7,0);
  }
}

/*void intToString(char* str, int i){
  char buf[6];
  sprintf (str, "%03i", i);
}*/

void displayInteger(uint8_t buffer[], int number, bool dispZero = false) {
  // Ellenőrizzük, hogy a szám 0 és 999999 között van-e
  if (number < 0 || number > 999999) {
    for (int i = 0; i < 6; i++) {
      buffer[i] = 0; // Kikapcsoljuk az összes kijelzőt
    }
    updateShiftRegisters(buffer, 6);
    return;
  }

  // A számjegyeket bontsuk le a megfelelő pozíciókra
  for (int i = 5; i >= 0; i--) {
    int digit = number % 10; // Az aktuális számjegy
    number /= 10;            // Haladjunk a következő számjegyre

    if (number == 0 && digit == 0 && i != 5) {
      // Ha nincs több számjegy és nem az utolsó kijelző, kapcsoljuk ki a kijelzőt
      if(dispZero){
        setDisplayCharacter(buffer, i, '0');
      }else{
        setDisplayCharacter(buffer, i, ' ');
      }
    } else {
      setDisplayCharacter(buffer, i, '0' + digit);
    }
  }

  // Frissítsük a shift regisztereket
  updateShiftRegisters(buffer, 6);
}

void displayFloat(uint8_t buffer[], float value, int decimals) {
  // Ellenőrizzük a tizedesek számát (legyen 0 és 2 között)
  if (decimals < 0 || decimals > 2) {
    decimals = 0; // Alapértelmezett pontosság 0 tizedesjegy
  }

  // Szám átalakítása az adott tizedes pontosságra
  int scaledValue = round(value * pow(10, decimals));

  // Számjegyek kiírása a kijelzőre
  for (int i = 5; i >= 0; i--) {
    if (scaledValue > 0 || i == 5 - decimals) {
      // Számjegy megjelenítése
      setDisplayCharacter(buffer, i, '0' + (scaledValue % 10));
      scaledValue /= 10;

      // Tizedespont hozzáadása a tört rész elé
      if (decimals > 0 && i == 5 - decimals) {
        setDisplayDot(buffer, i, true); // Tizedespont bekapcsolása
      }
    } else {
      // Kikapcsoljuk a maradék kijelzőt
      setDisplayCharacter(buffer, i, ' '); // Üres karakter
    }
  }

  // Shift regiszter frissítése
  updateShiftRegisters(buffer, 6);
}

#include <string.h> // String manipulációkhoz

void displayFloatWithUnit(uint8_t buffer[], float value, int decimals, const char* unit) {
  char output[16] = {0}; // Elég hely a float és az egység számára

  // Float konvertálása szöveggé a megadott tizedes pontossággal
  dtostrf(value, 1, decimals, output);

  // Mértékegység hozzáfűzése
  strcat(output, unit);

  // Karakterek kiírása a kijelzőre
  for (int i = 0, pos = 0; pos < 6; i++) {
    if (output[i] == '\0') {
      // Ha a string véget ér, az üres helyeket kikapcsoljuk
      setDisplayCharacter(buffer, pos, ' ');
      pos++;
    } else if (output[i] == '.') {
      // Ha pontot találunk, az előző kijelző pontját kapcsoljuk be
      if (pos > 0) {
        setDisplayDot(buffer, pos - 1, true);
      }
    } else {
      // Egyéb karakterek kiírása
      setDisplayCharacter(buffer, pos, output[i]);
      pos++;
    }
  }

  // Shift regiszter frissítése
  updateShiftRegisters(buffer, 6);
}

void displayString(uint8_t buffer[], const char* text) {
  int i = 0;

  // Karakterek kiírása a kijelzőkre a string hossza alapján
  while (i < 6 && text[i] != '\0') {
    setDisplayCharacter(buffer, i, text[i]);
    i++;
  }

  // Maradék kijelzők kikapcsolása
  while (i < 6) {
    setDisplayCharacter(buffer, i, ' ');
    i++;
  }

  // Shift regiszter frissítése
  updateShiftRegisters(buffer, 6);
}

void scrollText(uint8_t buffer[], const char* text, int delayMs) {
  int textLength = strlen(text);
  int scrollLength = textLength + 2; // Két szóköz a végére

  // Létrehozunk egy új stringet két szóközzel a végén
  char scrollingText[scrollLength + 1];
  snprintf(scrollingText, sizeof(scrollingText), "%s  ", text);

  // Futtatjuk a scroll animációt
  for (int offset = 0; offset < scrollLength; offset++) {
    // Minden kijelzőre kiírjuk az aktuális részt
    for (int i = 0; i < 6; i++) {
      int charIndex = offset + i - 5; // A kijelző indexének pozíciója a stringben
      if (charIndex >= 0 && charIndex < scrollLength) {
        setDisplayCharacter(buffer, 5 - i, scrollingText[charIndex]);
      } else {
        setDisplayCharacter(buffer, 5 - i, ' '); // Üres karakter, ha nincs adat
      }
    }

    // Shift regiszter frissítése
    updateShiftRegisters(buffer, 6);

    // Várakozás a léptetés előtt
    delay(delayMs);
  }
}

void displayDate(uint8_t buffer[], int year, int month, int day) {
  // Az év utolsó két számjegyének kiszámítása
  int yearLastTwo = year % 100;

  // Ellenőrizzük, hogy az év, hónap és nap érvényes-e
  if (year < 0 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31) {
    for (int i = 0; i < 6; i++) {
      buffer[i] = 0; // Kikapcsoljuk az összes kijelzőt
    }
    updateShiftRegisters(buffer, 6);
    return;
  }

  // Kijelzők tartalmának beállítása
  setDisplayCharacter(buffer, 0, '0' + (yearLastTwo / 10));   // Év utolsó két számjegye (tízes helyiérték)
  setDisplayCharacter(buffer, 1, '0' + (yearLastTwo % 10));   // Év utolsó két számjegye (egyes helyiérték)
  setDisplayDot(buffer, 1, true);
  setDisplayCharacter(buffer, 2, '0' + (month / 10));         // Hónap tízes helyiértéke
  setDisplayCharacter(buffer, 3, '0' + (month % 10));         // Hónap egyes helyiértéke
  setDisplayDot(buffer, 3, true);
  setDisplayCharacter(buffer, 4, '0' + (day / 10));           // Nap tízes helyiértéke
  setDisplayCharacter(buffer, 5, '0' + (day % 10));           // Nap egyes helyiértéke

  // Shift regiszter frissítése
  updateShiftRegisters(buffer, 6);
}

void displayTime(uint8_t buffer[], int hour, int minute, int second) {
  // Ellenőrizzük, hogy az idő érvényes-e
  if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
    for (int i = 0; i < 6; i++) {
      buffer[i] = 0; // Kikapcsoljuk az összes kijelzőt
    }
    updateShiftRegisters(buffer, 6);
    return;
  }

  // Kijelzők tartalmának beállítása
  setDisplayCharacter(buffer, 0, '0' + (hour / 10));     // Óra tízes helyiértéke
  setDisplayCharacter(buffer, 1, '0' + (hour % 10));     // Óra egyes helyiértéke
  setDisplayDot(buffer, 1, true);
  setDisplayCharacter(buffer, 2, '0' + (minute / 10));   // Perc tízes helyiértéke
  setDisplayCharacter(buffer, 3, '0' + (minute % 10));   // Perc egyes helyiértéke
  setDisplayDot(buffer, 3, true);
  setDisplayCharacter(buffer, 4, '0' + (second / 10));   // Másodperc tízes helyiértéke
  setDisplayCharacter(buffer, 5, '0' + (second % 10));   // Másodperc egyes helyiértéke

  // Shift regiszter frissítése
  updateShiftRegisters(buffer, 6);
}

void segmentTest(uint8_t buffer[], int size, int charCount){
  for(int i=0; i<8; i++){
    for(int digit=0; digit<size; digit++){
      buffer[digit] = 0;
      bitWrite(buffer[digit],i,1);
    }
    updateShiftRegisters(buffer, size);
    delay(1000);
  }
}

void dispTest(uint8_t buffer[], int size, int charCount){
  for(int c=0; c<charCount; c++){
    for(int digit=0; digit<size; digit++){
      buffer[digit] = charMap[c];
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
  updateShiftRegisters(displayBuffer, DISP_SIZE);
  delay(100);
}

int num = 0;

// Időállapotok
int hour = 20;
int minute = 54;
int second = 00;

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
  /*delay(1000);
  dispTest(displayBuffer, DISP_SIZE, CHAR_COUNT);*/

  segmentTest(displayBuffer, DISP_SIZE, CHAR_COUNT);
  //delay(2000);
  //displayInteger(displayBuffer, num++, true);
  /*displayDate(displayBuffer,2024,12,26);
  delay(2000);
  displayDate(displayBuffer,24,12,26);*/

 // Idő megjelenítése
 /* displayTime(displayBuffer, hour, minute, second);

  // Várakozás egy másodpercig
  delay(1000);

  // Idő léptetése
  second++;
  if (second >= 60) {
    second = 0;
    minute++;
    if (minute >= 60) {
      minute = 0;
      hour++;
      if (hour >= 24) {
        hour = 0;
      }
    }
  }*/
  /*displayFloatWithUnit(displayBuffer, 1.28, 1, " AcP");
  delay(5000);
  displayFloatWithUnit(displayBuffer, 15.08, 2, " c");
  delay(5000);
  displayFloatWithUnit(displayBuffer, 1001.08, 0, " P");
  delay(5000);*/

  /*displayString(displayBuffer, "ABC");
  delay(5000);
  displayString(displayBuffer,"DEF");
  delay(5000);
  displayString(displayBuffer,"ABCDEF");
  delay(5000);*/

  /*const char* text = "1234567890";
  scrollText(displayBuffer, text, 500); // 300 ms késleltetés minden lépésnél*/
}
