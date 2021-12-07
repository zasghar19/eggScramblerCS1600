#include <Stepper.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "eggScrambler.h"
#include "eggScramblerWifi.h"

// Set up for z-axis movement
#define dirPin 6
#define stepPin 5
#define enaPin 2
int UP_BUTTON_PIN = 1;
int DOWN_BUTTON_PIN = 0;
int SPATULA_LOW_PIN = 7;

// Set up for xy movement
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN 200
#define SERVOMAX 650
#define CONTMAXPOSITIVE 80// max positive speed at 80
#define CONTSTATIONARY 300 // 280
#define CONTMAXNEGATIVE 512 // starts negative at 310, max negative at 510
int HalfServoIndex = 14;
int FullServoIndex = 0;

// Set up for stove stepper motor
int in1Pin = 13;
int in2Pin = 10;
int in3Pin = 9;
int in4Pin = 8;
Stepper motor(768, in1Pin, in2Pin, in3Pin, in4Pin);


// Global variables
state CURRENT_STATE;
bool is_spatula_low, stove_rotated;
int saved_clock;
// inps[] is an array which contains the following values, [moveVert, is_button_pressed, stop_button_pressed, spatula_low_toggled]
int inps[] = {0, 0, 0, 0};

// Input variables -- cooking_time is set via Alice's website (check eggScramblerWifi)
int cooking_time;
int stove_rotation = -600;

//Interrupt pin
int INTERRUPT_PIN =  4;

void setup() {

  // Stove Dial stepper motor setup
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);

  // Set up for z-movement
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  // Buttons for UP and DOWN and IS_SPATULA_LOW:
  pinMode(UP_BUTTON_PIN, INPUT);
  pinMode(DOWN_BUTTON_PIN, INPUT);
  pinMode(SPATULA_LOW_PIN, INPUT);
  // Set the spinning direction CW/CCW:
  digitalWrite(dirPin, HIGH);

  // Set up for xy movement
  pwm.begin();
  pwm.setPWMFreq(60);

  // set up serial for errors
  Serial.begin(9600);
  while(!Serial);

  // initialize values
  CURRENT_STATE = (state) 1;
  saved_clock = 0;
  is_spatula_low = false;
  stove_rotated = false;

  // Stove dial stepper speed, need to speed up or it breaks
  motor.setSpeed(20);

//  test_all_tests();

  // Set up wifi
  setupWifi();
  
  // set up watchdog (early warning/window enabled)
  enableWDT();

  // set up interrupt
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interruptButtonClicked, RISING);

}

void loop() {
  // pet watchdog
  WDT->CLEAR.reg = 0xA5;
  
  update_inputs(); 
  CURRENT_STATE = update_fsm(CURRENT_STATE, millis(), inps[0], inps[1], inps[2], inps[3]);
  
  delay(100);
}

/*
 * Main loop that carries out the logic of our fsm. Importantly, invalid states/variables 
 * and state combos move to sTURN_STOVE_OFF.
 */
state update_fsm(state cur_state, long mils, int moveVert, int is_button_pressed, int stop_button_pressed, int is_spatula_low_pin) {
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

      // Should be inaccessible, error
      else {
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }

      break;     
    case sTURN_STOVE_ON:
    
      if (moveVert != 0 and !stop_button_pressed) { // transition 2-3
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low(is_spatula_low_pin);
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

      // Should be inaccessible, error state
      else {
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      
      break;
    case sLOWER_SPATULA:
      if (!is_spatula_low and moveVert == 0 and !stop_button_pressed){ // transition 3-3, not moving spatula
        is_spatula_low = read_if_spatula_low(is_spatula_low_pin);
        next_state = sLOWER_SPATULA;
      }

      else if (!is_spatula_low and moveVert != 0 and !stop_button_pressed) { // transition 3-3, moving spatula
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low(is_spatula_low_pin);
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

      // Should be inaccessible, error state
      else {
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
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

      // Should be inaccessible, error state
      else {
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    case sTURN_STOVE_OFF:
      if (moveVert != 0) { // transition 5-6
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low(is_spatula_low_pin);
        next_state = sRAISE_SPATULA;
      }

      else { // transition 5-5
        next_state = sTURN_STOVE_OFF;
      }
      
      break;
    case sRAISE_SPATULA:
      if (is_spatula_low and moveVert != 0) { // transition 6-6, moving spatula
        move_spatula_z(moveVert);
        is_spatula_low = read_if_spatula_low(is_spatula_low_pin);
        next_state = sRAISE_SPATULA;
      }

      else if (is_spatula_low and moveVert == 0) { // transition 6-6, not moving spatula
        is_spatula_low = read_if_spatula_low(is_spatula_low_pin);
        next_state = sRAISE_SPATULA;
      }

      else if (!is_spatula_low) { // transition 6-1
        next_state = sWAITING;
      }

      else {
        turn_stove(-1 * stove_rotation);
        stove_rotated = false;
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    default:
      next_state = sTURN_STOVE_OFF;
      turn_stove(-1 * stove_rotation);
      stove_rotated = false;
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
  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(5) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_SRC(3) | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_ID(3);
  while (GCLK->STATUS.bit.SYNCBUSY);
  // Configure and enable WDT:
  // WDT->CONFIG.reg, WDT->EWCTRL.reg, WDT->CTRL.reg
  WDT->CONFIG.reg = WDT_CONFIG_PER(9);
  WDT->EWCTRL.reg = WDT_EWCTRL_EWOFFSET(8);
//  while (WDT->STATUS.bit.SYNCBUSY);
  // Enable early warning interrupts on WDT:
  // reference WDT registers with WDT->register_name.reg
  WDT->INTENSET.reg = WDT_INTENSET_EW;
  WDT->CTRL.reg = WDT_CTRL_ENABLE;
}

void WDT_Handler() {
  // Clear interrupt register flag
  WDT->INTFLAG.reg |= WDT_INTFLAG_EW;
  
  // Warn user that a watchdog reset may happen
  Serial.println("A WATCHDOG RESET MAY HAPPEN!!!");
}

void move_spatula_z(int moveVert) {
  saved_clock = millis();

  if (moveVert == 1) {
    Serial.println("move up");
    digitalWrite(dirPin, HIGH);
    digitalWrite(enaPin, HIGH);
  }
  else if (moveVert == -1) {
    Serial.println("move doown");
    digitalWrite(dirPin, LOW);
    digitalWrite(enaPin, LOW);
  }

  while (millis() - saved_clock < 1000) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  
}


void move_spatula_xy() {
  for (int i = SERVOMIN; i <= SERVOMAX; i += 10) {
    // half speed positive
    int tmp = 0;
    int fullServoSpeed = map(tmp, SERVOMIN, SERVOMAX, CONTSTATIONARY, CONTMAXPOSITIVE);
    pwm.setPWM(HalfServoIndex, 0, i);
    delay(50);
  }
}

void turn_stove(int stove_rotation) {
  motor.step(stove_rotation);
}

void interruptButtonClicked() {
  if (stove_rotated) {
    turn_stove(-1 * stove_rotation);
    stove_rotated = false;
  }
  CURRENT_STATE  = sTURN_STOVE_OFF;
  Serial.println("Interrupted via stop button!");
}
