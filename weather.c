#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include "constants.h"
#include "structures.h"
#include "globals.h"
#include "weather.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif



void initClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].x = rand() % (MOUNTAIN_LENGTH + 800) - 400;
        clouds[i].y = 400 + rand() % 200;
        clouds[i].speed = 0.05f + (rand() % 30) / 200.0f;
        clouds[i].size = 40 + rand() % 60;
    }
}

void initRain() {
}

void initStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = rand() % WIDTH;
        stars[i].y = 300 + rand() % (HEIGHT - 300);
        stars[i].brightness = 0.65f + (rand() % 35) / 100.0f;
        stars[i].twinkle = (rand() % 100) / 100.0f;
    }
}
void initBirds() {
    for (int i = 0; i < MAX_BIRDS; i++) {
        birds[i].x = rand() % (MOUNTAIN_LENGTH + 800) - 400;
        birds[i].y = 370 + rand() % 140;
        birds[i].speed = 0.7f + (rand() % 10) / 10.0f;
        birds[i].wingPhase = (rand() % 100) / 100.0f * 2.0f * 3.14159f;
    }
}
void updateDayNight() {
    timeOfDay += DAY_SPEED;
    if (timeOfDay > 1.0f) timeOfDay = 0.0f;
}
void updateClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].x += clouds[i].speed + wind * 0.1f;

        if (clouds[i].x > MOUNTAIN_LENGTH + 800)
            clouds[i].x = -800;

        if (clouds[i].x < -800)
            clouds[i].x = MOUNTAIN_LENGTH + 800;
    }
}

void updateRain() {
}

void updateBirds() {
    float t = glutGet(GLUT_ELAPSED_TIME) * 0.002f;

    for (int i = 0; i < MAX_BIRDS; i++) {
        float depthFactor = 0.8f + 0.4f * sin(i * 1.3f + t * 0.5f);

        birds[i].x += birds[i].speed * depthFactor;

        /* wing animation */
        birds[i].wingPhase += 0.16f + birds[i].speed * 0.03f;
        if (birds[i].wingPhase > 6.28318f)
            birds[i].wingPhase -= 6.28318f;

        /* smooth natural sky flight */
        birds[i].y += sin(t * 1.3f + i * 0.9f) * 0.22f;
        birds[i].y += cos(t * 0.7f + i * 1.4f) * 0.08f;

        /* keep birds inside a nice sky band */
        if (birds[i].y < 360) birds[i].y = 360;
        if (birds[i].y > 540) birds[i].y = 540;

        if (birds[i].x > MOUNTAIN_LENGTH + 600) {
            birds[i].x = -600 - rand() % 300;
            birds[i].y = 370 + rand() % 140;
            birds[i].speed = 0.7f + (rand() % 10) / 10.0f;
            birds[i].wingPhase = (rand() % 100) / 100.0f * 2.0f * 3.14159f;
        }
    }
}
void generateLightningPath() {
    lightningSegments = LIGHTNING_SEGMENTS;

    float startX = WIDTH * 0.2f + rand() % (int)(WIDTH * 0.6f);
    float startY = HEIGHT;

    lightningX[0] = startX;
    lightningY[0] = startY;

    for (int i = 1; i <= lightningSegments; i++) {
        float prevX = lightningX[i - 1];
        float prevY = lightningY[i - 1];

        float dx = (rand() % 61) - 30;
        float dy = 28 + rand() % 30;

        lightningX[i] = prevX + dx;
        lightningY[i] = prevY - dy;

        if (lightningY[i] < HEIGHT * 0.45f) {
            lightningSegments = i;
            break;
        }
    }
}

void updateThunder() {
    if (!isRaining) {
        thunderActive = 0;
        thunderTimer = 0;
        thunderGapTimer = 0;
        thunderBlinkCount = 0;
        thunderBlinksLeft = 0;
        lightningSegments = 0;
        thunderCooldown = THUNDER_COOLDOWN_MIN +
                          rand() % (THUNDER_COOLDOWN_MAX - THUNDER_COOLDOWN_MIN + 1);
        return;
    }

    if (thunderActive) {
        thunderTimer--;
        if (thunderTimer <= 0) {
            thunderActive = 0;

            if (thunderBlinksLeft > 0) {
                thunderGapTimer = THUNDER_GAP_TIME;
            } else {
                thunderCooldown = THUNDER_COOLDOWN_MIN +
                                  rand() % (THUNDER_COOLDOWN_MAX - THUNDER_COOLDOWN_MIN + 1);
            }
        }
        return;
    }

    if (thunderGapTimer > 0) {
        thunderGapTimer--;
        if (thunderGapTimer <= 0 && thunderBlinksLeft > 0) {
            thunderActive = 1;
            thunderTimer = THUNDER_FLASH_TIME;
            thunderBlinksLeft--;
        }
        return;
    }

    thunderCooldown--;
    if (thunderCooldown <= 0) {
        thunderBlinkCount = THUNDER_MIN_BLINKS +
            rand() % (THUNDER_MAX_BLINKS - THUNDER_MIN_BLINKS + 1);

        thunderBlinksLeft = thunderBlinkCount - 1;
        generateLightningPath();

        thunderActive = 1;
        thunderTimer = THUNDER_FLASH_TIME;
    }
}
