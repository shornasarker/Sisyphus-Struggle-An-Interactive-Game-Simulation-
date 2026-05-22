#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct {
    float x;
    float size;
    int type;
    int active;
    int alreadyHit;
} Obstacle;

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    int active;
} Particle;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    int isRolling;
    float rotation;
} Rock;

typedef struct {
    float x, y;
    float vx, vy;
    int width, height;
    int isJumping;
    int canJump;
    int score;
    int lives;
    int invincible;
    int invincibleTimer;

    int shieldActive;
    int shieldTimer;

    int speedBoostActive;
    int speedBoostTimer;


    int energy;

    int resistingSlip;
} Man;

typedef struct {
    float x;
    float y;
    int type;
    int active;
    float bobPhase;
} Collectible;

typedef struct {
    float x;
    float y;
    float size;
    float speed;
} Cloud;

typedef struct {
    float x, y;
    float brightness;
    float twinkle;
} Star;

typedef struct {
    float x, y;
    float speed;
    float wingPhase;
} Bird;

#endif
