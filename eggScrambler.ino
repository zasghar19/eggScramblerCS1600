#include "eggScrambler.h"

state CURRENT_STATE;
bool is_button_pressed, stop_button_pressed, is_spatula_low, stove_rotated;
int saved_clock;

// Input variables -- we need to input the correct values
int cooking_time;
int stove_rotation;
int spatula_distance;

void setup() {

  // set up serial for errors
  Serial.begin(9600);
  while(!Serial);

  // initialize values
  CURRENT_STATE = (state) 1;
  saved_clock = 0;
  is_button_pressed = false;
  stop_button_pressed = false;
  is_spatula_low = false;
  stove_rotated = false;

  // move the peripherals to initial positions
  move_to_initial_position();

  // set up watchdog (normal mode, no early warning/window)
  enableWDT();
  

}

void loop() {
  // kick watchdog
  WDT->CLEAR.reg = 0xA5;

  update_inputs();
  CURRENT_STATE = update_fsm(CURRENT_STATE, millis());
  
  // delay?
}

// Invalid states/variables and state combos move to sTURN_STOVE_OFF
state update_fsm(state cur_state, long mils) {
  state next_state;

  switch(cur_state) {
    case sWAITING:
      if (!is_button_pressed) { // transition 1-1
        next_state = sWAITING;
      }

      else if (is_button_pressed) { // transition 1-2
        turn_stove(stove_rotation);
        stove_rotated = read_if_stove_rotated();
        next_state = sTURN_STOVE_ON;
      }

      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;     
    case sTURN_STOVE_ON:
      if (!stove_rotated and !stop_button_pressed) { // transition 2-2
        // if it isn't async, we should rotate stove again
        stove_rotated = read_if_stove_rotated();
        next_state = sTURN_STOVE_ON;
      }

      else if (stove_rotated and !stop_button_pressed) { // transitition 2-3
        move_spatula_z(-1 * spatula_distance);
        is_spatula_low = read_if_spatula_low();
        next_state = sLOWER_SPATULA;
      }

      else if (stop_button_pressed) { // transition 2-5
        turn_stove(-1 * stove_rotation);
        stove_rotated = read_if_stove_rotated();
        next_state = sTURN_STOVE_OFF;
      }
      
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    case sLOWER_SPATULA:
      if (!is_spatula_low and !stop_button_pressed){ // transition 3-3
        is_spatula_low = read_if_spatula_low();
        // if it isnt async, we should move spatula again
        next_state = sLOWER_SPATULA;
      }

      else if (is_spatula_low and !stop_button_pressed) { // transition 3-4
        move_spatula_xy();
        saved_clock = mils;
        next_state = sMOVE_SPATULA;
      }

      else if (stop_button_pressed) { // transition 3-5
        turn_stove(-1 * stove_rotation);
        stove_rotated = read_if_stove_rotated();
        next_state = sTURN_STOVE_OFF;
      }
      
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
        stove_rotated = read_if_stove_rotated();
        next_state = sTURN_STOVE_OFF;
      }
      
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    case sTURN_STOVE_OFF:
      if (stove_rotated) { // transition 5-5
        // if this isn't async, we need to rotate again
        stove_rotated = read_if_stove_rotated();
        next_state = sTURN_STOVE_OFF;
      }

      else if (!stove_rotated) { // transition 5-6
        move_to_initial_position();
        // if this isn't async, we need to figure this out
        is_spatula_low = read_if_spatula_low();
        next_state = sRAISE_SPATULA;
      }
      
      else {
        next_state = sTURN_STOVE_OFF;
        Serial.println("invalid state and variables, turning off stove");
      }
      
      break;
    case sRAISE_SPATULA:
      if (is_spatula_low) { // transition 6-6
        // if this isn't async, we need to move the spatula
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

void move_spatula_z(int spatula_distance) {
  
}

void move_spatula_xy() {
  
}

void turn_stove(int stove_rotation) {
  
}

void move_to_initial_position() {
  
}
