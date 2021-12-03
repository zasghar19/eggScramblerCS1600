#include "eggScramblerWifi.h"

int* update_inputs() {
  // check stop_button_pressed, is_button_pressed, up/down button, button to start spatula movement
  // inputs are {moveZ, is_button_pressed, stop_button_pressed, spatula_low_toggled}
//  int inps[] = {0, 0, 0};
// for testing

  if (digitalRead(UP_BUTTON_PIN)) {
    Serial.println("in up");
    inps[0] = 1;
  }
  else if (digitalRead(DOWN_BUTTON_PIN)) {
    Serial.println("in down");
    inps[0] = -1;
  } 
  else {
    inps[0] = 0;
  }
  
  int clientConnected = checkForClients();

  if (clientConnected == 1) {
    inps[1] = 1;
  } 

  else if (clientConnected == -1) {
    inps[2] = 1;
  }

  else {
    inps[1] = inps[2] = 0;
  }


  if (digitalRead(SPATULA_LOW_PIN)) {
    Serial.println("spatula low");
    inps[3] = 1;
  }
  else {
    inps[3] = 0;
  }

  Serial.println(inps[0]);
  Serial.println(inps[1]);
  Serial.println(inps[2]);
  Serial.println(inps[3]);

}

// by button, Sami will implement (based on inputs), return false by default
bool read_if_spatula_low(int is_spatula_low_pin) {
  if (is_spatula_low_pin) {
    return !is_spatula_low;
  }
  return is_spatula_low;
}
