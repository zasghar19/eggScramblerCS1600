#include "eggScrambler.h"
#include <Stepper.h>
#include "eggScramblerWifi.h"

// Set up for z-axis movement
#define dirPin 6
#define stepPin 5
#define enaPin 2
int UP_BUTTON_PIN = 1;
int DOWN_BUTTON_PIN = 0;

// stove dial stepper motor set-up, change pin numbers
int in1Pin = 12;
int in2Pin = 11;
int in3Pin = 10;
int in4Pin = 9;
Stepper motor(768, in1Pin, in2Pin, in3Pin, in4Pin);

state CURRENT_STATE;
bool is_spatula_low, stove_rotated;
int saved_clock;
int* inputs;
int moveVert;

// Input variables -- we need to input the correct values
int cooking_time;
int stove_rotation = -2000;

void setup() {

  // Stove Dial stepper motor setup
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);

  // Set up for z-movement
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  // Buttons for UP and DOWN:
  pinMode(UP_BUTTON_PIN, INPUT);
  pinMode(UP_BUTTON_PIN, INPUT);
  // Set the spinning direction CW/CCW:
  digitalWrite(dirPin, HIGH);

  // set up serial for errors
  Serial.begin(9600);
  while(!Serial);

  // initialize values
  CURRENT_STATE = (state) 1;
  saved_clock = 0;
  is_spatula_low = false;
  stove_rotated = false;

  // Stove dial stepper speed
  motor.setSpeed(20);

  // Set up wifi
  setupWifi();
  
  // set up watchdog (normal mode, no early warning/window)
  enableWDT();
  

}

void loop() {
  // kick watchdog
  WDT->CLEAR.reg = 0xA5;

  inputs = update_inputs();
  CURRENT_STATE = update_fsm(CURRENT_STATE, millis(), inputs[0], inputs[1], inputs[2]);
  
  delay(10);
}

// Invalid states/variables and state combos move to sTURN_STOVE_OFF
state update_fsm(state cur_state, long mils, int moveVert, int is_button_pressed, int stop_button_pressed) {
  state next_state;

  switch(cur_state) {
    case sWAITING:
      if (!is_button_pressed) { // transition 1-1
        next_state = sWAITING;
      }

      else if (is_button_pressed) { // transition 1-2
        turn_stove(stove_rotation);
        stove_rotated = true;
        next_state = sTURN_STOVE_ON;
      }

      // Should be inaccessible
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;     
    case sTURN_STOVE_ON:
      if (moveVert != 0 and !stop_button_pressed) { // transitition 2-3
        // enables movement up and down
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low();
        next_state = sLOWER_SPATULA;
      }

      else if (moveVert == 0 and !stop_button_pressed) { // transition 2-2
        next_state = sTURN_STOVE_ON;
      }

      else if (stop_button_pressed) { // transition 2-5
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
        next_state = sTURN_STOVE_OFF;
      }

      // Should be inaccessible
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      
      break;
    case sLOWER_SPATULA:
      if (!is_spatula_low and moveVert == 0 and !stop_button_pressed){ // transition 3-3, not moving spatula
        is_spatula_low = read_if_spatula_low();
        next_state = sLOWER_SPATULA;
      }

      else if (!is_spatula_low and moveVert != 0 and !stop_button_pressed) { // transition 3-3, moving spatula
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low();
        next_state = sLOWER_SPATULA;
      }

      else if (is_spatula_low and !stop_button_pressed) { // transition 3-4
        move_spatula_xy();
        saved_clock = mils;
        next_state = sMOVE_SPATULA;
      }

      else if (stop_button_pressed) { // transition 3-5
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
        next_state = sTURN_STOVE_OFF;
      }

      // Should be inaccessible
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    case sMOVE_SPATULA:
      if (mils - saved_clock < cooking_time and !stop_button_pressed) { // transition 4-4
        move_spatula_xy();
        next_state = sMOVE_SPATULA;
      }

      else if (mils - saved_clock >= cooking_time or stop_button_pressed) { // transition 4-5
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
        next_state = sTURN_STOVE_OFF;
      }

      // Should be inaccessible
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    case sTURN_STOVE_OFF:
      if (moveVert != 0) { // transition 5-6
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low();
        next_state = sRAISE_SPATULA;
      }

      else { // transition 5-5
        next_state = sTURN_STOVE_OFF;
      }
      
      break;
    case sRAISE_SPATULA:
      if (is_spatula_low and moveVert != 0) { // transition 6-6, moving spatula
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low();
        next_state = sRAISE_SPATULA;
      }

      else if (is_spatula_low and moveVert == 0) { // transition 6-6, not moving spatula
        is_spatula_low = read_if_spatula_low();
        next_state = sRAISE_SPATULA;
      }

      else if (!is_spatula_low) { // transition 6-1
        next_state = sWAITING;
      }

      
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    default:
      next_state = sTURN_STOVE_OFF;
      Serial.println("invalid state, turning stove off");
  }
  
  return next_state;
}

// We need to make sure that the stove can turn on/off within our WDT
void enableWDT() {
  // Clear and enable WDT
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_EnableIRQ(WDT_IRQn);

  // Configure and enable WDT GCLK:
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(4) | GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // set GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg = GCLK_GENCTRL_DIVSEL | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_ID(3);

  // Configure and enable WDT (4 second period)
  WDT->CONFIG.reg |= WDT_CONFIG_PER(9);
  while (WDT->STATUS.bit.SYNCBUSY);
  WDT->CTRL.reg |= WDT_CTRL_ENABLE;
  while (WDT->STATUS.bit.SYNCBUSY);
}

// Asynchronous
void move_spatula_z(int moveVert) {
  if (moveVert == 1) {
    digitalWrite(dirPin, HIGH);
    digitalWrite(enaPin, HIGH);
  }
  else if (moveVert == -1) {
    digitalWrite(dirPin, LOW);
    digitalWrite(enaPin, LOW);
  }
  
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(500);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(500);
}

// Sami tomorrow
void move_spatula_xy() {
  
}

// Synchronous, need to make sure WDT is longer than this
void turn_stove(int stove_rotation) {
  motor.step(stove_rotation);
}
