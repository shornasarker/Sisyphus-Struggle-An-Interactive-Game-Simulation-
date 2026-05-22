#ifndef CONSTANTS_H
#define CONSTANTS_H

// Window dimensions
#define WIDTH 900
#define HEIGHT 700

// Game constants
#define GRAVITY 0.15f
#define PUSH_FORCE 2.5f
#define JUMP_FORCE 6.0f
#define ROLLBACK_FORCE 0.10f
#define ROCK_RADIUS 25
#define MAN_WIDTH 28
#define MAN_HEIGHT 58
#define MOUNTAIN_LENGTH 5000
#define SUMMIT_X 4400
#define SUMMIT_TRIGGER_DISTANCE 35
#define MAX_OBSTACLES 20

// Collectibles
#define MAX_COLLECTIBLES 20

#define COLLECT_ENERGY 0
#define COLLECT_SHIELD 1
#define COLLECT_LIFE 2
#define COLLECT_SPEED 3

#define SHIELD_DURATION 400
#define SPEEDBOOST_DURATION 300
#define ENERGY_BONUS 20

// Day/Night cycle
#define DAY_CYCLE_TIME 3000.0f
#define DAY_SPEED (1.0f / DAY_CYCLE_TIME)

// Camera zoom states
#define ZOOM_NORMAL 1.0f
#define ZOOM_DISTANT 3.0f
#define ZOOM_SUMMIT 4.0f

// Weather constants
#define MAX_CLOUDS 12
#define MAX_RAIN 300
#define MAX_STARS 200
#define MAX_BIRDS 8

// Thunder constants
#define THUNDER_FLASH_TIME 4
#define THUNDER_GAP_TIME 3
#define THUNDER_COOLDOWN_MIN 220
#define THUNDER_COOLDOWN_MAX 520
#define THUNDER_MIN_BLINKS 2
#define THUNDER_MAX_BLINKS 4
#define LIGHTNING_SEGMENTS 12

// Game modes
#define MODE_PLAYING 0
#define MODE_SUMMIT 1
#define MODE_GAMEOVER 2

#endif
