/*
   Project Hack4Fede
   Author hackAbility@PoliTo
   Modified 06/04/2019 by Andrea
   Arduino Nano
*/
/*Bug
  Lo spazio non cancella il buffer dei caratteri (penso sia risolto)
  Se arrivo in fondo allo schemo non mi fa più cancellare i caratteri e si creano casini
*/

// -----DICHIARAZIONI ED INIZIALIZZAZIONI----- //
/*Librerie*/
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

/*Costanti*/
#define D_MAXINPUTTIME 800
#define LENGLCD 20 //colonne dello schermo LCD
#define toneDuration 150 //durata dei toni eseguiti alla pressione dei tasti [ms]
#define R 37 //righe
#define C 7 //colonne
/*Strutture*/
enum State { // Stato del bottone
  UP = 0,
  DOWN = 1
} state;

/*Pinout*/
int buttonDot = 9;
int buttonLine = 8;
int buttonSpace = 7;
int buttonEndChar = 6;
int buttonCanc = 10;
int buzzerPin = 13;
int timePin = A7; //se non si utilizza, collegare a massa

/*Inizializzazone delle librerie*/
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address, Arduino pin A4->SDA  A5->SCL

/*Prototipi funzioni*/
void saluto(); //Funzione di avvio, saluta l'utente e fornisce istruzioni circa la modalità impiegata tramite la chiamata a timeTrigger
void myReadChar(); //Controlla che la sequenza di punti e linee inserita corrisponda effettivamente ad una lettera
void readDashDot(State LineState, State DotState); //Legge e stampa su lcd il punto o la linea
char readCharacter(); //Trasforma punti e linee in lettera, restituisce false se non trova la corrispondenza
void timeTrigger(); //Regola la difficoltà (tempo minimo di attesa tra la pressione dei tasti) tramite input da trimmer
void clearCharacter(); //Pulisce la stringa usata per memorizzare la sequenza di punti e linee che compongono una lettera
void clearlcdline(int line); //Cancella la linea di schermo passata come parametro
void initgame(); //Esegue il gioco contenuto all'interno del programma
void checkword(char g); //Verifica che il carattere inserito corrisponda a quello richiesto dal gioco

/*Dichirazione matrice contenente la lettera ed il corrispettivo in morse*/
const char CLEAR = 0;
const char DOT = 1;
const char DASH = 2;
const char HEART = 3;
const char HAPPY = 4;
const char SAD = 5;

const char alphabet[R][C] {
  { 'A', DOT, DASH, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'B', DASH, DOT, DOT, DOT, CLEAR, CLEAR},
  { 'C', DASH, DOT, DASH, DOT, CLEAR, CLEAR},
  { 'D', DASH, DOT, DOT, CLEAR, CLEAR, CLEAR},
  { 'E', DOT, CLEAR, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'F', DOT, DOT, DASH, DOT, CLEAR, CLEAR},
  { 'G', DASH, DASH, DOT, CLEAR, CLEAR, CLEAR},
  { 'H', DOT, DOT, DOT, DOT, CLEAR, CLEAR},
  { 'I', DOT, DOT, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'J', DOT, DASH, DASH, DASH, CLEAR, CLEAR},
  { 'K', DASH, DOT, DASH, CLEAR, CLEAR, CLEAR},
  { 'L', DOT, DASH, DOT, DOT, CLEAR, CLEAR},
  { 'M', DASH, DASH, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'N', DASH, DOT, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'O', DASH, DASH, DASH, CLEAR, CLEAR, CLEAR},
  { 'P', DOT, DASH, DASH, DOT, CLEAR, CLEAR},
  { 'Q', DASH, DASH, DOT, DASH, CLEAR, CLEAR},
  { 'R', DOT, DASH, DOT, CLEAR, CLEAR, CLEAR},
  { 'S', DOT, DOT, DOT, CLEAR, CLEAR, CLEAR},
  { 'T', DASH, CLEAR, CLEAR, CLEAR, CLEAR, CLEAR},
  { 'U', DOT, DOT, DASH, CLEAR, CLEAR, CLEAR},
  { 'V', DOT, DOT, DOT, DASH, CLEAR, CLEAR},
  { 'W', DOT, DASH, DASH, CLEAR, CLEAR, CLEAR},
  { 'X', DASH, DOT, DOT, DASH, CLEAR, CLEAR},
  { 'Y', DASH, DOT, DASH, DASH, CLEAR, CLEAR},
  { 'Z', DASH, DASH, DOT, DOT, CLEAR, CLEAR},
  { '0', DASH, DASH, DASH, DASH, DASH, CLEAR},
  { '1', DOT, DASH, DASH, DASH, DASH, CLEAR},
  { '2', DOT, DOT, DASH, DASH, DASH, CLEAR},
  { '3', DOT, DOT, DOT, DASH, DASH, CLEAR},
  { '4', DOT, DOT, DOT, DOT, DASH, CLEAR},
  { '5', DOT, DOT, DOT, DOT, DOT, CLEAR},
  { '6', DASH, DOT, DOT, DOT, DOT, CLEAR},
  { '7', DASH, DASH, DOT, DOT, DOT, CLEAR},
  { '8', DASH, DASH, DASH, DOT, DOT, CLEAR},
  { '9', DASH, DASH, DASH, DASH, DOT, CLEAR},
  { '?', DOT, DOT, DASH, DASH, DOT, DOT}
};

/*Definizione bit disegni*/
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

byte happy[8] = {
  B00000,
  B01010,
  B01010,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000
};

byte sad[8] = {
  B00000,
  B01010,
  B01010,
  B00000,
  B00000,
  B01110,
  B10001,
  B00000
};

/*Variabili*/
int counter = 0; // contatore utilizzato per la posizione della lettera nello schermo LCD
int cnt = 0; // sets the LCD screen and dot/line sign

//Doppio click del tasto fine carattere/cancella
int count = 0;
unsigned long duration = 0, lastPress = 0;
bool oneClick = true;

bool isReadingChar = false, nextRead = true;
char character[C - 1]; // dash-dot-sequence of the current character
int characterIndex; // index of the next dot/dash in the current character

//varabili timeTrigger
int maxinputtime = D_MAXINPUTTIME;
int attesa = D_MAXINPUTTIME * 2;

//variabili sezione gioco
bool gamemode = false;
int r;
char gamechar;
int life = 0;


// --------SETUP----- //
void setup() {
  Serial.begin(9600);

  lcd.begin(LENGLCD, 4);
  lcd.createChar(HEART, heart);
  lcd.createChar(DASH, dash);
  lcd.createChar(DOT, dot);
  lcd.createChar(CLEAR, clear);
  lcd.createChar(HAPPY, happy);
  lcd.createChar(SAD, sad);

  pinMode(buttonDot, INPUT_PULLUP);
  pinMode(buttonLine, INPUT_PULLUP);
  pinMode(buttonSpace, INPUT_PULLUP);
  pinMode(buttonCanc, INPUT_PULLUP);
  pinMode(buttonEndChar, INPUT_PULLUP);

  analogReference(INTERNAL);
  randomSeed(analogRead(A0));
  lcd.backlight();
  delay(100);

  saluto();
}

// --------LOOP----- //
void loop() {

  State DotState = digitalRead(buttonDot) ? UP : DOWN;
  State LineState = digitalRead(buttonLine) ? UP : DOWN;
  State GameState = digitalRead(buttonCanc) ? UP : DOWN;
  State SpaceState = digitalRead(buttonSpace) ? UP : DOWN;
  State EndCharState = digitalRead(buttonEndChar) ? UP : DOWN;

  // if the button is pressed, play a tone
  if (DotState == DOWN && nextRead) {
    tone(buzzerPin, 440, toneDuration);
  }
  if (LineState == DOWN && nextRead) {
    tone(buzzerPin, 440, toneDuration);
  }

  if (DotState == DOWN || LineState == DOWN) { // leggo i punti e linee
    lastPress = millis();
    readDashDot(LineState, DotState);
  } else if (EndCharState == DOWN && oneClick && (millis() - lastPress) > maxinputtime) {
    lastPress = millis();
    oneClick = false;
    if (count++ == 0) {
      nextRead = false;
      duration = millis() + attesa;
    }
    lcd.setCursor(0, 3);
    if (count == 1) {
      tone(buzzerPin, 262, toneDuration);
      lcd.print("Un click");
    } else if (count == 2) {
      tone(buzzerPin, 294, toneDuration);
      lcd.print("Doppio click");
    }
  } else if (EndCharState == UP && !oneClick) oneClick = true;

  if (count > 0 && millis() >= duration) {
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

    } else if (!gamemode) { //cancella carattere
      lcd.setCursor(--counter, 0);
      if (counter < 0)
        counter = 0;
      lcd.print(' ');
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
    if (!nextRead) return;
    nextRead = false;
    //Serial.println(isReadingChar);
    if (isReadingChar) {
      myReadChar();
    }
    clearlcdline(1);
    lcd.setCursor(0, 1);
    lcd.print("Spazio");
    lcd.setCursor(counter++, 0);
    ///////////////////////////////// Aggiunta mia personale, riparte dall'inizio della riga se finisce lo spazio sullo schermo e elimina il buffer dei . - se metti spazio
    clearCharacter();
    if (counter > LENGLCD) {
      clearlcdline(0);
      lcd.setCursor(0, 0);
      counter = 1;
    }
    ///////////////////////////////
    lcd.print(' ');
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
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MODALITA' SCRITTURA");
      delay(1000);
      clearlcdline(0);
    } else {
      gamemode = true;
      initgame();
    }
  }

  if (DotState == UP && LineState == UP && SpaceState == UP && GameState == UP && EndCharState == UP && !nextRead && count == 0 && ((millis() - lastPress) > maxinputtime ) && cnt <= C - 1) nextRead = true;
}


// -----FUNZIONI----- //

/*Funzione di avvio, saluta l'utente e fornisce istruzioni circa la modalità impiegata tramite la chiamata a timeTrigger*/
void saluto() {
  lcd.setCursor(0, 0);
  lcd.print("Ciao, sono MORSY!");
  delay(2000);
  lcd.clear();
  timeTrigger();
  lcd.setCursor(0, 0);
  lcd.print("Ora sono pronto per");
  lcd.setCursor(0, 1);
  lcd.print("essere utilizzato!");
  delay(3500);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("MODALITA' SCRITTURA");
  delay(2500);
  lcd.clear();
}

/* Acquisisce la lettera ed effettua un controllo sulla sua validità */ //modificare la descrizione
void myReadChar() {
  if (!nextRead) return;
  nextRead = false;
  char c = readCharacter();

  if (c != 0) {
    if (!gamemode) {
      lcd.setCursor(counter++, 0);
      ///////////////////////////////// Aggiunta mia personale, riparte dall'inizio della riga se finisce lo spazio sullo schermo
      if (counter > LENGLCD) {
        clearlcdline(0);
        lcd.setCursor(0, 0);
        counter = 1;
      }
      ///////////////////////////////
      lcd.print(c);
    } else checkword(c);
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

/*Legge e stampa su lcd il punto o la linea*/
void readDashDot(State LineState, State DotState) {
  if (!nextRead || characterIndex >= C - 1) return;
  else {
    isReadingChar = true;
    nextRead = false;
    if (LineState == DOWN) {
      character[characterIndex] = DASH;
      //Serial.println("LINEA");
      clearlcdline(1);
      lcd.setCursor(0, 1);
      lcd.print("LINEA");
    } else if (DotState == DOWN) {
      character[characterIndex] = DOT;
      //Serial.println("PUNTO");
      clearlcdline(1);
      lcd.setCursor(0, 1);
      lcd.print("PUNTO");
    }
    lcd.setCursor(cnt, 2);
    lcd.write(character[characterIndex++]);
    cnt++;
  }
}

/*Trasforma punti e linee in lettera, restituisce false se non trova la corrispondenza*/ //da editare
char readCharacter() {
  bool found;
  for (int i = 0; i < R; ++i) {
    found = true;
    for (int j = 0; found && j < C - 1; ++j) {
      if (character[j] != alphabet[i][j + 1]) {
        found = false;
      }
    }
    if (found) return alphabet[i][0];
  }
  return 0;
}

/*Regola la difficoltà (tempo minimo di attesa tra la pressione dei tasti) tramite input da trimmer*/
void timeTrigger() {
  int reg = analogRead(timePin) + 20;

  lcd.setCursor(0, 0);
  lcd.print("MODO");
  lcd.setCursor(10, 0);
  lcd.print("Base");
  lcd.setCursor(10, 1);
  lcd.print("Normale");
  lcd.setCursor(10, 2);
  lcd.print("Esperto");
  lcd.setCursor(10, 3);
  lcd.print("Pro");

  if (reg > 19) {
    if (reg <= 270) {
      maxinputtime = D_MAXINPUTTIME * 2;
      attesa = maxinputtime * 2;
      //Serial.println("Modalità Base");
      lcd.setCursor(18, 0);
      lcd.print("<-");
    } else if (reg <= 520) {
      maxinputtime = D_MAXINPUTTIME;
      attesa = maxinputtime * 2;
      //Serial.println("Modalità Normale");
      lcd.setCursor(18, 1);
      lcd.print("<-");
    } else if (reg <= 770) {
      maxinputtime = D_MAXINPUTTIME / 2;
      attesa = maxinputtime * 2;
      //Serial.println("Modalità Esperto");
      lcd.setCursor(18, 2);
      lcd.print("<-");
    } else if (reg > 770) {
      maxinputtime = D_MAXINPUTTIME / 6;
      attesa = maxinputtime * 3;
      //Serial.println("Modalità Pro");
      lcd.setCursor(18, 3);
      lcd.print("<-");
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERRORE MODO'");
  }
  delay(2500);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Per modificare il");
  lcd.setCursor(0, 1);
  lcd.print("modo ruota la");
  lcd.setCursor(0, 2);
  lcd.print("manopola, se esiste,");
  lcd.setCursor(0, 3);
  lcd.print("e riavviami.");
  delay(5000);
  lcd.clear();
}

/*Pulisce la stringa usata per memorizzare la sequenza di punti e linee che compongono una lettera*/
void clearCharacter() {
  characterIndex = 0;
  for (int i = 0; i < C - 1; ++i) {
    character[i] = CLEAR;
  }
}

/*Cancella la linea di schermo passata come parametro*/
void clearlcdline(int line) {
  int i;
  for (i = 0; i < LENGLCD; i++) {
    lcd.setCursor(i, line);
    lcd.print(" ");
  }
}

// -----FUNZIONI MODALITA' GIOCO----- //

/*Inizializzazione modalità gioco*/
void initgame() {
  r = random(R);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MODALITA' GIOCO");
  delay(1000);
  clearlcdline(0);
  gamechar = alphabet[r][0];
  lcd.setCursor(0, 0);
  lcd.print("Traduci in Morse: ");
  lcd.print(gamechar);
  lcd.setCursor(17, 3);
  lcd.write(HEART); lcd.write(HEART); lcd.write(HEART);
  life = 3;
}

/*Verifica che il carattere inserito corrisponda a quello richiesto dal gioco*/
void checkword(char g) {
  int thisNote, noteDuration, pauseBetweenNotes, i;
  /*Array toni e musichette*/
  int melodyWin[] = {462, 396, 396, 420, 396, 0, 447, 462};
  int melodyFail[] = {494, 0, 480, 0, 461, 0, 600, 0};
  // durata note: 4 = quarter note, 8 = eighth note, etc.
  int noteDurationsWin[] = {4, 8, 8, 4, 4, 4, 4, 4 };
  int noteDurationsFail[] = {4, 16, 4, 16, 4, 16, 2, 4};

  lcd.setCursor(0, 3);
  lcd.print("              ");
  if (g == gamechar || life == 1) {
    clearlcdline(1); clearlcdline(2); clearlcdline(3);
    lcd.setCursor(0, 1);
    if (life > 1) {
      lcd.print("HAI VINTO ");
      lcd.print(HAPPY);
      for (thisNote = 0; thisNote < 8; thisNote++) {
        noteDuration = 1000 / noteDurationsWin[thisNote];
        tone(buzzerPin, melodyWin[thisNote], noteDuration);
        pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(buzzerPin);
      }
    }
    else {
      lcd.print("HAI PERSO ");
      lcd.print(SAD);
      for (thisNote = 0; thisNote < 8; thisNote++) {
        noteDuration = 1000 / noteDurationsFail[thisNote];
        tone(buzzerPin, melodyFail[thisNote], noteDuration);
        pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(buzzerPin);
      }
    }
    lcd.setCursor(0, 2);
    lcd.print("Corretta: ");
    for (i = 1; i < C; i++) lcd.write(alphabet[r][i]);
    delay(5000);
    initgame();

  } else {
    clearlcdline(1);
    lcd.setCursor(0, 1);
    lcd.print("Hai: ");
    lcd.write(0x30 + --life);
    lcd.print(" tentativi!");
    lcd.setCursor(17 + (life), 3);
    lcd.print(' ');
  }
}
