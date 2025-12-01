# Reflex Master PRO v2.0 (Project: ReflexMasterPro)

**Reflex Master PRO v2.0** is an Arduino-based reaction time training game designed to help fencers (and anyone else!) improve their reflexes. It features a menu system, multiple game rounds, high score tracking, and audio-visual feedback.

> [!NOTE]
> This project was formerly named `esgrima`. The main sketch file is now `ReflexMasterPro.ino`. Please ensure the parent folder is also named `ReflexMasterPro` for the Arduino IDE to open it correctly.

## Hardware Setup

The project uses an Arduino board with the following connections:

### Display
*   **LCD (16x2)**:
    *   RS -> Pin 12
    *   E  -> Pin 11
    *   D4 -> Pin 5
    *   D5 -> Pin 4
    *   D6 -> Pin 3
    *   D7 -> Pin 2

### Controls
*   **Joystick**:
    *   VRx -> A0 (Left/Right navigation)
    *   VRy -> A1 (Up/Down navigation)
    *   SW  -> Pin 7 (Select/Enter/Cancel)
*   **Game Buttons**:
    *   Left Button   -> Pin 6
    *   Center Button -> Pin 8
    *   Right Button  -> Pin 9

### Indicators
*   **LEDs** (Active High):
    *   Left LED   -> A2
    *   Center LED -> A3
    *   Right LED  -> A4
*   **Audio**:
    *   Speaker/Buzzer -> Pin 10

## Features

*   **Menu System**: Navigate through options to Start Game, Configure Rounds, or View High Scores.
*   **Customizable Length**: Adjust the number of rounds per game (default is 10).
*   **High Score**: Keeps track of the best session score (reset on power cycle).
*   **Anti-Cheat**: Detects false starts (holding a button before the signal) and penalizes the player.
*   **Precision Timing**: Measures reaction time in milliseconds.

## How to Play

1.  **Start**: Select "INICIAR" from the main menu.
2.  **En Garde**: Wait for the "EN GARDE..." signal. Keep your hands ready but do not press any buttons yet!
3.  **React**: One of the three LEDs (Left, Center, Right) will light up randomly, accompanied by a sound.
4.  **Strike**: Press the corresponding button as fast as you can!
    *   **Success**: You get points based on your speed (faster = more points).
    *   **Miss**: If you press the wrong button, you lose points.
    *   **False Start**: If you press a button before the light, you get a penalty.
5.  **Result**: After the set number of rounds, the game ends and shows your total score.

## Controls Summary

*   **Joystick Up/Down**: Navigate Menu
*   **Joystick Left/Right**: Adjust Settings (e.g., Round count)
*   **Joystick Click**: Select / Confirm / Cancel Game
*   **Game Buttons**: Hit the target during the game
