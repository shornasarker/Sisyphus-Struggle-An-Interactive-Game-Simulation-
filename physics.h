#ifndef PHYSICS_H
#define PHYSICS_H

float getMountainY(float x);
float getMountainSlope(float x);
float getLightIntensity();

void spawnParticles(float x, float y, int count);
void updateParticles();

void updateCamera();
void checkCollisions();
void jump();
void triggerSummit();

// ✅ NEW
void updateBoosts();
void checkCollectibles();

#endif
