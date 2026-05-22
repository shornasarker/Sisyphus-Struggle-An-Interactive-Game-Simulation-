#ifndef GLOBALS_H
#define GLOBALS_H

#include "structures.h"
#include "constants.h"

// ===== GAME OBJECTS =====
extern Rock rock;
extern Man man;

extern Obstacle obstacles[MAX_OBSTACLES];
extern int numObstacles;

extern Particle particles[100];
extern int numParticles;

// ===== COLLECTIBLES =====
extern Collectible collectibles[MAX_COLLECTIBLES];
extern int numCollectibles;

// ===== CAMERA =====
extern float cameraX;
extern float cameraY;
extern float targetCameraY;
extern float cameraZoom;
extern float targetZoom;
extern int windowWidth;
extern int windowHeight;

// ===== WORLD =====
extern float mountainBase;
extern float mountainTop;

// ===== GAME STATE =====
extern int gameMode;
extern float modeTimer;
extern int gameActive;
extern int restartRequested;
extern int summitReached;

// ===== WEATHER =====
extern float wind;
extern float timeOfDay;
extern int isRaining;

// ===== THUNDER =====
extern int thunderActive;
extern int thunderTimer;
extern int thunderCooldown;

extern int thunderBlinkCount;
extern int thunderBlinksLeft;
extern int thunderGapTimer;

// ===== LIGHTNING =====
extern int lightningSegments;
extern float lightningX[LIGHTNING_SEGMENTS + 1];
extern float lightningY[LIGHTNING_SEGMENTS + 1];

// ===== ENVIRONMENT =====
extern Cloud clouds[MAX_CLOUDS];
extern Star stars[MAX_STARS];
extern Bird birds[MAX_BIRDS];

#endif
