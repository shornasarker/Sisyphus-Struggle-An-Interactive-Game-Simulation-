#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "constants.h"
#include "structures.h"
#include "globals.h"
#include "graphics.h"
#include "physics.h"
#include "weather.h"
#include "game.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

Rock rock;
Man man;
Obstacle obstacles[MAX_OBSTACLES];
Particle particles[100];
Collectible collectibles[MAX_COLLECTIBLES];

int numObstacles = 0;
int numParticles = 0;
int numCollectibles = 0;

float cameraX = 0;
float cameraY = 0;
float cameraZoom = 1.0f;
float targetZoom = 1.0f;
float targetCameraY = 0;
float mountainBase = 80;
float mountainTop = 550;
int windowWidth = WIDTH;
int windowHeight = HEIGHT;
int gameMode = MODE_PLAYING;
float modeTimer = 0;
int gameActive = 1;
int restartRequested = 0;
int summitReached = 0;

float wind = 0.0f;
float timeOfDay = 0.25f;
int isRaining = 0;

int thunderActive = 0;
int thunderTimer = 0;
int thunderCooldown = 300;

int thunderBlinkCount = 0;
int thunderBlinksLeft = 0;
int thunderGapTimer = 0;

int lightningSegments = 0;
float lightningX[LIGHTNING_SEGMENTS + 1];
float lightningY[LIGHTNING_SEGMENTS + 1];

Cloud clouds[MAX_CLOUDS];
Star stars[MAX_STARS];
Bird birds[MAX_BIRDS];

int moveLeftPressed = 0;
int moveRightPressed = 0;

void generateObstacles(void);
void generateCollectibles(void);
void initGame(void);
void forceRestart(void);
void update(int value);
void handleKeyboard(unsigned char key, int x, int y);
void handleKeyboardUp(unsigned char key, int x, int y);
void handleSpecialKeys(int key, int x, int y);
void handleSpecialKeysUp(int key, int x, int y);
void display(void);
void reshape(int w, int h);

void generateObstacles(void)
{
    int i, j;
    float xPos;
    int valid;

    numObstacles = 10;

    for (i = 0; i < numObstacles; i++)
    {
        do
        {
            valid = 1;
            xPos = 500 + rand() % 4000;

            for (j = 0; j < i; j++)
            {
                if (fabs(xPos - obstacles[j].x) < 300)
                {
                    valid = 0;
                    break;
                }
            }
        }
        while (!valid);

        obstacles[i].x = xPos;
        obstacles[i].size = 0.6f + (rand() % 6) / 10.0f;
        obstacles[i].type = rand() % 2;
        obstacles[i].active = 1;
        obstacles[i].alreadyHit = 0;

        if (obstacles[i].type == 1 && rand() % 3 != 0)
            obstacles[i].type = 0;
    }
}

void generateCollectibles(void)
{
    int i, j, valid;
    float xPos;

    numCollectibles = 12;

    for (i = 0; i < numCollectibles; i++)
    {
        do
        {
            valid = 1;
            xPos = 350 + rand() % 4200;

            for (j = 0; j < i; j++)
            {
                if (fabs(xPos - collectibles[j].x) < 250)
                {
                    valid = 0;
                    break;
                }
            }

            for (j = 0; j < numObstacles; j++)
            {
                if (j < MAX_OBSTACLES && obstacles[j].active)
                {
                    if (fabs(xPos - obstacles[j].x) < 120)
                    {
                        valid = 0;
                        break;
                    }
                }
            }
        }
        while (!valid);

        collectibles[i].x = xPos;
        collectibles[i].y = getMountainY(xPos) + 22.0f;

        {
            int r = rand() % 100;
            if (r < 50) collectibles[i].type = COLLECT_ENERGY;
            else if (r < 70) collectibles[i].type = COLLECT_SHIELD;
            else if (r < 85) collectibles[i].type = COLLECT_LIFE;
            else collectibles[i].type = COLLECT_SPEED;
        }

        collectibles[i].active = 1;
        collectibles[i].bobPhase = 0.0f;
    }
}

void initGame(void)
{
    srand((unsigned int)time(NULL));

    mountainBase = 80;
    mountainTop = 550;

    man.x = 200;
    man.y = getMountainY(200) + 5;
    man.vx = 0;
    man.vy = 0;
    man.width = MAN_WIDTH;
    man.height = MAN_HEIGHT;
    man.isJumping = 0;
    man.canJump = 1;
    man.score = 0;
    man.lives = 5;
    man.invincible = 0;
    man.invincibleTimer = 0;

    man.shieldActive = 0;
    man.shieldTimer = 0;
    man.speedBoostActive = 0;
    man.speedBoostTimer = 0;
    man.energy = 0;
    man.resistingSlip = 0;

    rock.x = man.x + 30;
    rock.y = getMountainY(rock.x) + ROCK_RADIUS;
    rock.vx = 0;
    rock.vy = 0;
    rock.radius = ROCK_RADIUS;
    rock.isRolling = 0;
    rock.rotation = 0;

    generateObstacles();
    generateCollectibles();

    initClouds();
    initStars();
    initBirds();

    thunderActive = 0;
    thunderTimer = 0;
    thunderCooldown = THUNDER_COOLDOWN_MIN +
                      rand() % (THUNDER_COOLDOWN_MAX - THUNDER_COOLDOWN_MIN + 1);
    thunderBlinkCount = 0;
    thunderBlinksLeft = 0;
    thunderGapTimer = 0;
    lightningSegments = 0;

    cameraX = 0;
    cameraY = 0;
    cameraZoom = ZOOM_NORMAL;
    targetZoom = ZOOM_NORMAL;
    targetCameraY = 0;

    moveLeftPressed = 0;
    moveRightPressed = 0;

    gameMode = MODE_PLAYING;
    modeTimer = 0;
    gameActive = 1;
    restartRequested = 0;
    summitReached = 0;

    wind = (rand() % 20 - 10) / 30.0f;
    timeOfDay = 0.25f;
    isRaining = 0;
}

void forceRestart(void)
{
    int i;

    mountainBase = 80;
    mountainTop = 550;

    man.x = 200;
    man.y = getMountainY(200) + 5;
    man.vx = 0;
    man.vy = 0;
    man.width = MAN_WIDTH;
    man.height = MAN_HEIGHT;
    man.isJumping = 0;
    man.canJump = 1;
    man.score = 0;
    man.lives = 5;
    man.invincible = 0;
    man.invincibleTimer = 0;

    man.shieldActive = 0;
    man.shieldTimer = 0;
    man.speedBoostActive = 0;
    man.speedBoostTimer = 0;
    man.energy = 0;
    man.resistingSlip = 0;

    rock.x = man.x + 30;
    rock.y = getMountainY(rock.x) + ROCK_RADIUS;
    rock.vx = 0;
    rock.vy = 0;
    rock.isRolling = 0;
    rock.rotation = 0;
    rock.radius = ROCK_RADIUS;

    thunderActive = 0;
    thunderTimer = 0;
    thunderCooldown = THUNDER_COOLDOWN_MIN +
                      rand() % (THUNDER_COOLDOWN_MAX - THUNDER_COOLDOWN_MIN + 1);
    thunderBlinkCount = 0;
    thunderBlinksLeft = 0;
    thunderGapTimer = 0;
    lightningSegments = 0;

    for (i = 0; i < MAX_OBSTACLES; i++)
    {
        obstacles[i].active = 0;
        obstacles[i].alreadyHit = 0;
    }
    numObstacles = 0;
    generateObstacles();

    for (i = 0; i < 100; i++)
        particles[i].active = 0;
    numParticles = 0;

    for (i = 0; i < MAX_COLLECTIBLES; i++)
        collectibles[i].active = 0;
    numCollectibles = 0;
    generateCollectibles();

    cameraX = 0;
    cameraY = 0;
    cameraZoom = ZOOM_NORMAL;
    targetZoom = ZOOM_NORMAL;
    targetCameraY = 0;

    moveLeftPressed = 0;
    moveRightPressed = 0;

    gameMode = MODE_PLAYING;
    modeTimer = 0;
    gameActive = 1;
    restartRequested = 0;
    summitReached = 0;

    wind = (rand() % 20 - 10) / 30.0f;
    timeOfDay = 0.25f;
    isRaining = 0;

    glutPostRedisplay();
}

void update(int value)
{
    if (gameActive)
    {
        if (modeTimer > 0)
            modeTimer--;

        /* Game stays on GAME OVER screen until the player presses R */

        if (gameMode == MODE_PLAYING)
        {
            float desiredGap = 6.0f;

            updateDayNight();
            updateClouds();
            updateBirds();
            updateThunder();
            updateParticles();

            if (moveRightPressed)
            {
                float moveSpeed = man.speedBoostActive ? 3.2f : 2.2f;

                /* If man reaches rock, push rock too */
                if (man.x + man.width + desiredGap >= rock.x)
                {
                    rock.x += moveSpeed * 0.9f;
                    rock.vx = moveSpeed * 0.35f;

                    man.x = rock.x - man.width - desiredGap;
                }
                else
                {
                    man.x += moveSpeed;
                }

                if (man.x > MOUNTAIN_LENGTH - 120)
                    man.x = MOUNTAIN_LENGTH - 120;

                if (rock.x > MOUNTAIN_LENGTH - 50)
                    rock.x = MOUNTAIN_LENGTH - 50;
            }

            if (moveLeftPressed)
            {
                float backSpeed = 1.6f;
                man.x -= backSpeed;

                if (man.x < 50)
                    man.x = 50;
            }

            checkCollisions();
            checkSummitReached();
            updateCamera();

            if (rand() % 800 == 0)
                wind = (rand() % 20 - 10) / 30.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void handleKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;

    case 'r':
    case 'R':
        forceRestart();
        return;

    case ' ':
        if (gameMode == MODE_PLAYING)
            jump();
        break;

    case 's':
    case 'S':
        if (gameMode == MODE_PLAYING)
            man.resistingSlip = 1;
        break;

    case 'w':
    case 'W':
        if (gameMode == MODE_PLAYING)
            isRaining = !isRaining;
        break;

    case 'a':
    case 'A':
        if (gameMode == MODE_PLAYING)
            moveLeftPressed = 1;
        break;

    case 'd':
    case 'D':
        if (gameMode == MODE_PLAYING)
            moveRightPressed = 1;
        break;
    }
}

void handleKeyboardUp(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 's':
    case 'S':
        man.resistingSlip = 0;
        break;

    case 'a':
    case 'A':
        moveLeftPressed = 0;
        break;

    case 'd':
    case 'D':
        moveRightPressed = 0;
        break;
    }
}

void handleSpecialKeys(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_F1:
        forceRestart();
        return;

    case GLUT_KEY_LEFT:
        if (gameMode == MODE_PLAYING)
            moveLeftPressed = 1;
        break;

    case GLUT_KEY_RIGHT:
        if (gameMode == MODE_PLAYING)
            moveRightPressed = 1;
        break;
    }
}

void handleSpecialKeysUp(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_LEFT:
        moveLeftPressed = 0;
        break;

    case GLUT_KEY_RIGHT:
        moveRightPressed = 0;
        break;
    }
}

void display(void)
{
    drawSky();

    drawSun();
    drawMoon();
    drawStars();
    drawClouds();
    drawBirds();

    drawFarMountains();
    drawRiver();          /* river between mountains and village */
    drawGrassLand();

    drawBackgroundTrees();
    drawVillage();
    drawTrees();

    drawMountain();
    drawForegroundGrass();
    drawObstacles();
    drawSummitMarker();
    drawCollectibles();
    drawRock();
    drawMan();
    drawParticles();

    drawRain();
    drawLightning();
    drawThunderFlash();

    drawUI();

    if (gameMode == MODE_GAMEOVER)
    {
        char scoreText[80];

        glColor3f(1.0f, 0.15f, 0.15f);
        drawText(WIDTH / 2 - 70, HEIGHT / 2 + 10, "GAME OVER");

        sprintf(scoreText, "Final Score: %d", man.score);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(WIDTH / 2 - 85, HEIGHT / 2 - 20, scoreText);

        glColor3f(1.0f, 0.85f, 0.2f);
        drawText(WIDTH / 2 - 105, HEIGHT / 2 - 55, "PRESS R TO RESTART");
    }

    if (gameMode == MODE_SUMMIT)
    {
        glColor3f(0.2f, 1.0f, 0.2f);
        drawText(WIDTH / 2 - 155, HEIGHT / 2 + 10, "Congrats! Heaven Is Near!");
        drawText(WIDTH / 2 - 175, HEIGHT / 2 - 20, "Press R To Start Again!!");
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Sisyphus: Rock Pusher");

    glutFullScreen();

    glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);

    initGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyboard);
    glutKeyboardUpFunc(handleKeyboardUp);
    glutSpecialFunc(handleSpecialKeys);
    glutSpecialUpFunc(handleSpecialKeysUp);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
