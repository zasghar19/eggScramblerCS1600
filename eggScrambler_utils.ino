#include "eggScramblerWifi.h"

int* update_inputs() {
  // check stop_button_pressed, is_button_pressed, up/down button, button to start spatula movement
  // inputs are {moveZ, is_button_pressed, stop_button_pressed}
  int inps[] = {0, 0, 0};

  if (digitalRead(UP_BUTTON_PIN)) {
    inps[0] = 1;
  }
  else if (digitalRead(DOWN_BUTTON_PIN)) {
    inps[0] = -1;
  }

  int clientConnected = checkForClients();

  if (clientConnected == 1) {
    inps[1] = 1;
  } 

  else if (clientConnected == -1) {
    inps[2] = 1;
  }

  return inps;
}

// by button, Sami will implement (based on inputs)
bool read_if_spatula_low() {

}
