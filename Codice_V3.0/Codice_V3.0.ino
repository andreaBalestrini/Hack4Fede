/*ARDUINO NANO OLD BOOT*/

/*Librerie*/
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

/*Costanti*/
#define MAXINPUTTIME 300
#define ATTESA 2000

/*Strutture*/
enum State{ // Stato del bottone
  UP = 0,
  DOWN = 1
}state;

/*Prototipi funzioni*/
void checkword(char g);
void clearlcdline(int line);
void saluto(int flag);
void readDashDot(State LineState, State DotState);
char readCharacter();

/*Inizializzazone delle librerie*/
SoftwareSerial BTserial(0, 1); // RX | TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


int LENGLCD = 0;
int counter = 0; // contatore utilizzato per la posizione della lettera
int cnt = 0; // sets the LCD screen and dot/line sign
int flag = 0; // usato per la funzione di saluto iniziale

const char CLEAR = 0;
const char DOT = 1;
const char DASH = 2;
char string_to_send[20];

/////////////////////////////////////////////////////////////////////
//Doppio click del tasto fine carattere/cancella
int count = 0;
unsigned long duration = 0, lastPress = 0;
bool oneClick = true;
///////////////////////////////////////////////////////////////////////

//variabili sezione gioco
bool gamemode = false;
char gamechar;
int life = 0;

/*Definizione bit disegni*/
byte heart[8] = {
  B00000,
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

byte dash[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

byte dot[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B01110,
  B00100,
  B00000,
  B00000
};

byte clear[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

/*Array toni musichette*/
int melodyWin[] = {462, 396, 396, 420, 396, 0, 447, 462};
int melodyFail[] = {494, 0, 480, 0, 461, 0, 600, 0};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurationsWin[] = {4, 8, 8, 4, 4, 4, 4, 4 };
int noteDurationsFail[] = {4, 16, 4, 16, 4, 16, 2, 4};
//////////////////////////////////////////////////////////////////////

/*Dichirazione array contenente la lettera ed il corrispettivo in morse*/
const char alphabet[26][6]{
  { 'A', DOT, DASH, CLEAR, CLEAR, CLEAR},
  { 'B', DASH, DOT, DOT, DOT, CLEAR},
  { 'C', DASH, DOT, DASH, DOT, CLEAR},
  { 'D', DASH, DOT, DOT, CLEAR, CLEAR},
  { 'E', DOT, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'F', DOT, DOT, DASH, DOT, CLEAR},
  { 'G', DASH, DASH, DOT, CLEAR, CLEAR},
  { 'H', DOT, DOT, DOT, DOT, CLEAR},
  { 'I', DOT, DOT, CLEAR, CLEAR, CLEAR},
  { 'J', DOT, DASH, DASH, DASH, CLEAR},
  { 'K', DASH, DOT, DASH, CLEAR, CLEAR},
  { 'L', DOT, DASH, DOT, DOT, CLEAR},
  { 'M', DASH, DASH, CLEAR, CLEAR, CLEAR},
  { 'N', DASH, DOT, CLEAR, CLEAR, CLEAR},
  { 'O', DASH, DASH, DASH, CLEAR, CLEAR},
  { 'P', DOT, DASH, DASH, DOT, CLEAR},
  { 'Q', DASH, DASH, DOT, DASH, CLEAR},
  { 'R', DOT, DASH, DOT, CLEAR, CLEAR},
  { 'S', DOT, DOT, DOT, CLEAR, CLEAR},
  { 'T', DASH, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'U', DOT, DOT, DASH, CLEAR, CLEAR},
  { 'V', DOT, DOT, DOT, DASH, CLEAR},
  { 'W', DOT, DASH, DASH, CLEAR, CLEAR},
  { 'X', DASH, DOT, DOT, DASH, CLEAR},
  { 'Y', DASH, DOT, DASH, DASH, CLEAR},
  { 'Z', DASH, DASH, DOT, DOT, CLEAR}
};


bool isReadingChar = false, nextRead = true;

char character[5]; // dash-dot-sequence of the current character

int characterIndex; // index of the next dot/dash in the current character

// pin to which the button is connected
int buttonDot = 9;
int buttonLine = 8;
int buttonSpace = 7;
int buttonEndChar = 6;
int buttonCanc = 10;
// pin to which the buzzer is connected
int buzzerPin = A4;


//--------SETUP-----//
void setup(){
  Serial.begin(9600);
  BTserial.begin(9600);

  lcd.createChar(4, heart);
  lcd.createChar(DASH, dash);
  lcd.createChar(DOT, dot);
  lcd.createChar(CLEAR, clear);
  lcd.begin(20, 4);

  pinMode(buttonDot, INPUT_PULLUP);
  pinMode(buttonLine, INPUT_PULLUP);
  pinMode(buttonCanc, INPUT_PULLUP);
  pinMode(buttonSpace, INPUT_PULLUP);
  pinMode(buttonEndChar, INPUT_PULLUP);

  randomSeed(analogRead(0));
}

//--------LOOP-----//
void loop(){
  saluto(flag);
  flag = 1;

  State DotState = digitalRead(buttonDot) ? UP : DOWN;
  State LineState = digitalRead(buttonLine) ? UP : DOWN;
  State GameState = digitalRead(buttonCanc) ? UP : DOWN;
  State SpaceState = digitalRead(buttonSpace) ? UP : DOWN;
  State EndCharState = digitalRead(buttonEndChar) ? UP : DOWN;

  // if the button is pressed, play a tone
  if(DotState == DOWN && nextRead){
    tone(buzzerPin, 3000, 200);
  }
  if(LineState == DOWN && nextRead){
    tone(buzzerPin, 2900, 200);
  }

  if(DotState == DOWN || LineState == DOWN){ // leggo i punti e linee
    lastPress = millis();
    readDashDot(LineState, DotState);
  }

  else if(EndCharState == DOWN && oneClick && (millis() - lastPress) > MAXINPUTTIME){
    lastPress = millis();
    oneClick = false;
    if (count++ == 0) {
      nextRead = false;
      duration = millis() + ATTESA;
    }
    lcd.setCursor(0, 3);
    if(count == 1){
      tone(buzzerPin, 200, 200);
      lcd.print("Un click");
    }
    else if (count == 2){
      tone(buzzerPin, 1200, 200);
      lcd.print("Doppio click");
    }
  }
  else if (EndCharState == UP && !oneClick) {
    oneClick = true;
  }

  if(count > 0 && millis() >= duration){
    if (count == 1) { //fine carattere
      nextRead = true;
      myReadChar();

    } else if (isReadingChar) { //cancella buffer
      clearCharacter();
      cnt = 0;
      clearlcdline(1);
      clearlcdline(2);
      lcd.setCursor(0, 1);
      lcd.print("Reinserire carattere");
      isReadingChar = false;

    } else if (!gamemode) { //cancela carattere
      lcd.setCursor(--counter, 0);
      if (counter < 0)
        counter = 0;
      lcd.print(' ');
      string_to_send[counter] = ' ';
      clearlcdline(1);
      lcd.setCursor(0, 1);
      lcd.print("Lettera cancellata");
      clearlcdline(2);
      cnt = 0;
    }
    count = 0;
    lcd.setCursor(0, 3);
    lcd.print("              ");
  }
  //////////////////////////////////////////////////////////////////////////

  else if (SpaceState == DOWN && !gamemode) { // stampo a video lo spazio
    //leggo carattere se necessario
    lastPress = millis();
    if (!nextRead)
      return;
    nextRead = false;
    if (isReadingChar) {
      myReadChar();
    }
    clearlcdline(1);
    lcd.setCursor(0, 1);
    lcd.print("Spazio");
    lcd.setCursor(counter++, 0);
    lcd.print(' ');
    string_to_send[counter] = ' ';
    BTserial.print(string_to_send);
    clearlcdline(2);
    cnt = 0;
  }

  else if (GameState == DOWN) {
    lastPress = millis();
    if (!nextRead)
      return;
    nextRead = false;
    if (gamemode) {
      //esco da game mode
      gamemode = false;
      counter = 0;
      clearlcdline(0); clearlcdline(1); clearlcdline(2); clearlcdline(3);
      lcd.setCursor(0, 0);
      lcd.print("MODALITA' SCRITTURA");
      delay(1000);
      clearlcdline(0);
    } else {
      gamemode = true;
      initgame();
    }
  }

  if (DotState == UP && LineState == UP && SpaceState == UP && GameState == UP && EndCharState == UP && !nextRead && count == 0 && ((millis() - lastPress) > MAXINPUTTIME ) && cnt <= 4)
    nextRead = true;
}

// -----FUNZIONI----- //

//// FUNZIONE SALUTO INIZIALE ////
void saluto(int flag){
  if (flag == 0) {
    lcd.setCursor(0, 0);
    lcd.print("CIAO");
    lcd.setCursor(0, 1);
    lcd.print("SONO PRONTO PER");
    lcd.setCursor(0, 2);
    lcd.print("ESSERE UTILIZZATO!!");
    delay(2000);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("MODALITA' SCRITTURA");
    delay(1000);
    clearlcdline(0);
  }
}

///////////////////////////////////////////////////////////////////////

//// CANCELLA LA LINEA DELLO SCHERMO CHE E' STATA PASSATA ALLA FUNZIONE ////

void clearlcdline(int line){
  lcd.setCursor(0, line);
  lcd.print("                    ");
}

///////////////////////////////////////////////////////////////////////

//// AZZERA LA SEQUENZA DI CARATTERI PUNTO-LINEA ////

void clearCharacter(){
  characterIndex = 0;
  for (int i = 0; i < 5; ++i) {
    character[i] = CLEAR;
  }
}
/////////////////////////////////////////////////////////////////////////

//// LEGGE SE SI TRATTA DI PUNTO O LINEA ////
void readDashDot(State LineState, State DotState){
  if (!nextRead || characterIndex >= 5){}
  else{
    isReadingChar = true;
    nextRead = false;
    if (LineState == DOWN) {
      character[characterIndex] = DASH;
      Serial.println("LINEA");
      clearlcdline(1);
      lcd.setCursor(0, 1);
      lcd.print("LINEA");
  
    } else if (DotState == DOWN) {
      character[characterIndex] = DOT;
      Serial.println("PUNTO");
      clearlcdline(1);
      lcd.setCursor(0, 1);
      lcd.print("PUNTO");
    }
  
    lcd.setCursor(cnt, 2);
    lcd.write(character[characterIndex++]);
    cnt++;
  }
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//// TRASFORMA L'INPUT DI PUNTI E LINEE IN TESTO ////
char readCharacter(){
  bool found;
  for (int i = 0; i < 26; ++i) {
    found = true;
    for (int j = 0; found && j < 5; ++j) {
      if (character[j] != alphabet[i][j + 1]) {
        found = false;
      }
    }
    if (found) {
      return alphabet[i][0];
    }
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

//// ACQUISISCE LA LETTERA ED EFFETTUA UN CONTROLLO SULLA SUA VALIDITA' ////
void myReadChar() {
  if (!nextRead)
    return;
  nextRead = false;
  char c = readCharacter();

  if (c != 0) {
    if (!gamemode) {
      lcd.setCursor(counter++, 0);
      lcd.print(c);
      string_to_send[counter] = c;
    } else
      checkword(c);
    cnt = 0;
    clearlcdline(2);

  } else {
    clearlcdline(1);
    lcd.setCursor(0, 1);
    lcd.print("Lettera non valida");
    cnt = 0;
    clearlcdline(2);

  }
  isReadingChar = false;
  clearCharacter();
}


///////////////////// FUNZIONI MOD GIOCO //////////////////////

//// INIZIALIZZAZIONE GIOCO ////
void initgame() {

  clearlcdline(0); clearlcdline(1); clearlcdline(2); clearlcdline(3);

  lcd.setCursor(0, 0);
  lcd.print("MODALITA' GIOCO");
  delay(1000);
  clearlcdline(0);

  gamechar = random(65, 91);
  lcd.setCursor(0, 0);
  lcd.print("Scrivi la lettera: ");
  lcd.print(gamechar);
  lcd.setCursor(17, 3);
  lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));
  life = 3;
}
//// VERIFICA PAROLA ////
void checkword(char g) {
  lcd.setCursor(0, 3);
  lcd.print("              ");
  if (g == gamechar || life == 1) {
    clearlcdline(1); clearlcdline(2); clearlcdline(3);
    lcd.setCursor(0, 1);
    if (life > 1) {
      lcd.print("HAI VINTO");
      for (int thisNote = 0; thisNote < 8; thisNote++) {
        int noteDuration = 1000 / noteDurationsWin[thisNote];
        tone(buzzerPin, melodyWin[thisNote], noteDuration);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(buzzerPin);
      }
    }
    else {
      lcd.print("HAI PERSO");
      for (int thisNote = 0; thisNote < 8; thisNote++) {
        int noteDuration = 1000 / noteDurationsFail[thisNote];
        tone(buzzerPin, melodyFail[thisNote], noteDuration);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(buzzerPin);
      }
    }
    lcd.setCursor(0, 2);
    lcd.print("Corretta: ");
    for (int i = 1; i < 6; i++)
      lcd.write(alphabet[gamechar - (char)'A'][i]);
    delay(5000);
    initgame();

  } else {
    clearlcdline(1);
    lcd.setCursor(0, 1);
    lcd.print("Hai: ");
    lcd.write(0x30 + --life);
    lcd.print(" tentativi!!");
    lcd.setCursor(17 + (life), 3);
    lcd.print(' ');
  }
}
