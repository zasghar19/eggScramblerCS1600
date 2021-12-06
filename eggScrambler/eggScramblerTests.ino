 #define TESTING

/*
 * A struct to keep all state inputs in one place
 */
typedef struct {
  long mils;
  int movVert;
  int is_button_pressed;
  int stop_button_pressed;
  int is_spatula_low_pin;
} state_inputs;

/*
 * A struct to keep all 9 state variables in one place
 */
typedef struct {
  bool is_spatula_low;
  bool stove_rotated;
  int cooking_time;
  int stove_rotation;
  int saved_clock;
} state_vars;

bool test_transition(state start_state,
                     state end_state,
                     state_inputs test_state_inputs, 
                     state_vars start_state_vars,
                     state_vars end_state_vars,
                     bool verbos);
///*        
// * Helper function for printing states
// */
//char* s2str(state s) {
//  switch(s) {
//    case sWAITING:
//    return "(1) WAITING";
//    case sTURN_STOVE_ON:
//    return "(2) TURN_STOVE_ON";
//    case sLOWER_SPATULA:
//    return "(3) LOWER_SPATULA";
//    case sMOVE_SPATULA:
//    return "(4) MOVE_SPATULA";
//    case sTURN_STOVE_OFF:
//    return "(5) TURN_STOVE_OFF";
//    case sRAISE_SPATULA:
//    return "(6) RAISE_SPATULA";
//    default:
//    return "???";
//  }
//}

///*
// * Helper function for printing orientations
// */
//char* o2str(orientation o) {
//  switch(o) {
//    case UP:
//    return "UP";
//    case RIGHT:
//    return "RIGHT";
//    case DOWN:
//    return "DOWN";
//    case LEFT:
//    return "LEFT";
//    default:
//    return "???";
//  }
//}

/*
 * Given a start state, inputs, and starting values for state variables, tests that
 * update_fsm returns the correct end state and updates the state variables correctly
 * returns true if this is the case (test passed) and false otherwise (test failed)
 * 
 * Need to use "verbos" instead of "verbose" because verbose is apparently a keyword
 */
bool test_transition(state start_state,
                     state end_state,
                     state_inputs test_state_inputs, 
                     state_vars start_state_vars,
                     state_vars end_state_vars,
                     bool verbos) {
  is_spatula_low = start_state_vars.is_spatula_low;
  stove_rotated = start_state_vars.stove_rotated;
  saved_clock = start_state_vars.saved_clock;
  cooking_time = start_state_vars.cooking_time;
  stove_rotation = start_state_vars.stove_rotation;
  state result_state = update_fsm(start_state, test_state_inputs.mils, test_state_inputs.movVert, test_state_inputs.is_button_pressed, test_state_inputs.stop_button_pressed, test_state_inputs.is_spatula_low_pin);
  bool passed_test = (end_state == result_state and
                      is_spatula_low == end_state_vars.is_spatula_low and
                      stove_rotated == end_state_vars.stove_rotated and
                      saved_clock == end_state_vars.saved_clock and
                      cooking_time == end_state_vars.cooking_time and
                      stove_rotation == end_state_vars.stove_rotation);
                      

  if (! verbos) {
    return passed_test;
  } else if (passed_test) {
//    char s_to_print[200];
//    sprintf(s_to_print, "Test from %s to %s PASSED", start_state, end_state);
//    Serial.println(s_to_print);
    return true;
  } else {
    char s_to_print[200];
//    sprintf(s_to_print, "Test from %s to %s FAILED", start_state, end_state);
//    Serial.println(s_to_print);
//    sprintf(s_to_print, "End state expected: %s | actual: %s", end_state, result_state);
//    Serial.println("End state expected: %s | actual: %s", end_state, result_state);
//    Serial.println("End state expected: ", end_state)
//    sprintf(s_to_print, "Inputs: mils %ld | movVert %d | is_button_pressed %s | stop_button_pressed %s | is_spatula_low_pin %s", test_state_inputs.mils, test_state_inputs.movVert, test_state_inputs.is_button_pressed, test_state_inputs.stop_button_pressed, test_state_inputs.is_spatula_low_pin);
//    Serial.println(s_to_print);
//    sprintf(s_to_print, "          %2s | %2s | %5s | %3s | %3s ", "is_spatual_low", "stove_rotated", "saved_clock", "cooking_time", "stove_rotation");
//    Serial.println(s_to_print);
    sprintf(s_to_print, "expected: %2d | %2d | %2d | %3d | %3d ", end_state_vars.is_spatula_low, end_state_vars.stove_rotated, end_state_vars.saved_clock, end_state_vars.cooking_time, end_state_vars.stove_rotation);
    Serial.println(s_to_print);
    sprintf(s_to_print, "actual:   %2d | %2d | %2d | %3d | %3d ", is_spatula_low, stove_rotated, saved_clock, cooking_time, stove_rotation);
    Serial.println(s_to_print);
    Serial.print(end_state);
    Serial.print(result_state);
    return false;
  }
}

/*
 * REPLACE THE FOLLOWING 6 LINES WITH YOUR TEST CASES
 */
const state test_states_in[14] = {(state) 1, (state) 1, (state) 2, (state) 2, (state) 2, (state) 3, (state) 3, (state) 3, (state) 4, (state) 4, (state) 5, (state) 5, (state) 6, (state) 6};
const state test_states_out[14] = {(state) 1, (state) 2, (state) 2, (state) 3, (state) 5, (state) 3, (state) 4, (state) 5, (state) 4, (state) 5, (state) 5, (state) 6, (state) 1, (state) 1};
const state_inputs test_input[14] = {{0,0,false,false,0}, {0,0,true,false,0}, {0,0,true,false,0}, {0,1,true,false,1}, {0,0,true,true,1}, {0,0,false,false,1}, {0,0,true,false,1}, {0,1,true,true,1}, {60,0,true,false,1}, {200,0,true,false,1}, {0,0,true,false,1}, {0,1,true,false,0}, {0,0,true,false,0}, {0,0,true,false,0}};
const state_vars test_in_vars[14] = {{0,0,0,0,0}, {false,false,0,0,0}, {false,false,0,0,0}, {false,true,5,0,5}, {0,0,0,0,0}, {false,0,0,0,0}, {true,true,0,0,0}, {0,0,0,0,0}, {true,true,100,0,20}, {true,true,100,0,50}, {false,true,0,0,0}, {true,false,0,0,0}, {false,true,0,0,0}, {false,false,0,0,0}};
const state_vars test_out_vars[14] = {{0,0,0,0,0}, {false,true,0,0,0}, {false,false,0,0,0}, {true,true,5,0,5}, {false,false,0,0,0}, {true,0,0,0,0}, {true,true,0,0,0}, {false,false,0,0,0}, {true,true,100,0,20}, {true,false,100,0,50}, {false,true,0,0,0}, {true,false,0,0,5608}, {false,true,0,0,0}, {false,false,0,0,0}};
const int num_tests = 14;
/*
 * Runs through all the test cases defined above
 */
bool test_all_tests() {
  for (int i = 0; i < num_tests; i++) {
    Serial.print("Running test ");
    Serial.println(i);
    if (!test_transition(test_states_in[i], test_states_out[i], test_input[i], test_in_vars[i], test_out_vars[i], true)) {
      return false;
    }
    Serial.println();
  }
  Serial.println("All tests passed!");
  return true;
}
