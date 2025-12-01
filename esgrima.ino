#include <LiquidCrystal.h>

/**
 * =================================================================================
 * PROJECT: Reflex Master PRO v2.0
 * DESCRIPTION: Arduino-based reflex training game for fencing.
 * AUTHOR: Márcio Menezes
 * DATE: 2025
 * =================================================================================
 */

// ===================== HARDWARE CONFIGURATION =====================

// --- LCD Display (16x2) ---
// The LCD is used to show the menu, score, and game messages.
// Connection: RS=12, E=11, D4=5, D5=4, D6=3, D7=2
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// --- Joystick ---
// The joystick controls menu navigation.
const int joyXPin = A0; // X-Axis (Left/Right) - Used to adjust values
const int joyYPin = A1; // Y-Axis (Up/Down) - Used to navigate options
const int joySwitch =
    7; // Joystick Button (Digital Pin 7) - Used to select/cancel

// --- Sound ---
// Speaker for audio feedback (beeps, game sounds).
const int speakerPin = 10; // Speaker + Resistor

// --- GAME: 3 Buttons and 3 LEDs ---
// These are the game targets. The LED lights up and the player must press the
// corresponding button. Note: LEDs are connected to Analog pins (A2-A4) used as
// digital outputs.
const int ledPins[] = {A2, A3, A4}; // LED Left (A2), Center (A3), Right (A4)
const int btnPins[] = {6, 8, 9};    // Button Left (6), Center (8), Right (9)
const int numTargets = 3;           // Total number of targets

// ===================== GAME VARIABLES =====================

int totalRounds = 10; // Number of rounds per game (Can be changed in the menu)
int gameState = 0;    // Current game state:
                      // 0 = Main Menu
                      // 1 = Preparation (En Garde)
                      // 2 = Action (Waiting for reflex)
                      // 3 = Round Result (Shows time)
                      // 4 = Game Over (Shows total)
                      // 5 = High Score (Hall of Fame)

int menuOption = 0;      // Selected menu option: 0=Play, 1=Config, 2=Record
int lastMenuOption = -1; // Auxiliary variable to avoid unnecessary menu redraws
int currentRound = 1;    // Current round counter
unsigned long startTime; // Timestamp when the LED lit up
unsigned long reactionTime;    // Stores the time the player took to react
int roundScore = 0;            // Score of the current round
long totalGameScore = 0;       // Total game score
long highScore = 0;            // Best score recorded (resets on power cycle)
unsigned long lastNavTime = 0; // To control menu navigation speed (debounce)
int currentTarget = 0; // Index of the active target (0=Left, 1=Center, 2=Right)

// --- Custom Icons (Pixel Art for LCD) ---
// Definition of 5x8 bitmaps to create special characters on the LCD.
byte iconSword[8] = {B00100, B00100, B00100, B01110,
                     B01110, B00100, B01110, B00100}; // Sword Icon
byte iconTrophy[8] = {B11111, B10001, B10001, B01010,
                      B00100, B00100, B01110, B00000}; // Trophy Icon
byte iconSelect[8] = {B10000, B11000, B11100, B11110,
                      B11100, B11000, B10000, B00000}; // Selection Arrow Icon
byte iconClock[8] = {B00000, B01110, B10101, B10101,
                     B10111, B10001, B01110, B00000}; // Clock Icon

// ===================== FUNCTION PROTOTYPES =====================
// Forward declaration of functions to ensure the compiler recognizes them.
void handleMenu();
void startNewGame();
void showHighScore();
void runEnGarde();
void checkReflexes();
void processRoundResult(boolean success);
void introAnimation();
void drawMenu();
void cancelGame();
void gameOver();

// ===================== SETUP (Start) =====================
// This function runs only once when the Arduino powers up.
void setup() {
  // Initialize the LCD with 16 columns and 2 rows
  lcd.begin(16, 2);

  // Configure Pins
  pinMode(joySwitch, INPUT_PULLUP); // Joystick button with internal pull-up
  pinMode(speakerPin, OUTPUT);      // Speaker as output

  // Configure LED and Button pins
  for (int i = 0; i < numTargets; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(btnPins[i],
            INPUT_PULLUP); // Buttons with pull-up (LOW when pressed)
  }

  // Initialize Random Seed
  // Reads a disconnected analog pin (A5) to generate noise and ensure
  // randomness
  randomSeed(analogRead(A5));

  // Load Icons into LCD memory
  lcd.createChar(0, iconSword);
  lcd.createChar(1, iconTrophy);
  lcd.createChar(2, iconSelect);
  lcd.createChar(3, iconClock);

  // Show Intro Animation and Menu
  introAnimation();
  drawMenu();
}

// ===================== MAIN LOOP =====================
// This function runs continuously.
void loop() {

  // The game logic is based on a "State Machine".
  // Depending on the value of 'gameState', the code executes different actions.

  // --- STATE 0: MENU ---
  if (gameState == 0) {
    handleMenu(); // Manages menu navigation
  }

  // --- STATE 1: PREPARATION (En Garde) ---
  else if (gameState == 1) {
    runEnGarde(); // Prepares the round (random wait)
  }

  // --- STATE 2: GAME (Reflexes) ---
  else if (gameState == 2) {
    checkReflexes(); // Waits for the player to press a button
  }

  // --- STATE 3: ROUND RESULT ---
  else if (gameState == 3) {
    delay(800); // Quick pause to see the result

    // Turn off all LEDs
    for (int i = 0; i < numTargets; i++)
      digitalWrite(ledPins[i], LOW);

    // Check if the game continues or ends
    if (currentRound < totalRounds) {
      currentRound++;
      gameState = 1; // Returns to preparation state for the next round
    } else {
      gameOver(); // Game over, show final score
    }
  }

  // --- STATE 4 & 5: GAME OVER / HIGH SCORE ---
  else if (gameState == 4 || gameState == 5) {
    // In these states, wait for the player to press a button to return to the
    // menu. Exit with joystick button or center game button
    if (digitalRead(joySwitch) == LOW || digitalRead(btnPins[1]) == LOW) {
      gameState = 0;              // Return to Menu
      menuOption = 0;             // Reset selection
      lastMenuOption = -1;        // Force redraw
      tone(speakerPin, 600, 100); // Confirmation sound
      drawMenu();
      delay(500); // Debounce
    }
  }
}

// ===================== GAME FUNCTIONS =====================

// Starts a new game, resetting variables
void startNewGame() {
  currentRound = 1;
  totalGameScore = 0;
  gameState = 1; // Switches to preparation state
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("PREPARAR");
  lcd.setCursor(4, 1);
  lcd.print("COMBATE!");
  delay(1000);
}

// Preparation Phase ("En Garde")
// Shows round info and waits a random time before lighting the LED
void runEnGarde() {
  // Update LCD with round and score
  lcd.clear();
  lcd.print("Rnd ");
  lcd.print(currentRound);
  lcd.print("/");
  lcd.print(totalRounds);
  lcd.setCursor(9, 0);
  lcd.print("Pts:");
  lcd.print(totalGameScore);
  lcd.setCursor(0, 1);
  lcd.print("EN GARDE...");

  // Ensure LEDs are off
  for (int i = 0; i < numTargets; i++)
    digitalWrite(ledPins[i], LOW);

  // Safety: Wait for the player to release all buttons before starting
  // This prevents the player from holding the button to cheat
  boolean buttonsFree = false;
  while (!buttonsFree) {
    buttonsFree = true;
    for (int i = 0; i < numTargets; i++) {
      if (digitalRead(btnPins[i]) == LOW)
        buttonsFree = false;
    }
    delay(10);
  }
  delay(200);

  // Random wait time (between 0.8s and 3.0s) to create tension
  int randomDelay = random(800, 3000);
  unsigned long delayStart = millis();

  while (millis() - delayStart < randomDelay) {
    // 1. Check for Cheat (False Start)
    // If pressed before time, lose points!
    for (int i = 0; i < numTargets; i++) {
      if (digitalRead(btnPins[i]) == LOW) {
        tone(speakerPin, 100, 400); // Severe error sound
        lcd.setCursor(0, 1);
        lcd.print("  FALTA! -500   ");
        totalGameScore -= 500;
        delay(1500);
        return; // Exit function and restart round (gameState remains 1)
      }
    }
    // 2. Check for Cancellation (Joystick Click)
    // Allows exiting the game mid-way
    if (digitalRead(joySwitch) == LOW) {
      cancelGame();
      return;
    }
  }

  // --- LIGHT THE TARGET! ---
  // Wait time is over, action starts now
  gameState = 2;                // Switch to Game state
  currentTarget = random(0, 3); // Randomly choose: 0, 1, or 2

  // Show visual indication on LCD
  lcd.setCursor(0, 1);
  if (currentTarget == 0)
    lcd.print("ESQUERDA! <     ");
  if (currentTarget == 1)
    lcd.print(" CENTRO!  ^     ");
  if (currentTarget == 2)
    lcd.print("DIREITA!  >     ");

  // Light up corresponding LED and play sound
  digitalWrite(ledPins[currentTarget], HIGH);
  tone(speakerPin, 1500, 150);
  startTime = millis(); // Mark start time to measure reflex
}

// Game Phase: Checks if the player pressed the button
void checkReflexes() {
  for (int i = 0; i < numTargets; i++) {
    // If any button is pressed...
    if (digitalRead(btnPins[i]) == LOW) {
      reactionTime = millis() - startTime;       // Calculate reaction time
      digitalWrite(ledPins[currentTarget], LOW); // Turn off LED immediately

      if (i == currentTarget)
        processRoundResult(true); // Hit the correct button
      else
        processRoundResult(false); // Pressed the wrong button
    }
  }
}

// Process round result (Score and Feedback)
void processRoundResult(boolean success) {
  lcd.clear();
  if (success) {
    // Score Calculation: Base 1000 minus time in ms.
    // Example: 200ms reaction = 800 points.
    if (reactionTime > 1000)
      roundScore = 0; // Too slow (more than 1s) gives 0 points
    else
      roundScore = 1000 - reactionTime;

    // Audio Feedback
    if (roundScore > 800) {
      // Special sound for very fast reactions (<200ms)
      tone(speakerPin, 1000, 80);
      delay(80);
      tone(speakerPin, 2000, 150);
    } else {
      // Normal hit sound
      tone(speakerPin, 800, 100);
    }

    lcd.setCursor(0, 0);
    lcd.print("TOP! ");
    lcd.print(reactionTime);
    lcd.print("ms");
  } else {
    // Missed the button
    roundScore = -200;          // Penalty
    tone(speakerPin, 100, 300); // Error sound
    lcd.setCursor(0, 0);
    lcd.print("ERRADO!");
  }

  // Update total score
  totalGameScore += roundScore;
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(totalGameScore);

  gameState = 3; // Advance to Result state
}

// ===================== MENU AND NAVIGATION =====================

// Manages Main Menu logic
void handleMenu() {
  int joyY = analogRead(joyYPin);
  int joyX = analogRead(joyXPin);

  // Vertical Navigation (Up/Down) with delay to prevent being too fast
  if (millis() - lastNavTime > 200) {
    // Move Up
    if (joyY < 100 && menuOption > 0) {
      menuOption--;
      tone(speakerPin, 1000, 20); // Navigation beep
      lastNavTime = millis();
      drawMenu();
    }
    // Move Down
    if (joyY > 900 && menuOption < 2) {
      menuOption++;
      tone(speakerPin, 1000, 20);
      lastNavTime = millis();
      drawMenu();
    }

    // Round Configuration (Left/Right) - Only works if on Option 1 (Rounds)
    if (menuOption == 1) {
      if (joyX > 900) { // Right: Increase rounds
        totalRounds++;
        if (totalRounds > 99)
          totalRounds = 99; // Max limit
        tone(speakerPin, 1200, 20);
        lastNavTime = millis();
        drawMenu();
      }
      if (joyX < 100) { // Left: Decrease rounds
        totalRounds--;
        if (totalRounds < 1)
          totalRounds = 1; // Min limit
        tone(speakerPin, 800, 20);
        lastNavTime = millis();
        drawMenu();
      }
    }
  }

  // Selection (Joystick Click)
  if (digitalRead(joySwitch) == LOW) {
    tone(speakerPin, 1800, 50);
    delay(100);
    tone(speakerPin, 2000, 100);

    // Execute action based on selected option
    if (menuOption == 0)
      startNewGame();
    else if (menuOption == 1)
      startNewGame(); // Also starts if clicking on rounds
    else
      showHighScore();
    delay(500);
  }
}

// Draws the Menu on the LCD
void drawMenu() {
  lcd.clear();

  // Scroll Logic: LCD only has 2 rows, but we have 3 options.
  // If we are on option 2 (Record), show rows 1 and 2.
  // Otherwise, show rows 0 and 1.
  int startItem = 0;
  if (menuOption == 2)
    startItem = 1;

  for (int i = 0; i < 2; i++) {
    int itemIndex = startItem + i;
    lcd.setCursor(1, i); // Leave space in column 0 for the arrow

    if (itemIndex == 0) {
      lcd.write(byte(0)); // Sword Icon
      lcd.print(" INICIAR");
    } else if (itemIndex == 1) {
      lcd.write(byte(3)); // Clock Icon
      lcd.print(" RONDAS: <");
      lcd.print(totalRounds);
      lcd.print(">");
    } else if (itemIndex == 2) {
      lcd.write(byte(1)); // Trophy Icon
      lcd.print(" RECORDE");
    }
  }

  // Draw Arrow on selected option
  lcd.setCursor(0, menuOption - startItem);
  lcd.write(byte(2)); // Arrow Icon
}

// ===================== UTILITIES =====================

// Intro animation with lights and sound
void introAnimation() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("REFLEX MASTER");
  lcd.setCursor(4, 1);
  lcd.print("PRO v2.0");

  // Blink LEDs and play sound 2 times
  for (int k = 0; k < 2; k++) {
    for (int i = 0; i < 3; i++)
      digitalWrite(ledPins[i], HIGH);
    tone(speakerPin, 400 + (k * 200), 100);
    delay(100);
    for (int i = 0; i < 3; i++)
      digitalWrite(ledPins[i], LOW);
    delay(100);
  }
}

// Shows High Score screen
void showHighScore() {
  gameState = 5;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1)); // Trophy
  lcd.print(" HALL OF FAME ");
  lcd.write(byte(1));
  lcd.setCursor(4, 1);
  lcd.print(highScore);
  lcd.print(" pts");
}

// Cancels current game and returns to menu
void cancelGame() {
  gameState = 0;
  for (int i = 0; i < 3; i++)
    digitalWrite(ledPins[i], LOW);
  lcd.clear();
  lcd.print("CANCELADO");
  tone(speakerPin, 100, 400);
  delay(1500);
  lastMenuOption = -1;
  drawMenu();
}

// Game Over Screen
void gameOver() {
  gameState = 4;
  lcd.clear();

  // Check if record was broken
  if (totalGameScore > highScore) {
    highScore = totalGameScore;
    lcd.setCursor(1, 0);
    lcd.write(byte(1));
    lcd.print(" RECORDE! ");
    lcd.write(byte(1));
    lcd.setCursor(4, 1);
    lcd.print(highScore);
    // Victory music
    tone(speakerPin, 523, 150);
    delay(150);
    tone(speakerPin, 659, 150);
    delay(150);
    tone(speakerPin, 784, 150);
    delay(150);
    tone(speakerPin, 1046, 600);
  } else {
    lcd.print("  FIM DO JOGO  ");
    lcd.setCursor(2, 1);
    lcd.print("Total: ");
    lcd.print(totalGameScore);
    tone(speakerPin, 300, 600); // End sound
  }
}