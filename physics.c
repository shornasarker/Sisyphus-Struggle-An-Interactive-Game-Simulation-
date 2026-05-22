#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "physics.h"
#include "globals.h"
#include "graphics.h"

#ifdef _WIN32
#include <windows.h>
#endif

void playObstacleSound() {
#ifdef _WIN32
    MessageBeep(MB_ICONHAND);
#else
    printf("\a");
    fflush(stdout);
#endif
}

void playCoinSound() {
    /* sound disabled */
}
float getMountainY(float x)
{
    /*
     * Mountain profile matching the steep red-line drawing:
     *
     *  Zone 0: flat start          x in [0,   600)   — gentle low ground
     *  Zone 1: early gentle rise   x in [600, 1800)  — slow ramp-up
     *  Zone 2: main steep climb    x in [1800,3800)  — the big long slope
     *  Zone 3: final push          x in [3800,SUMMIT_X-130) — even steeper
     *  Zone 4: narrow summit top   x in [SUMMIT_X-130, SUMMIT_X+180]
     *  Zone 5: descent             x beyond summit
     *
     * Small sinusoidal bumps are added throughout for surface texture.
     */

    const float flatEnd       = 600.0f;
    const float rampStart     = flatEnd;
    const float climbStart    = 1800.0f;
    const float climbEnd      = 3800.0f;
    const float finalStart    = climbEnd;
    const float hillTopStart  = SUMMIT_X - 130.0f;
    const float hillTopEnd    = SUMMIT_X + 180.0f;
    const float downEnd       = SUMMIT_X + 900.0f;

    /* Heights (world-Y units) at key transitions */
    const float yFlat         = 30.0f;   /* ground level at start          */
    const float yAfterRamp    = 60.0f;   /* height after gentle early ramp  */
    const float yAfterClimb   = 310.0f;  /* height after the main slope     */
    const float yAtSummit     = 430.0f;  /* peak height                     */

    float t, ease, baseY;

    /* small surface bumps – same across all zones for consistency */
    float bump = 4.0f  * sin(x * 0.045f + 2.0f)
               + 2.5f  * sin(x * 0.080f - 1.0f);

    if (x < 0) x = 0;
    if (x > MOUNTAIN_LENGTH) x = MOUNTAIN_LENGTH;

    /* ── Zone 0: flat start ── */
    if (x < flatEnd)
        return yFlat + bump;

    /* ── Zone 1: gentle early ramp (600 → 1800) ── */
    if (x < climbStart)
    {
        t = (x - rampStart) / (climbStart - rampStart);
        baseY = yFlat + t * (yAfterRamp - yFlat);
        return baseY + bump;
    }

    /* ── Zone 2: main steep climb (1800 → 3800) ── */
    if (x < climbEnd)
    {
        t    = (x - climbStart) / (climbEnd - climbStart);
        ease = t * t * (3.0f - 2.0f * t);          /* smooth-step */
        baseY = yAfterRamp + ease * (yAfterClimb - yAfterRamp);
        return baseY + bump;
    }

    /* ── Zone 3: final steep push (3800 → hillTopStart) ── */
    if (x < hillTopStart)
    {
        t    = (x - finalStart) / (hillTopStart - finalStart);
        ease = t * t * (3.0f - 2.0f * t);
        baseY = yAfterClimb + ease * (yAtSummit - yAfterClimb);
        return baseY + bump;
    }

    /* ── Zone 4: narrow summit plateau ── */
    if (x <= hillTopEnd)
    {
        t = (x - hillTopStart) / (hillTopEnd - hillTopStart);
        return yAtSummit + 4.0f * sin(t * 3.14159f) + bump;
    }

    /* ── Zone 5: descent after summit ── */
    if (x <= downEnd)
    {
        t = (x - hillTopEnd) / (downEnd - hillTopEnd);
        ease = t * t * (3.0f - 2.0f * t);
        return yAtSummit - ease * 200.0f + 3.0f * sin(t * 2.0f * 3.14159f) + bump;
    }

    /* beyond descent – flat low ground */
    return yFlat - 40.0f + bump;
}
float getMountainSlope(float x) {
    float dx = 1.0f;
    return (getMountainY(x + dx) - getMountainY(x - dx)) / (2 * dx);
}

float getLightIntensity() {
    return 0.5f + 0.5f * sin(timeOfDay * 2 * 3.14159f);
}

void spawnParticles(float x, float y, int count) {
    for (int i = 0; i < count && numParticles < 100; i++) {
        particles[numParticles].x = x;
        particles[numParticles].y = y;
        particles[numParticles].vx = (rand() % 10 - 5) * 0.3f;
        particles[numParticles].vy = (rand() % 10) * 0.3f;
        particles[numParticles].life = 20 + rand() % 20;
        particles[numParticles].active = 1;
        numParticles++;
    }
}

void updateParticles() {
    for (int i = 0; i < numParticles; i++) {
        if (!particles[i].active) continue;

        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy -= 0.1f;
        particles[i].life--;

        if (particles[i].life <= 0) {
            particles[i].active = 0;
        }
    }
}
void updateCamera() {
    float targetOffset = WIDTH * 0.30f;   /* keep man a bit left of center */
    float desiredCameraX = man.x - targetOffset;
    float followSpeed = 0.05f;            /* softer follow so man visibly moves */

    if (desiredCameraX < 0)
        desiredCameraX = 0;

    if (desiredCameraX > MOUNTAIN_LENGTH - WIDTH)
        desiredCameraX = MOUNTAIN_LENGTH - WIDTH;

    /* During rain slip, do not drag camera backward with the man */
    if (isRaining && !man.resistingSlip && desiredCameraX < cameraX) {
        return;
    }

    cameraX += (desiredCameraX - cameraX) * followSpeed;

    /*
     * Track the mountain's vertical rise so the ground doesn't
     * scroll off screen as the slope gets steep.
     * Keep the man's feet at roughly 22% up from the bottom.
     */
    {
        float groundAtStart  = getMountainY(0.0f);
        float groundAtCamera = getMountainY(cameraX + targetOffset);
        float rise = groundAtCamera - groundAtStart;
        float desiredCameraY = rise * 0.88f;   /* foreground moves with camera */

        if (desiredCameraY < 0.0f) desiredCameraY = 0.0f;

        cameraY += (desiredCameraY - cameraY) * followSpeed;
    }
}
void jump() {
    if (gameMode != MODE_PLAYING) return;

    if (man.canJump && !man.isJumping) {
        man.vy = JUMP_FORCE;
        man.isJumping = 1;
        man.canJump = 0;
    }
}

void checkSummitReached() {
    float manCenterX;
    const float triggerStart = SUMMIT_X - 18.0f;
    const float triggerEnd   = SUMMIT_X + 22.0f;

    if (gameMode != MODE_PLAYING || summitReached) return;

    manCenterX = man.x + man.width * 0.5f;

    if (manCenterX >= triggerStart && manCenterX <= triggerEnd) {
        summitReached = 1;
        triggerSummit();
    }
}

void triggerSummit() {
    if (gameMode != MODE_PLAYING) return;

    gameMode = MODE_SUMMIT;
    summitReached = 1;

    man.vx = 0.0f;
    man.vy = 0.0f;
    man.isJumping = 0;
    man.canJump = 0;

    rock.vx = 0.0f;
    rock.vy = 0.0f;
    rock.isRolling = 0;
}

void updateBoosts() {
    if (man.shieldActive) {
        man.shieldTimer--;
        if (man.shieldTimer <= 0) {
            man.shieldActive = 0;
        }
    }

    if (man.speedBoostActive) {
        man.speedBoostTimer--;
        if (man.speedBoostTimer <= 0) {
            man.speedBoostActive = 0;
        }
    }

    if (man.invincible) {
        man.invincibleTimer--;
        if (man.invincibleTimer <= 0) {
            man.invincible = 0;
        }
    }
}

void checkCollectibles() {
    int i;
    float manCenterX = man.x + man.width * 0.5f;
    float manCenterY = man.y + man.height * 0.5f;

    for (i = 0; i < numCollectibles; i++) {
        float dx, dy, distSq;

        if (!collectibles[i].active) continue;

        dx = collectibles[i].x - manCenterX;
        dy = collectibles[i].y - manCenterY;
        distSq = dx * dx + dy * dy;

        if (distSq < 28.0f * 28.0f) {
            collectibles[i].active = 0;
            playCoinSound();
            spawnParticles(collectibles[i].x, collectibles[i].y, 12);

            if (collectibles[i].type == COLLECT_ENERGY) {
                man.score += 1;
                man.energy += 10;
                if (man.energy > 50) man.energy = 50;
            }
            else if (collectibles[i].type == COLLECT_LIFE) {
                if (man.lives < 5) man.lives++;
            }
            else if (collectibles[i].type == COLLECT_SHIELD) {
                man.shieldActive = 1;
                man.shieldTimer = SHIELD_DURATION;
            }
            else if (collectibles[i].type == COLLECT_SPEED) {
                man.speedBoostActive = 1;
                man.speedBoostTimer = SPEEDBOOST_DURATION;
            }
        }
    }

    if (man.energy > 50) {
        man.energy = 50;
    }
}

void checkCollisions() {
    int i;
    float desiredGap = 6.0f;
    float groundY;
    float manFront;
    float manCenterX;
    float manFeetY;

    if (gameMode != MODE_PLAYING) return;

    updateBoosts();

    man.vy -= GRAVITY;
    man.y += man.vy;

    groundY = getMountainY(man.x) + 5.0f;

    if (man.y <= groundY) {
        man.y = groundY;
        man.vy = 0.0f;
        man.isJumping = 0;
        man.canJump = 1;
    }

    rock.x += rock.vx;
    rock.vx *= 0.85f;

    manFront = man.x + man.width;

    if (manFront >= rock.x - desiredGap) {
        rock.x = manFront + desiredGap;
        rock.vx = 0.0f;
    }

    if (rock.x < man.x + man.width + desiredGap) {
        rock.x = man.x + man.width + desiredGap;
    }

    /* Rain slip */
    if (isRaining) {
        float slipPower = 0.20f;

        if (!man.resistingSlip) {
            man.x -= slipPower;
            rock.x -= slipPower;
        } else {
            if (man.energy > 0) man.energy -= 1;
            if (man.energy < 0) man.energy = 0;
        }
    }

    if (man.x < 50) man.x = 50;

    if (rock.x < man.x + man.width + desiredGap) {
        rock.x = man.x + man.width + desiredGap;
    }

    if (rock.x > MOUNTAIN_LENGTH - 50) {
        rock.x = MOUNTAIN_LENGTH - 50;
        if (man.x > rock.x - man.width - desiredGap)
            man.x = rock.x - man.width - desiredGap;
    }

    man.y = fmaxf(man.y, getMountainY(man.x) + 5.0f);
    rock.y = getMountainY(rock.x) + ROCK_RADIUS;

    manCenterX = man.x + man.width * 0.5f;
    manFeetY = man.y;

    for (i = 0; i < numObstacles; i++) {
        float dx;

        if (!obstacles[i].active || obstacles[i].alreadyHit) continue;

        dx = fabs(manCenterX - obstacles[i].x);

        if (obstacles[i].type == 0) {
            float rockSize = 8.0f * obstacles[i].size;
            float hitWidth = rockSize + man.width * 0.45f;
            float safeHeight = getMountainY(obstacles[i].x) + rockSize * 1.8f;

            if (dx < hitWidth) {
                if (manFeetY < safeHeight) {
                    if (!man.shieldActive && !man.invincible) {
                        man.lives--;
                        man.invincible = 1;
                        man.invincibleTimer = 60;
                        playObstacleSound();
                        spawnParticles(obstacles[i].x, getMountainY(obstacles[i].x) + 15.0f, 14);
                    }
                    obstacles[i].alreadyHit = 1;
                }
            }
        }
        else {
            float pitHalfWidth = 20.0f * obstacles[i].size;

            if (dx < pitHalfWidth) {
                if (manFeetY <= getMountainY(man.x) + 12.0f) {
                    if (!man.shieldActive && !man.invincible) {
                        man.lives--;
                        man.invincible = 1;
                        man.invincibleTimer = 60;
                        playObstacleSound();
                        spawnParticles(obstacles[i].x, getMountainY(obstacles[i].x), 14);
                    }
                    obstacles[i].alreadyHit = 1;
                }
            }
        }
    }

    checkCollectibles();

    if (man.lives <= 0) {
        man.lives = 0;
        gameMode = MODE_GAMEOVER;
        modeTimer = 180;
    }
}
