#define TESTING
bool test_all_tests();

typedef enum {
    sWAITING = 1,
    sTURN_STOVE_ON = 2,
    sLOWER_SPATULA = 3,
    sMOVE_SPATULA = 4,
    sTURN_STOVE_OFF = 5,
    sRAISE_SPATULA = 6
} state;

// Helper Functions

state update_fsm(state cur_state, long mils, int moveVert, int is_button_pressed, int stop_button_pressed, int is_spatula_low_pin);

void enableWDT();

void move_spatula_z(int moveVert);

void move_spatula_xy();

void turn_stove(int stove_rotation);
