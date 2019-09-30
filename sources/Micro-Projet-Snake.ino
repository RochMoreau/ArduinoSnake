#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "marylinBmp.h"
#include "starwars.h"

// ------------------------ HARDWARE ------------------------------
// Declare LCD object for software SPI
// Adafruit_PCD8544(CLK,DIN,D/C,CE,RST);
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// For led score display
const int latchPin = 9;
const int clockPin = 10;
const int dataPin = 8;

// Control buttons
const int lb = A5; // Left Button
const int rb = A2; // Right Button
const int db = A3; // Down Button
const int ub = A4; // Up Button

// Potentiometer
const int potPin = A0;

// Buzzer output
const int buzzerPin = 11;


// ------------------------ SOFTWARE ------------------------------

// GLOBAL CONST
// In each coordinates or dimensions array, first cell is for X, second cell is Y
const int screen_dim[2] = {84,48};
//typedef enum DirectionEnum DirectionEnum;

enum DirectionEnum {
  left,
  right,
  down,
  up
};


// GAME VARIABLES
byte leds = 0; // Score display

unsigned long snake_last_moove = 0; // Last time the snake has been mooved (millis)

int score = 0;
unsigned long last_buzzer = 0; // The last time the buzzer was activated

int treat_coords[2];
int treat_is_alive = false;

DirectionEnum snake_direction = right;
int snake_coords[2] = {0,0};
int snake_refresh_rate = 200; // Every snake_refresh_rate the snake will moove


// ------------------------ SETUP ---------------------------------

void setup()   {
  Serial.begin(9600);

  //Initialize Display
  display.begin();
  // you can change the contrast around to adapt the display for the best viewing!
  display.setContrast(57);
  // Clear the buffer.
  display.clearDisplay();
  

  // LED SCORE DISPLAY
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);

  // Button input init
  pinMode(lb, INPUT);
  pinMode(rb, INPUT);
  pinMode(db, INPUT);
  pinMode(ub, INPUT);

  // Potentiometer
  pinMode(potPin, INPUT);

  // Buzzer
  pinMode(buzzerPin, OUTPUT);
}


// ------------------------ UTILITIES -----------------------------

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}

bool doOverlap() 
{

  // If one rectangle is on left side of other 
  if (snake_coords[0] > treat_coords[0]+3 || treat_coords[0] > snake_coords[0]+3) 
    return false; 
  
  // If one rectangle is above other 
  if (snake_coords[1] > treat_coords[1]+3 || treat_coords[1] > snake_coords[1]+3) 
    return false; 
  
  return true; 
} 


// ------------------------ GAME FUNCTIONS ------------------------

// Generate random coordinates used later to spawn a "treat" to catch in the game
void generate_treat() {
  if(!treat_is_alive) { // If treat is not on the screen, generate one
    treat_coords[0] = random(0,screen_dim[0]-2);
    treat_coords[1] = random(0,screen_dim[1]-2);
    treat_is_alive = true;
  }
}


// Based on the buttons inputs, define the direction in which the snake (i.e. the player) mooves
void direction_manager() {
  int goLeft = digitalRead(lb);
  int goRight = digitalRead(rb);
  int goUp = digitalRead(ub);
  int goDown = digitalRead(db);

  if(goLeft == HIGH) {
    snake_direction = left;
  }
  if(goRight == HIGH) {
    snake_direction = right;
  }
  if(goUp == HIGH) {
    snake_direction = up;
  }
  if(goDown == HIGH) {
    snake_direction = down;
  }

}


// Based on the potentiometer input, regulate the speed at which the snake mooves
void speed_manager() {
  snake_refresh_rate = map(analogRead(potPin), 1024, 0, 0, 200);
}


// Set the new coordinates every
void moove() {
  unsigned long now = millis();
  
  if( now - snake_last_moove > snake_refresh_rate) { // Check if snake is allowed to moove
    switch(snake_direction) {
      case right:
        snake_coords[0] = (snake_coords[0] + 2) % screen_dim[0];
        break;
      case left: 
        snake_coords[0] = (snake_coords[0] - 2 + screen_dim[0]) % screen_dim[0];
        break;
      case up:
        snake_coords[1] = (snake_coords[1] - 2 + screen_dim[1]) % screen_dim[1];
        break;
      case down:
        snake_coords[1] = (snake_coords[1] + 2) % screen_dim[1];
        break;
      default:
        break;
    }
    snake_last_moove = now;
  }
}


// Display the game screen at current state
void display_game() {
  //Draw snake
  display.fillRect(snake_coords[0], snake_coords[1], 4, 4, BLACK);

  // draw treat
  display.drawRoundRect(treat_coords[0], treat_coords[1], 4, 4, 2, BLACK);

  display.display();
  display.clearDisplay();  
}


// Display the score on the leds through the shift register
void score_display() {
  leds = 0;
  updateShiftRegister();
  for(int i = 0; i < score; i++) {
    bitSet(leds, i);
  }
  updateShiftRegister();
}


// Check if user eats the treat and add a point when it occurs
void score_manager() {
  if(doOverlap()) {
    treat_is_alive = false;
    last_buzzer = millis();
    ++score;
  }
}


// Bip when snake eat a treat
void buzzer_eat() {
  unsigned long now = millis();
  if(now - last_buzzer < 200) { //200ms activation time
      tone(buzzerPin, 500);
  } else {
    noTone(buzzerPin);
  }

}


//
void display_win() {
  display.drawBitmap(0, 0,  MarilynMonroe, screen_dim[0], screen_dim[1], BLACK);
  display.display();
}

// ------------------------ GAME EXECUTION ------------------------

void loop() {
  if(score < 8) {
    generate_treat();
    
    direction_manager();
    speed_manager();
    moove();
  
    score_manager();
    score_display();
    buzzer_eat();
    
    display_game();  
  } else {
    noTone(buzzerPin);
    display_win();
    music();
  }
}
