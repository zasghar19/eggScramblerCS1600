#include "eggScramblerWifi.h"

int* update_inputs() {
  // inps is an array of [moveVert, is_button_pressed, stop_button_pressed, spatula_low_toggled]

  if (digitalRead(UP_BUTTON_PIN)) {
    inps[0] = 1;
  } else if (digitalRead(DOWN_BUTTON_PIN)) {
    inps[0] = -1;
  } else {
    inps[0] = 0;
  }
  
  int clientConnected = checkForClients();

  if (clientConnected == 1) {
    inps[1] = 1;
    inps[2] = 0;
  } else if (clientConnected == -1) {
    inps[2] = 1;
    inps[1] = 0;
  } else {
    inps[1] = inps[2] = 0;
  }

  if (digitalRead(SPATULA_LOW_PIN)) {
    inps[3] = 1;
  } else {
    inps[3] = 0;
  }
}

bool read_if_spatula_low(int is_spatula_low_pin) {
  if (is_spatula_low_pin) {
    return !is_spatula_low;
  }
  return is_spatula_low;
}
