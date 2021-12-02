typedef enum {
    sWAITING = 1,
    sTURN_STOVE_ON = 2,
    sLOWER_SPATULA = 3,
    sMOVE_SPATULA = 4,
    sTURN_STOVE_OFF = 5,
    sRAISE_SPATULA = 6
} state;

// Variables



// Helper Functions

state update_fsm(state cur_state, long mils, int moveVert);

void enableWDT();

void move_spatula_z(int spatula_distance);

void move_spatula_xy();

void turn_stove(int stove_rotation);

void move_to_initial_position();