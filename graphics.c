#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "globals.h"
#include "structures.h"
#include "graphics.h"
#include "physics.h"

#define OBSTACLE_ROCK   0
#define OBSTACLE_PIT    1
#define OBSTACLE_SPIKE  2
#define OBSTACLE_FIRE   3

float getBackgroundDrop(void);

float getDistantLandY(float worldX)
{
    return 200.0f
           + sin(worldX * 0.0010f) * 18.0f
           + cos(worldX * 0.00050f) * 12.0f;
}

/* ===== BASIC DRAWING ===== */
void drawLineDDA(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float x = x1, y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++)
    {
        glVertex2i(round(x), round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

void drawLineBresenham(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while (1)
    {
        glVertex2i(x1, y1);
        if (x1 == x2 && y1 == y2) break;

        {
            int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y1 += sy;
            }
        }
    }
    glEnd();
}

void drawCircleMidpoint(int xc, int yc, int r)
{
    int x = 0;
    int y = r;
    int p = 1 - r;

    glBegin(GL_POINTS);
    while (x <= y)
    {
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);
        glVertex2i(xc + y, yc + x);
        glVertex2i(xc - y, yc + x);
        glVertex2i(xc + y, yc - x);
        glVertex2i(xc - y, yc - x);

        x++;
        if (p < 0)
        {
            p += 2 * x + 1;
        }
        else
        {
            y--;
            p += 2 * (x - y) + 1;
        }
    }
    glEnd();
}

void drawEllipseMidpoint(int xc, int yc, int rx, int ry)
{
    int x = 0;
    int y = ry;
    float d1 = (ry * ry) - (rx * rx * ry) + (0.25f * rx * rx);
    int dx = 2 * ry * ry * x;
    int dy = 2 * rx * rx * y;

    glBegin(GL_POINTS);
    while (dx < dy)
    {
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);

        x++;
        dx = 2 * ry * ry * x;
        if (d1 < 0)
        {
            d1 += dx + ry * ry;
        }
        else
        {
            y--;
            dy = 2 * rx * rx * y;
            d1 += dx - dy + ry * ry;
        }
    }

    {
        float d2 = (ry * ry) * (x + 0.5f) * (x + 0.5f) +
                   (rx * rx) * (y - 1) * (y - 1) -
                   (rx * rx * ry * ry);

        while (y >= 0)
        {
            glVertex2i(xc + x, yc + y);
            glVertex2i(xc - x, yc + y);
            glVertex2i(xc + x, yc - y);
            glVertex2i(xc - x, yc - y);

            y--;
            dy = 2 * rx * rx * y;
            if (d2 > 0)
            {
                d2 += rx * rx - dy;
            }
            else
            {
                x++;
                dx = 2 * ry * ry * x;
                d2 += dx - dy + rx * rx;
            }
        }
    }
    glEnd();
}

void drawRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawCircle(float cx, float cy, float r, int segments)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++)
    {
        float angle = i * 2.0f * 3.14159f / segments;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawText(float x, float y, char *string)
{
    glRasterPos2f(x, y);
    for (char *c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

/* ===== SKY ===== */
void drawSky()
{
    float t = timeOfDay;

    float rTop, gTop, bTop;
    float rMid, gMid, bMid;
    float rLow, gLow, bLow;

    if (t < 0.3f)
    {
        rTop = 0.01f;
        gTop = 0.03f;
        bTop = 0.10f;
        rMid = 0.05f;
        gMid = 0.10f;
        bMid = 0.20f;
        rLow = 0.16f;
        gLow = 0.18f;
        bLow = 0.24f;
    }
    else if (t < 0.7f)
    {
        rTop = 0.16f;
        gTop = 0.40f;
        bTop = 0.82f;
        rMid = 0.34f;
        gMid = 0.58f;
        bMid = 0.90f;
        rLow = 0.78f;
        gLow = 0.86f;
        bLow = 0.92f;
    }
    else
    {
        rTop = 0.06f;
        gTop = 0.12f;
        bTop = 0.28f;
        rMid = 0.18f;
        gMid = 0.26f;
        bMid = 0.44f;
        rLow = 0.42f;
        gLow = 0.44f;
        bLow = 0.52f;
    }

    if (thunderActive)
    {
        float flash = 0.18f;
        rTop += flash;
        gTop += flash;
        bTop += flash;
        rMid += flash;
        gMid += flash;
        bMid += flash;
        rLow += flash;
        gLow += flash;
        bLow += flash;

        if (rTop > 1.0f) rTop = 1.0f;
        if (gTop > 1.0f) gTop = 1.0f;
        if (bTop > 1.0f) bTop = 1.0f;
        if (rMid > 1.0f) rMid = 1.0f;
        if (gMid > 1.0f) gMid = 1.0f;
        if (bMid > 1.0f) bMid = 1.0f;
        if (rLow > 1.0f) rLow = 1.0f;
        if (gLow > 1.0f) gLow = 1.0f;
        if (bLow > 1.0f) bLow = 1.0f;
    }

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    glColor3f(rLow, gLow, bLow);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);

    glColor3f(rTop, gTop, bTop);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();

    glBegin(GL_QUADS);
    glColor4f(rMid, gMid, bMid, 0.35f);
    glVertex2f(0, windowHeight * 0.28f);
    glVertex2f(windowWidth, windowHeight * 0.28f);
    glColor4f(rTop, gTop, bTop, 0.0f);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();

    glBegin(GL_QUADS);
    glColor4f(1.0f, 0.95f, 0.82f, 0.08f);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glColor4f(1.0f, 0.95f, 0.82f, 0.0f);
    glVertex2f(windowWidth, 220);
    glVertex2f(0, 220);
    glEnd();
}
void drawSun()
{
    float sunAngle = (timeOfDay - 0.5f) * 3.14159f;
    float sunX = (sin(sunAngle) + 1) / 2 * WIDTH;

    float progress = (sin(sunAngle) + 1) / 2;
    float maxHeight = HEIGHT * 0.46f;
    float sunY = 4 * maxHeight * progress * (1 - progress) + 145;

    if (timeOfDay > 0.2f && timeOfDay < 0.8f)
    {
        int i;
        float coreR = 26.0f;
        float rayInner = 34.0f;
        float rayOuter = 52.0f;

        glColor3f(1.0f, 0.78f, 0.08f);
        for (i = 0; i < 20; i++)
        {
            float a1 = (i * 18.0f - 4.0f) * 3.14159f / 180.0f;
            float a2 = (i * 18.0f + 4.0f) * 3.14159f / 180.0f;
            float am = (i * 18.0f) * 3.14159f / 180.0f;

            glBegin(GL_TRIANGLES);
            glVertex2f(sunX + cos(a1) * rayInner, sunY + sin(a1) * rayInner);
            glVertex2f(sunX + cos(a2) * rayInner, sunY + sin(a2) * rayInner);
            glVertex2f(sunX + cos(am) * rayOuter, sunY + sin(am) * rayOuter);
            glEnd();
        }

        glColor4f(1.0f, 0.85f, 0.20f, 0.18f);
        drawCircle(sunX, sunY, 36, 40);

        glColor3f(1.0f, 0.88f, 0.12f);
        drawCircle(sunX, sunY, coreR, 36);

        glColor3f(1.0f, 0.94f, 0.35f);
        drawCircle(sunX - 4, sunY + 4, 18, 28);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCircle(sunX - 8, sunY + 5, 4.5f, 16);
        drawCircle(sunX + 8, sunY + 5, 4.5f, 16);

        glColor3f(0.25f, 0.65f, 1.0f);
        drawCircle(sunX - 8, sunY + 5, 2.2f, 12);
        drawCircle(sunX + 8, sunY + 5, 2.2f, 12);

        glColor3f(0.05f, 0.05f, 0.05f);
        drawCircle(sunX - 8, sunY + 5, 1.0f, 10);
        drawCircle(sunX + 8, sunY + 5, 1.0f, 10);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCircle(sunX - 7.2f, sunY + 6.0f, 0.6f, 8);
        drawCircle(sunX + 8.8f, sunY + 6.0f, 0.6f, 8);

        glColor3f(0.85f, 0.55f, 0.05f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        if (isRaining)
        {
            glVertex2f(sunX - 12, sunY + 13);
            glVertex2f(sunX - 5,  sunY + 10);

            glVertex2f(sunX + 5,  sunY + 10);
            glVertex2f(sunX + 12, sunY + 13);
        }
        else
        {
            glVertex2f(sunX - 12, sunY + 11);
            glVertex2f(sunX - 5,  sunY + 13);

            glVertex2f(sunX + 5,  sunY + 13);
            glVertex2f(sunX + 12, sunY + 11);
        }
        glEnd();

        glColor3f(0.75f, 0.20f, 0.12f);
        glLineWidth(3.0f);
        glBegin(GL_LINE_STRIP);
        if (isRaining)
        {
            glVertex2f(sunX - 10, sunY - 12);
            glVertex2f(sunX - 6,  sunY - 8);
            glVertex2f(sunX,      sunY - 6);
            glVertex2f(sunX + 6,  sunY - 8);
            glVertex2f(sunX + 10, sunY - 12);
        }
        else
        {
            glVertex2f(sunX - 10, sunY - 6);
            glVertex2f(sunX - 6,  sunY - 10);
            glVertex2f(sunX,      sunY - 12);
            glVertex2f(sunX + 6,  sunY - 10);
            glVertex2f(sunX + 10, sunY - 6);
        }
        glEnd();
    }
}
void drawMoon()
{
    /*
     * Mirror the sun's orbit exactly, offset by half a day.
     * Sun:  visible during timeOfDay in (0.2, 0.8), arc angle = (t - 0.5) * PI
     * Moon: visible during timeOfDay outside (0.2, 0.8), i.e. the night half.
     *       We shift timeOfDay by 0.5 (wrapping) so the moon uses the
     *       identical arc formula, rising from the left and setting on the right.
     */
    float t;
    float moonAngle;
    float moonX, moonY;
    float progress;
    float maxHeight = HEIGHT * 0.46f;   /* same arc height as the sun */

    /* Only draw during night (outside sun window with small overlap for fade) */
    if (timeOfDay >= 0.22f && timeOfDay <= 0.78f) return;

    /* Shift time by 0.5, wrapping to [0,1] */
    t = timeOfDay + 0.5f;
    if (t >= 1.0f) t -= 1.0f;

    /* Identical formula to the sun */
    moonAngle = (t - 0.5f) * 3.14159f;
    moonX     = (sin(moonAngle) + 1.0f) / 2.0f * WIDTH;
    progress  = (sin(moonAngle) + 1.0f) / 2.0f;
    moonY     = 4.0f * maxHeight * progress * (1.0f - progress) + 145;

    glColor4f(0.8f, 0.8f, 1.0f, 0.2f);
    drawCircle(moonX, moonY, 40, 30);

    glColor3f(0.95f, 0.95f, 1.0f);
    drawCircle(moonX, moonY, 20, 30);

    glColor3f(0.7f, 0.7f, 0.8f);
    drawCircle(moonX - 5, moonY + 4,  4,   8);
    drawCircle(moonX + 6, moonY - 5,  3.5f, 8);
    drawCircle(moonX + 3, moonY + 8,  3,   6);
}

void drawStars()
{
    if (timeOfDay > 0.30f && timeOfDay < 0.70f) return;

    {
        float nightIntensity = 1.0f;
        float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;  /* real time in seconds */

        if (timeOfDay < 0.30f)
            nightIntensity = 1.0f - (timeOfDay / 0.30f);
        else if (timeOfDay > 0.70f)
            nightIntensity = (timeOfDay - 0.70f) / 0.30f;

        if (nightIntensity < 0.0f) nightIntensity = 0.0f;
        if (nightIntensity > 1.0f) nightIntensity = 1.0f;

        for (int i = 0; i < MAX_STARS; i++)
        {
            /* Each star has its own blink speed (1.5–5 Hz range) driven by twinkle phase */
            float blinkFreq  = 1.5f + stars[i].twinkle * 3.5f;
            float blinkPhase = stars[i].twinkle * 6.28318f;  /* unique offset per star */
            float blink      = 0.55f + 0.45f * sin(t * blinkFreq * 6.28318f + blinkPhase);

            /* Base brightness raised: range now 0.75–1.0 instead of 0.3–1.0 */
            float brightness = (0.75f + stars[i].brightness * 0.25f) * nightIntensity * blink;

            float sx = stars[i].x;
            float sy = stars[i].y;

            glPointSize(1.8f + blink * 0.8f);

            glColor3f(brightness, brightness, fminf(brightness * 1.08f, 1.0f));
            glBegin(GL_POINTS);
            glVertex2f(sx, sy);
            glEnd();
        }
    }
}

void drawClouds()
{
    float lightIntensity = (0.7f + 0.3f * sin(timeOfDay * 2.0f * 3.14159f));
    float cloudRise = getBackgroundDrop() * 0.18f;

    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        float x = clouds[i].x - cameraX * 0.3f;
        float y = clouds[i].y + cloudRise;
        float s = clouds[i].size * 0.75f;

        if (x < -260 || x > WIDTH + 260) continue;

        if (timeOfDay < 0.3f)
        {
            glColor4f(0.88f * lightIntensity, 0.84f * lightIntensity, 0.90f * lightIntensity, 0.95f);
        }
        else if (timeOfDay > 0.7f)
        {
            glColor4f(0.90f * lightIntensity, 0.86f * lightIntensity, 0.92f * lightIntensity, 0.95f);
        }
        else
        {
            glColor4f(0.98f * lightIntensity, 0.98f * lightIntensity, 1.00f * lightIntensity, 0.98f);
        }

        drawCircle(x - s * 1.10f, y + s * 0.02f, s * 0.28f, 24);
        drawCircle(x - s * 0.75f, y + s * 0.20f, s * 0.34f, 24);
        drawCircle(x - s * 0.30f, y + s * 0.28f, s * 0.42f, 24);
        drawCircle(x + s * 0.16f, y + s * 0.25f, s * 0.40f, 24);
        drawCircle(x + s * 0.58f, y + s * 0.14f, s * 0.33f, 24);
        drawCircle(x + s * 0.98f, y + s * 0.02f, s * 0.24f, 24);

        drawCircle(x - s * 0.88f, y - s * 0.06f, s * 0.24f, 22);
        drawCircle(x - s * 0.48f, y - s * 0.08f, s * 0.28f, 22);
        drawCircle(x - s * 0.06f, y - s * 0.08f, s * 0.26f, 22);
        drawCircle(x + s * 0.34f, y - s * 0.07f, s * 0.24f, 22);
        drawCircle(x + s * 0.72f, y - s * 0.03f, s * 0.20f, 22);

        if (i % 2 == 0)
        {
            drawCircle(x + s * 1.30f, y - s * 0.02f, s * 0.10f, 18);
            drawCircle(x + s * 1.52f, y - s * 0.04f, s * 0.07f, 18);
        }
    }

    /* extra background clouds */
    for (int i = 0; i < MAX_CLOUDS / 2; i++)
    {
        float x = fmodf(clouds[i].x * 0.8f - cameraX * 0.18f + i * 130.0f, WIDTH + 300.0f) - 150.0f;
        float y = clouds[i].y + 65.0f + (i % 3) * 18.0f + cloudRise * 1.15f;
        float s = clouds[i].size * 0.42f;

        if (timeOfDay < 0.3f) glColor4f(0.78f, 0.78f, 0.86f, 0.45f);
        else if (timeOfDay > 0.7f) glColor4f(0.76f, 0.78f, 0.86f, 0.45f);
        else glColor4f(1.0f, 1.0f, 1.0f, 0.38f);

        drawCircle(x - s * 0.6f, y, s * 0.24f, 18);
        drawCircle(x - s * 0.2f, y + s * 0.12f, s * 0.30f, 18);
        drawCircle(x + s * 0.25f, y + s * 0.10f, s * 0.28f, 18);
        drawCircle(x + s * 0.62f, y, s * 0.20f, 18);
    }
}

float getBackgroundDrop()
{
    /*
     * Return how much the background should shift DOWN to stay aligned
     * with the rising foreground mountain.
     * We measure the mountain's actual height at the current camera
     * position relative to its starting height, then apply a parallax
     * factor so the background moves more slowly than the foreground.
     */
    float groundAtStart  = getMountainY(0.0f);
    float groundAtCamera = getMountainY(cameraX + WIDTH * 0.30f);
    float rise = groundAtCamera - groundAtStart;

    if (rise < 0.0f) rise = 0.0f;

    /* Background moves at ~55% of the foreground rise (parallax) */
    return rise * 0.55f;
}

void drawVillage()
{
    float light = getLightIntensity();
    float bgDrop = getBackgroundDrop() * 0.78f;

    float housePositions[] = {260, 760, 1260, 1820, 2460};
    int houseCount = 5;

    for (int i = 0; i < houseCount; i++)
    {
        float worldX = housePositions[i];
        float x = worldX - cameraX * 0.18f;

        if (x < -220 || x > WIDTH + 220) continue;

        float y = 176.0f - bgDrop;
        float scale = 0.92f + (i % 2) * 0.05f;

        float w = 96.0f * scale;
        float h = 46.0f * scale;
        float roofH = 26.0f * scale;
        float sideInset = 10.0f * scale;

        glColor3f(0.30f * light, 0.32f * light, 0.24f * light);
        drawRect(x - 6.0f * scale, y - 8.0f * scale, w + 12.0f * scale, 8.0f * scale);

        glColor3f(0.66f * light, 0.70f * light, 0.48f * light);
        drawRect(x, y, w, h);

        glColor3f(0.52f * light, 0.56f * light, 0.38f * light);
        drawRect(x, y, w, h * 0.10f);

        glColor3f(0.62f * light, 0.38f * light, 0.24f * light);
        glBegin(GL_POLYGON);
        glVertex2f(x - sideInset,      y + h);
        glVertex2f(x + w + sideInset,  y + h);
        glVertex2f(x + w - 8.0f*scale, y + h + roofH);
        glVertex2f(x + 8.0f*scale,     y + h + roofH);
        glEnd();

        glColor3f(0.46f * light, 0.24f * light, 0.14f * light);
        drawRect(x - sideInset, y + h - 3.0f * scale, w + 2.0f * sideInset, 4.0f * scale);

        glColor3f(0.80f * light, 0.62f * light, 0.48f * light);
        for (int r = 0; r < 6; r++)
        {
            float yy = y + h + 2.0f * scale + r * 4.0f * scale;
            glBegin(GL_LINES);
            glVertex2f(x + 6.0f * scale + r * 0.5f, yy);
            glVertex2f(x + w - 6.0f * scale - r * 0.5f, yy);
            glEnd();
        }

        glColor3f(0.55f * light, 0.34f * light, 0.20f * light);
        for (int c = 0; c < 9; c++)
        {
            float xx = x + 10.0f * scale + c * (w - 20.0f * scale) / 8.0f;
            glBegin(GL_LINES);
            glVertex2f(xx, y + h + 1.0f * scale);
            glVertex2f(xx - 8.0f * scale, y + h + roofH - 1.0f * scale);
            glEnd();
        }

        float doorW = 20.0f * scale;
        float doorH = 30.0f * scale;
        float doorX = x + w * 0.5f - doorW * 0.5f;

        glColor3f(0.30f * light, 0.18f * light, 0.10f * light);
        drawRect(doorX - 2.0f * scale, y, doorW + 4.0f * scale, doorH + 2.0f * scale);

        glColor3f(0.58f * light, 0.38f * light, 0.22f * light);
        drawRect(doorX, y, doorW, doorH);

        glColor3f(0.38f * light, 0.22f * light, 0.12f * light);
        glBegin(GL_LINES);
        glVertex2f(doorX + doorW * 0.5f, y);
        glVertex2f(doorX + doorW * 0.5f, y + doorH);
        glVertex2f(doorX, y + doorH * 0.5f);
        glVertex2f(doorX + doorW, y + doorH * 0.5f);
        glEnd();

        glColor3f(0.95f, 0.82f, 0.28f);
        drawCircle(doorX + doorW * 0.75f, y + doorH * 0.42f, 1.4f * scale, 10);

        glColor3f(0.44f * light, 0.46f * light, 0.36f * light);
        drawRect(doorX - 8.0f * scale, y - 4.0f * scale, doorW + 16.0f * scale, 4.0f * scale);

        float winW = 14.0f * scale;
        float winH = 16.0f * scale;
        float leftWinX = x + 16.0f * scale;
        float rightWinX = x + w - 16.0f * scale - winW;
        float winY = y + 14.0f * scale;

        glColor3f(0.28f * light, 0.18f * light, 0.10f * light);
        drawRect(leftWinX - 2.0f * scale,  winY - 2.0f * scale,  winW + 4.0f * scale, winH + 4.0f * scale);
        drawRect(rightWinX - 2.0f * scale, winY - 2.0f * scale,  winW + 4.0f * scale, winH + 4.0f * scale);

        if (timeOfDay < 0.30f || timeOfDay > 0.70f)
            glColor3f(1.0f, 0.84f, 0.38f);
        else
            glColor3f(0.18f * light, 0.32f * light, 0.30f * light);

        drawRect(leftWinX,  winY, winW, winH);
        drawRect(rightWinX, winY, winW, winH);

        glColor3f(0.90f, 0.78f, 0.26f);
        glBegin(GL_LINES);
        glVertex2f(leftWinX + winW * 0.5f, winY);
        glVertex2f(leftWinX + winW * 0.5f, winY + winH);
        glVertex2f(leftWinX, winY + winH * 0.5f);
        glVertex2f(leftWinX + winW, winY + winH * 0.5f);

        glVertex2f(rightWinX + winW * 0.5f, winY);
        glVertex2f(rightWinX + winW * 0.5f, winY + winH);
        glVertex2f(rightWinX, winY + winH * 0.5f);
        glVertex2f(rightWinX + winW, winY + winH * 0.5f);
        glEnd();

        glColor3f(0.40f * light, 0.24f * light, 0.14f * light);
        drawRect(x + 6.0f * scale,     y, 2.5f * scale, h);
        drawRect(x + w - 8.5f * scale, y, 2.5f * scale, h);

        drawGrassClump(x + 4.0f * scale,     y, 1.0f * scale, light);
        drawGrassClump(x + w - 4.0f * scale, y, 1.0f * scale, light);
    }
}
void drawBirds()
{
    if (timeOfDay < 0.3f || timeOfDay > 0.7f || isRaining) return;

    {
        float lightIntensity = getLightIntensity();
        float t = glutGet(GLUT_ELAPSED_TIME) * 0.002f;

        for (int i = 0; i < MAX_BIRDS; i++)
        {
            float depth = 0.75f + 0.25f * sin(i * 1.7f + t * 0.7f);
            float scale = 0.7f + depth * 0.9f;

            float screenX = birds[i].x - cameraX * (0.10f + depth * 0.10f);
            float screenY = birds[i].y + sin(t * 1.4f + i * 0.8f) * 4.0f;

            if (screenX > -120 && screenX < WIDTH + 120)
            {
                float flap = sin(birds[i].wingPhase) * (4.5f * scale);

                float bodyW = 20.0f * scale;
                float bodyH = 11.0f * scale;
                float headR = 5.5f * scale;
                float wingW = 13.0f * scale;
                float wingH = 8.0f * scale;
                float tailL = 9.0f * scale;
                float legL = 6.0f * scale;

                glColor4f(0.0f, 0.0f, 0.0f, 0.10f * depth);
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(screenX + 2.0f, screenY - 2.0f);
                for (int a = 0; a <= 360; a += 20)
                {
                    float ang = a * 3.14159f / 180.0f;
                    glVertex2f(screenX + 2.0f + cos(ang) * bodyW * 0.55f,
                               screenY - 2.0f + sin(ang) * bodyH * 0.55f);
                }
                glEnd();

                glColor3f(0.10f * lightIntensity, 0.28f * lightIntensity, 0.55f * lightIntensity);
                glBegin(GL_TRIANGLES);
                glVertex2f(screenX - bodyW * 0.65f, screenY + 1.0f);
                glVertex2f(screenX - bodyW * 0.65f - tailL, screenY + 5.0f * scale);
                glVertex2f(screenX - bodyW * 0.45f, screenY - 3.0f * scale);
                glEnd();

                glColor3f(0.12f * lightIntensity, 0.42f * lightIntensity, 0.85f * lightIntensity);
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(screenX, screenY);
                for (int a = 0; a <= 360; a += 15)
                {
                    float ang = a * 3.14159f / 180.0f;
                    glVertex2f(screenX + cos(ang) * bodyW * 0.55f,
                               screenY + sin(ang) * bodyH * 0.55f);
                }
                glEnd();

                glColor3f(0.92f, 0.82f, 0.62f);
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(screenX + bodyW * 0.10f, screenY - bodyH * 0.05f);
                for (int a = 210; a <= 360; a += 15)
                {
                    float ang = a * 3.14159f / 180.0f;
                    glVertex2f(screenX + bodyW * 0.10f + cos(ang) * bodyW * 0.33f,
                               screenY - bodyH * 0.05f + sin(ang) * bodyH * 0.38f);
                }
                glEnd();

                glColor3f(0.08f * lightIntensity, 0.32f * lightIntensity, 0.70f * lightIntensity);
                glBegin(GL_TRIANGLES);
                glVertex2f(screenX - 1.0f * scale, screenY + 1.0f * scale);
                glVertex2f(screenX - wingW * 0.45f, screenY + flap + wingH);
                glVertex2f(screenX + wingW * 0.15f, screenY + 1.0f * scale);
                glEnd();

                glColor3f(0.05f * lightIntensity, 0.25f * lightIntensity, 0.62f * lightIntensity);
                glBegin(GL_TRIANGLES);
                glVertex2f(screenX + 1.0f * scale, screenY);
                glVertex2f(screenX - wingW * 0.15f, screenY + flap + wingH * 0.75f);
                glVertex2f(screenX + wingW * 0.60f, screenY - 1.0f * scale);
                glEnd();

                glColor3f(0.34f * lightIntensity, 0.78f * lightIntensity, 0.98f * lightIntensity);
                drawCircle(screenX + bodyW * 0.48f, screenY + bodyH * 0.28f, headR, 18);

                glColor3f(0.95f, 0.95f, 0.98f);
                drawCircle(screenX + bodyW * 0.58f, screenY + bodyH * 0.18f, headR * 0.65f, 14);

                glColor3f(0.08f, 0.08f, 0.10f);
                drawCircle(screenX + bodyW * 0.66f, screenY + bodyH * 0.34f, headR * 0.14f, 10);

                glColor3f(0.95f, 0.52f, 0.12f);
                glBegin(GL_TRIANGLES);
                glVertex2f(screenX + bodyW * 0.90f, screenY + bodyH * 0.18f);
                glVertex2f(screenX + bodyW * 1.22f, screenY + bodyH * 0.24f);
                glVertex2f(screenX + bodyW * 0.92f, screenY + bodyH * 0.06f);
                glEnd();

                glColor3f(0.92f, 0.60f, 0.22f);
                glLineWidth(1.2f);
                glBegin(GL_LINES);
                glVertex2f(screenX - bodyW * 0.05f, screenY - bodyH * 0.45f);
                glVertex2f(screenX - bodyW * 0.10f, screenY - bodyH * 0.45f - legL);

                glVertex2f(screenX + bodyW * 0.15f, screenY - bodyH * 0.42f);
                glVertex2f(screenX + bodyW * 0.10f, screenY - bodyH * 0.42f - legL);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(screenX - bodyW * 0.10f, screenY - bodyH * 0.45f - legL);
                glVertex2f(screenX - bodyW * 0.18f, screenY - bodyH * 0.45f - legL - 1.5f * scale);

                glVertex2f(screenX - bodyW * 0.10f, screenY - bodyH * 0.45f - legL);
                glVertex2f(screenX - bodyW * 0.04f, screenY - bodyH * 0.45f - legL - 1.0f * scale);

                glVertex2f(screenX + bodyW * 0.10f, screenY - bodyH * 0.42f - legL);
                glVertex2f(screenX + bodyW * 0.02f, screenY - bodyH * 0.42f - legL - 1.5f * scale);

                glVertex2f(screenX + bodyW * 0.10f, screenY - bodyH * 0.42f - legL);
                glVertex2f(screenX + bodyW * 0.16f, screenY - bodyH * 0.42f - legL - 1.0f * scale);
                glEnd();
            }
        }
    }
}

void drawRain()
{
    if (!isRaining) return;

    {
        float lightIntensity = (0.7f + 0.3f * sin(timeOfDay * 2 * 3.14159f));
        glColor4f(0.6f * lightIntensity, 0.7f * lightIntensity, 1.0f * lightIntensity, 0.4f);
        glLineWidth(1.5f);

        glBegin(GL_LINES);
        for (int i = 0; i < MAX_RAIN; i++)
        {
            float x = (float)(rand() % WIDTH);
            float y = (float)(rand() % HEIGHT);
            glVertex2f(x, y);
            glVertex2f(x - wind * 2, y - 15);
        }
        glEnd();
    }
}

void drawLightning()
{
    if (!thunderActive || !isRaining || lightningSegments <= 0) return;

    glLineWidth(8);
    glColor4f(0.7f, 0.85f, 1.0f, 0.18f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= lightningSegments; i++)
    {
        glVertex2f(lightningX[i], lightningY[i]);
    }
    glEnd();

    glLineWidth(4);
    glColor3f(0.95f, 0.98f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= lightningSegments; i++)
    {
        glVertex2f(lightningX[i], lightningY[i]);
    }
    glEnd();

    glLineWidth(2);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= lightningSegments; i++)
    {
        glVertex2f(lightningX[i], lightningY[i]);
    }
    glEnd();
}

void drawThunderFlash()
{
    if (!thunderActive) return;

    {
        float alpha = 0.10f + ((float)thunderTimer / (float)THUNDER_FLASH_TIME) * 0.25f;

        glColor4f(0.85f, 0.9f, 1.0f, alpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WIDTH, 0);
        glVertex2f(WIDTH, HEIGHT);
        glVertex2f(0, HEIGHT);
        glEnd();
    }
}

/* ===== DISTANT LANDSCAPE ===== */
void drawRiver()
{
    float light = getLightIntensity();
    float bgDrop = getBackgroundDrop() * 0.95f;

    glColor3f(0.08f * light, 0.24f * light, 0.60f * light);

    glBegin(GL_POLYGON);

    glVertex2f(0,    145 - bgDrop);
    glVertex2f(200,  142 - bgDrop);
    glVertex2f(400,  144 - bgDrop);
    glVertex2f(600,  150 - bgDrop);
    glVertex2f(800,  155 - bgDrop);
    glVertex2f(1200, 160 - bgDrop);
    glVertex2f(1600, 165 - bgDrop);
    glVertex2f(2000, 170 - bgDrop);
    glVertex2f(2500, 175 - bgDrop);

    glVertex2f(2500, 110 - bgDrop);
    glVertex2f(2000, 108 - bgDrop);
    glVertex2f(1600, 106 - bgDrop);
    glVertex2f(1200, 105 - bgDrop);
    glVertex2f(800,  106 - bgDrop);
    glVertex2f(600,  108 - bgDrop);
    glVertex2f(400,  110 - bgDrop);
    glVertex2f(200,  113 - bgDrop);
    glVertex2f(0,    116 - bgDrop);

    glEnd();
}
void drawFarMountains()
{
    float light = getLightIntensity();
    float bgDrop = getBackgroundDrop() * 0.55f;

    glBegin(GL_TRIANGLES);

    glColor3f(0.56f * light, 0.46f * light, 0.36f * light);
    glVertex2f(-50, 180 - bgDrop);
    glVertex2f(120, 320 - bgDrop);
    glVertex2f(290, 180 - bgDrop);

    glColor3f(0.50f * light, 0.41f * light, 0.33f * light);
    glVertex2f(200, 180 - bgDrop);
    glVertex2f(320, 280 - bgDrop);
    glVertex2f(450, 180 - bgDrop);

    glColor3f(0.46f * light, 0.39f * light, 0.31f * light);
    glVertex2f(420, 180 - bgDrop);
    glVertex2f(650, 470 - bgDrop);
    glVertex2f(900, 180 - bgDrop);

    glColor3f(0.40f * light, 0.34f * light, 0.28f * light);
    glVertex2f(720, 180 - bgDrop);
    glVertex2f(840, 300 - bgDrop);
    glVertex2f(980, 180 - bgDrop);

    glEnd();
}
void drawGrassLand() {
    float light = getLightIntensity();

    /* Refill the green land on this side of the road */
    glColor3f(0.50f * light, 0.78f * light, 0.32f * light);

    glBegin(GL_POLYGON);
    glVertex2f(0, 118);
    glVertex2f(900, 118);
    glVertex2f(900, 0);
    glVertex2f(0, 0);
    glEnd();
}
void drawBackgroundTrees()
{
    float light = getLightIntensity();
    float bgDrop = getBackgroundDrop() * 0.72f;

    float treeX[] = {60, 120, 780, 840};
    int count = 4;

    for (int i = 0; i < count; i++)
    {
        float x = treeX[i] - cameraX * 0.10f;

        if (x < -80 || x > WIDTH + 80) continue;

        float y = 185.0f - bgDrop;

        glColor3f(0.40f * light, 0.22f * light, 0.08f * light);
        drawRect(x, y, 6, 26);

        glColor3f(0.16f * light, 0.62f * light, 0.20f * light);
        drawCircle(x + 3,  y + 36, 16, 20);
        drawCircle(x - 8,  y + 28, 12, 20);
        drawCircle(x + 12, y + 28, 12, 20);
    }
}
void drawGrassClump(float x, float y, float scale, float light)
{
    /* base darker grass */
    glColor3f(0.12f * light, 0.45f * light, 0.12f * light);

    glBegin(GL_TRIANGLES);

    /* left blades */
    glVertex2f(x - 6*scale, y);
    glVertex2f(x - 2*scale, y + 10*scale);
    glVertex2f(x - 1*scale, y);

    /* middle blades */
    glVertex2f(x - 2*scale, y);
    glVertex2f(x, y + 14*scale);
    glVertex2f(x + 2*scale, y);

    /* right blades */
    glVertex2f(x + 1*scale, y);
    glVertex2f(x + 5*scale, y + 11*scale);
    glVertex2f(x + 7*scale, y);

    glEnd();

    /* lighter highlight blades */
    glColor3f(0.35f * light, 0.75f * light, 0.25f * light);

    glBegin(GL_LINES);

    glVertex2f(x - 3*scale, y);
    glVertex2f(x - 2*scale, y + 10*scale);

    glVertex2f(x, y);
    glVertex2f(x, y + 12*scale);

    glVertex2f(x + 3*scale, y);
    glVertex2f(x + 4*scale, y + 10*scale);

    glEnd();
}
void drawTrees()
{
    float light = getLightIntensity();
    float bgDrop = getBackgroundDrop() * 0.60f;

    float treePositions[] = {70, 145, 505, 930, 1305, 1760, 2050, 2540};
    float housePositions[] = {250, 710, 1180, 1720, 2360};

    int treeCount = 8;
    int houseCount = 5;

    for (int i = 0; i < treeCount; i++)
    {
        float worldX = treePositions[i];
        float x = worldX - cameraX * 0.16f;
        int tooCloseToHouse = 0;

        if (x < -120 || x > WIDTH + 120) continue;

        for (int h = 0; h < houseCount; h++)
        {
            if (fabs(worldX - housePositions[h]) < 120.0f)
            {
                tooCloseToHouse = 1;
                break;
            }
        }

        if (tooCloseToHouse) continue;

        /* single y declaration only */
        float y = 178.0f - bgDrop;
        float scale = 0.88f + (i % 3) * 0.08f;
        float trunkH = 34.0f * scale;
        float trunkW = 6.0f * scale;

        glColor3f(0.42f * light, 0.22f * light, 0.08f * light);
        drawRect(x - trunkW / 2, y, trunkW, trunkH);

        glColor3f(0.62f * light, 0.36f * light, 0.14f * light);
        drawRect(x - trunkW / 2 + 1.5f * scale, y, 1.5f * scale, trunkH);

        glColor3f(0.08f * light, 0.42f * light, 0.10f * light);
        drawCircle(x - 16.0f * scale, y + trunkH + 18.0f * scale, 11.0f * scale, 18);
        drawCircle(x - 5.0f  * scale, y + trunkH + 26.0f * scale, 12.0f * scale, 18);
        drawCircle(x + 8.0f  * scale, y + trunkH + 24.0f * scale, 12.0f * scale, 18);
        drawCircle(x + 18.0f * scale, y + trunkH + 16.0f * scale, 10.0f * scale, 18);
        drawCircle(x - 2.0f  * scale, y + trunkH + 12.0f * scale, 12.0f * scale, 18);

        glColor3f(0.18f * light, 0.62f * light, 0.16f * light);
        drawCircle(x - 14.0f * scale, y + trunkH + 20.0f * scale, 9.0f * scale, 18);
        drawCircle(x - 4.0f  * scale, y + trunkH + 28.0f * scale, 10.0f * scale, 18);
        drawCircle(x + 8.0f  * scale, y + trunkH + 26.0f * scale, 10.0f * scale, 18);
        drawCircle(x + 16.0f * scale, y + trunkH + 18.0f * scale, 8.5f * scale, 18);
        drawCircle(x,                y + trunkH + 14.0f * scale, 10.0f * scale, 18);

        glColor3f(0.60f * light, 0.84f * light, 0.22f * light);
        drawCircle(x - 12.0f * scale, y + trunkH + 24.0f * scale, 4.8f * scale, 14);
        drawCircle(x - 1.0f  * scale, y + trunkH + 31.0f * scale, 4.5f * scale, 14);
        drawCircle(x + 10.0f * scale, y + trunkH + 27.0f * scale, 4.3f * scale, 14);

        drawGrassClump(x - 6.0f * scale, y, 1.0f * scale, light);
        drawGrassClump(x + 5.0f * scale, y, 0.9f * scale, light);
        drawGrassClump(x, y, 1.1f * scale, light);
    }
}
void drawForegroundGrass()
{
    float light = getLightIntensity();
    int t = glutGet(GLUT_ELAPSED_TIME);

    for (int i = 0; i < WIDTH; i += 14)
    {
        float worldX = cameraX + i * cameraZoom;
        float baseY = getMountainY(worldX) - cameraY;
        float slope = getMountainSlope(worldX);

        float nx = -slope;
        float ny = 1.0f;
        float len = sqrt(nx * nx + ny * ny);

        if (len != 0.0f)
        {
            nx /= len;
            ny /= len;
        }

        {
            int tuftSize = 3 + rand() % 3;
            float spread = 5.0f + rand() % 4;

            for (int j = 0; j < tuftSize; j++)
            {
                float along = ((float)j / (float)(tuftSize - 1) - 0.5f) * spread;
                float gx = i + along;
                float gy = baseY + along * slope * 0.15f;

                float height = 6.0f + rand() % 5;
                float sway = sin(t * 0.003f + i * 0.12f + j) * 1.0f;

                float r, g, b;
                if (timeOfDay < 0.3f)
                {
                    r = 0.24f * light;
                    g = 0.42f * light;
                    b = 0.16f * light;
                }
                else if (timeOfDay > 0.7f)
                {
                    r = 0.16f * light;
                    g = 0.28f * light;
                    b = 0.10f * light;
                }
                else
                {
                    r = 0.22f * light;
                    g = 0.55f * light;
                    b = 0.18f * light;
                }

                glColor3f(r, g, b);

                glBegin(GL_LINES);
                glVertex2f(gx, gy);
                glVertex2f(gx + nx * height + sway, gy + ny * height);
                glEnd();
            }
        }
    }
}

/* ===== MAIN MOUNTAIN ===== */
void drawMountain()
{
    float lightIntensity = (0.7f + 0.3f * sin(timeOfDay * 2 * 3.14159f));

    glBegin(GL_TRIANGLE_STRIP);
    for (float x = 0; x < WIDTH + 20; x += 4)
    {
        float worldX = cameraX + x * cameraZoom;
        float y = getMountainY(worldX) - cameraY;

        if (worldX >= 0 && worldX <= MOUNTAIN_LENGTH)
        {
            if (y > 360)
            {
                glColor3f(0.10f * lightIntensity, 0.45f * lightIntensity, 0.12f * lightIntensity);
            }
            else if (y > 280)
            {
                glColor3f(0.14f * lightIntensity, 0.55f * lightIntensity, 0.16f * lightIntensity);
            }
            else
            {
                glColor3f(0.08f * lightIntensity, 0.38f * lightIntensity, 0.10f * lightIntensity);
            }

            glVertex2f(x, 0);
            glVertex2f(x, y);
        }
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for (float x = 0; x < WIDTH + 20; x += 4)
    {
        float worldX = cameraX + x * cameraZoom;
        float y = getMountainY(worldX) - cameraY;

        if (worldX >= 0 && worldX <= MOUNTAIN_LENGTH)
        {
            glColor4f(0.03f * lightIntensity, 0.18f * lightIntensity, 0.05f * lightIntensity, 0.45f);
            glVertex2f(x, y - 18);

            glColor4f(0.10f * lightIntensity, 0.35f * lightIntensity, 0.10f * lightIntensity, 0.08f);
            glVertex2f(x, y + 4);
        }
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for (float x = 0; x < WIDTH + 20; x += 6)
    {
        float worldX = cameraX + x * cameraZoom;
        float y = getMountainY(worldX) - cameraY;

        if (worldX >= 0 && worldX <= MOUNTAIN_LENGTH)
        {
            glColor3f(0.40f * lightIntensity, 0.28f * lightIntensity, 0.15f * lightIntensity);
            glVertex2f(x, y - 3);

            glColor3f(0.62f * lightIntensity, 0.46f * lightIntensity, 0.24f * lightIntensity);
            glVertex2f(x, y + 5);
        }
    }
    glEnd();

    glLineWidth(1.5f);
    glColor3f(0.78f * lightIntensity, 0.63f * lightIntensity, 0.36f * lightIntensity);
    glBegin(GL_LINE_STRIP);
    for (float x = 0; x < WIDTH + 20; x += 8)
    {
        float worldX = cameraX + x * cameraZoom;
        float y = getMountainY(worldX) - cameraY;

        if (worldX >= 0 && worldX <= MOUNTAIN_LENGTH)
        {
            glVertex2f(x, y + 5);
        }
    }
    glEnd();
}

void drawObstacles()
{
    int time_ms = glutGet(GLUT_ELAPSED_TIME);

    for (int i = 0; i < numObstacles; i++)
    {
        if (!obstacles[i].active) continue;

        {
            float screenX = (obstacles[i].x - cameraX) / cameraZoom;
            if (screenX > -70 && screenX < WIDTH + 70)
            {
                float y = getMountainY(obstacles[i].x) - cameraY;

                if (obstacles[i].type == 0)
                {
                    float size = 13.0f * obstacles[i].size;

                    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
                    drawCircle(screenX + 4, y + size / 2 - 4, size + 2, 14);

                    if (obstacles[i].alreadyHit)
                    {
                        glColor4f(0.55f, 0.10f, 0.10f, 0.45f);
                    }
                    else
                    {
                        glColor3f(0.85f, 0.12f, 0.12f);
                    }
                    drawCircle(screenX, y + size / 2, size, 14);

                    glColor4f(1.0f, 0.45f, 0.45f, 0.35f);
                    drawCircle(screenX - 3, y + size / 2 + 3, size * 0.32f, 10);
                }
                else
                {
                    float gapWidth = 28.0f * obstacles[i].size;
                    float pulse = 0.7f + 0.3f * sin(time_ms * 0.003f + i);

                    glColor3f(0.18f, 0.02f, 0.02f);
                    drawRect(screenX - gapWidth, y - 24, gapWidth * 2, 36);

                    glColor3f(0.9f * pulse, 0.1f, 0.1f);
                    drawRect(screenX - gapWidth, y - 2, gapWidth * 2, 10);

                    if (!obstacles[i].alreadyHit)
                    {
                        glColor3f(1.0f, 0.2f * pulse, 0.2f * pulse);
                        drawRect(screenX - 3, y + 18, 6, 24);
                        drawRect(screenX - 15, y + 36, 30, 12);

                        glColor3f(1.0f, 1.0f, 1.0f);
                        drawText(screenX - 3, y + 39, "!");
                    }
                }
            }
        }
    }
}
void drawMan()
{
    float x = (man.x - cameraX) / cameraZoom;
    float y = man.y - cameraY;
    float w = man.width;
    float h = man.height;

    if (man.invincible && (man.invincibleTimer / 4) % 2 == 0)
    {
        glColor3f(0.9f, 0.9f, 0.2f);
    }
    else
    {
        glColor3f(0.2f, 0.5f, 1.0f);
    }
    drawRect(x, y, w, h);

    glColor3f(1.0f, 0.8f, 0.6f);
    {
        float headR = w * 0.3f;
        drawCircle(x + w / 2, y + h + headR * 0.8f, headR, 20);
    }

    glColor3f(0.15f, 0.25f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(x + w * 0.3f, y);
    glVertex2f(x + w * 0.25f, y - 18);

    glVertex2f(x + w * 0.7f, y);
    glVertex2f(x + w * 0.75f, y - 18);
    glEnd();

    glColor3f(1.0f, 0.8f, 0.6f);

    if (isRaining)
    {
        glBegin(GL_LINES);
        glVertex2f(x, y + h * 0.7f);
        glVertex2f(x - 10, y + h * 0.5f);
        glEnd();
    }
    else
    {
        glBegin(GL_LINES);
        glVertex2f(x, y + h * 0.7f);
        glVertex2f(x - 10, y + h * 0.5f);

        glVertex2f(x + w, y + h * 0.7f);
        glVertex2f(x + w + 10, y + h * 0.5f);
        glEnd();
    }

    if (isRaining)
    {
        float headR = w * 0.3f;
        float headCenterX = x + w / 2;
        float headTopY = y + h + headR * 1.6f;

        float umbrellaCX = headCenterX + 8.0f;
        float umbrellaCY = headTopY + 24.0f;

        float rx = 34.0f;
        float ry = 17.0f;

        float handX = x + w + 4.0f;
        float handY = y + h * 0.62f;

        glColor3f(1.0f, 0.8f, 0.6f);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex2f(x + w, y + h * 0.72f);
        glVertex2f(handX, handY);
        glEnd();

        glColor4f(0.35f, 0.18f, 0.02f, 0.22f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(umbrellaCX + 2.5f, umbrellaCY - 3.0f);
        for (int i = 0; i <= 180; i += 6)
        {
            float a = i * 3.14159f / 180.0f;
            glVertex2f(umbrellaCX + cos(a) * rx + 2.5f,
                       umbrellaCY + sin(a) * ry - 3.0f);
        }
        glEnd();

        glColor3f(0.96f, 0.56f, 0.02f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(umbrellaCX, umbrellaCY);
        for (int i = 0; i <= 180; i += 6)
        {
            float a = i * 3.14159f / 180.0f;
            glVertex2f(umbrellaCX + cos(a) * rx,
                       umbrellaCY + sin(a) * ry);
        }
        glEnd();

        glColor3f(1.0f, 0.82f, 0.28f);
        glBegin(GL_TRIANGLES);
        glVertex2f(umbrellaCX - 20, umbrellaCY);
        glVertex2f(umbrellaCX - 11, umbrellaCY + 14);
        glVertex2f(umbrellaCX - 5, umbrellaCY - 1);

        glVertex2f(umbrellaCX + 3, umbrellaCY);
        glVertex2f(umbrellaCX + 10, umbrellaCY + 15);
        glVertex2f(umbrellaCX + 17, umbrellaCY - 1);
        glEnd();

        glColor3f(1.0f, 0.74f, 0.18f);
        glBegin(GL_TRIANGLES);
        glVertex2f(umbrellaCX - 3, umbrellaCY);
        glVertex2f(umbrellaCX, umbrellaCY + 17);
        glVertex2f(umbrellaCX + 3, umbrellaCY);
        glEnd();

        glColor4f(0.88f, 0.42f, 0.00f, 0.95f);
        glBegin(GL_TRIANGLES);
        glVertex2f(umbrellaCX - 31, umbrellaCY);
        glVertex2f(umbrellaCX - 22, umbrellaCY + 9);
        glVertex2f(umbrellaCX - 18, umbrellaCY - 1);

        glVertex2f(umbrellaCX + 31, umbrellaCY);
        glVertex2f(umbrellaCX + 22, umbrellaCY + 9);
        glVertex2f(umbrellaCX + 18, umbrellaCY - 1);
        glEnd();

        glColor4f(0.82f, 0.45f, 0.04f, 0.75f);
        glLineWidth(1.6f);
        glBegin(GL_LINES);
        glVertex2f(umbrellaCX, umbrellaCY);
        glVertex2f(umbrellaCX - 27, umbrellaCY + 2);

        glVertex2f(umbrellaCX, umbrellaCY);
        glVertex2f(umbrellaCX - 16, umbrellaCY + 11);

        glVertex2f(umbrellaCX, umbrellaCY);
        glVertex2f(umbrellaCX, umbrellaCY + 16);

        glVertex2f(umbrellaCX, umbrellaCY);
        glVertex2f(umbrellaCX + 16, umbrellaCY + 11);

        glVertex2f(umbrellaCX, umbrellaCY);
        glVertex2f(umbrellaCX + 27, umbrellaCY + 2);
        glEnd();

        glColor3f(0.98f, 0.68f, 0.10f);
        drawCircle(umbrellaCX - 24, umbrellaCY - 1.0f, 4.0f, 12);
        drawCircle(umbrellaCX - 12, umbrellaCY - 2.5f, 4.5f, 12);
        drawCircle(umbrellaCX,      umbrellaCY - 3.0f, 5.0f, 12);
        drawCircle(umbrellaCX + 12, umbrellaCY - 2.5f, 4.5f, 12);
        drawCircle(umbrellaCX + 24, umbrellaCY - 1.0f, 4.0f, 12);

        glColor4f(0.96f, 0.56f, 0.02f, 1.0f);
        drawRect(umbrellaCX - 30, umbrellaCY - 1.0f, 60, 5.0f);

        glColor3f(0.90f, 0.72f, 0.18f);
        drawCircle(umbrellaCX, umbrellaCY + 16.5f, 2.5f, 10);

        glColor3f(0.28f, 0.10f, 0.18f);
        glLineWidth(3.5f);
        glBegin(GL_LINES);
        glVertex2f(umbrellaCX, umbrellaCY - 1.5f);
        glVertex2f(umbrellaCX, handY - 16.0f);
        glEnd();

        glColor3f(0.98f, 0.56f, 0.02f);
        glLineWidth(6.0f);
        glBegin(GL_LINES);
        glVertex2f(umbrellaCX, handY - 16.0f);
        glVertex2f(umbrellaCX, handY - 33.0f);
        glEnd();

        glLineWidth(5.0f);
        glBegin(GL_LINE_STRIP);
        glVertex2f(umbrellaCX, handY - 20.0f);
        glVertex2f(umbrellaCX, handY - 26.0f);
        glVertex2f(umbrellaCX + 3.0f, handY - 25.0f);
        glVertex2f(umbrellaCX + 7.0f, handY - 25.0f);
        glVertex2f(umbrellaCX + 9.0f, handY - 27.0f);
        glVertex2f(umbrellaCX + 9.0f, handY - 23.0f);
        glEnd();

        glColor4f(1.0f, 0.82f, 0.35f, 0.45f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(umbrellaCX - 1.2f, handY - 18.0f);
        glVertex2f(umbrellaCX - 1.2f, handY - 32.0f);
        glEnd();

        glColor3f(1.0f, 0.8f, 0.6f);
        drawCircle(handX, handY, 3.2f, 10);
    }
}

void drawRock()
{
    float screenX = (rock.x - cameraX) / cameraZoom;
    float screenY = rock.y - cameraY;

    if (screenX < -100 || screenX > WIDTH + 100) return;

    {
        float lightIntensity = (0.7f + 0.3f * sin(timeOfDay * 2 * 3.14159f));

        glColor3f(0.1f, 0.1f, 0.1f);
        drawCircle(screenX, screenY - 10, rock.radius, 20);

        rock.rotation += rock.vx * 2;
        glColor3f(0.6f * lightIntensity, 0.6f * lightIntensity, 0.6f * lightIntensity);

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(screenX, screenY);
        for (int i = 0; i <= 360; i += 20)
        {
            float angle = (i + rock.rotation) * 3.14159f / 180;
            float xOffset = cos(angle) * rock.radius;
            float yOffset = sin(angle) * rock.radius * 0.8f;
            glVertex2f(screenX + xOffset, screenY + yOffset);
        }
        glEnd();

        glColor3f(0.8f * lightIntensity, 0.8f * lightIntensity, 0.8f * lightIntensity);
        glPointSize(3);
        glBegin(GL_POINTS);
        glVertex2f(screenX - 6, screenY + 6);
        glVertex2f(screenX + 8, screenY - 4);
        glEnd();
    }
}

void drawParticles()
{
    for (int i = 0; i < numParticles; i++)
    {
        if (!particles[i].active) continue;

        {
            float screenX = (particles[i].x - cameraX) / cameraZoom;
            float screenY = particles[i].y - cameraY;

            glColor4f(1.0f, 1.0f, 0.5f, particles[i].life / 40.0f);
            glPointSize(4);
            glBegin(GL_POINTS);
            glVertex2f(screenX, screenY);
            glEnd();
        }
    }
}

void drawCollectibles()
{
    int t = glutGet(GLUT_ELAPSED_TIME);

    for (int i = 0; i < numCollectibles; i++)
    {
        if (!collectibles[i].active) continue;

        {
            float x = collectibles[i].x - cameraX;
            float y = collectibles[i].y - cameraY + sin(t * 0.005f + i) * 4.0f;

            glColor3f(1.0f, 0.84f, 0.0f);
            drawCircle(x, y, 11, 20);

            glColor3f(1.0f, 0.95f, 0.45f);
            drawCircle(x - 2, y + 2, 5, 12);

            glColor3f(0.95f, 0.65f, 0.0f);
            drawCircle(x + 2, y - 2, 3, 10);
        }
    }
}

void drawUI()
{
    char text[120];
    float fillWidth = (man.energy / 50.0f) * 200.0f;

    glColor3f(1.0f, 0.85f, 0.35f);
    sprintf(text, "Score: %d   Lives: %d   Energy: %d/50", man.score, man.lives, man.energy);
    drawText(20, windowHeight - 30, text);

    glColor3f(0.2f, 0.2f, 0.2f);
    drawRect(20, windowHeight - 60, 200, 14);

    glColor3f(0.0f, man.energy / 50.0f, 0.0f);
    drawRect(20, windowHeight - 60, fillWidth, 14);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(230, windowHeight - 58, "Reach the flag to finish");
}

void drawSummitMarker()
{
    float screenX = (SUMMIT_X - cameraX) / cameraZoom;
    float groundY;
    float poleBottomY;
    float poleTopY;
    float flagTipX;
    float pulse;

    if (screenX < -120 || screenX > WIDTH + 120) return;

    groundY = getMountainY(SUMMIT_X) - cameraY;
    poleBottomY = groundY + 6.0f;
    poleTopY = poleBottomY + 85.0f;
    flagTipX = screenX + 34.0f;
    pulse = 0.75f + 0.25f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.008f);

    glColor4f(1.0f, 0.92f, 0.25f, 0.18f * pulse);
    drawCircle(screenX, groundY + 8.0f, 34.0f, 28);

    glColor4f(1.0f, 0.96f, 0.50f, 0.28f * pulse);
    drawCircle(screenX, groundY + 8.0f, 20.0f, 24);

    glColor3f(0.35f, 0.20f, 0.08f);
    drawRect(screenX - 2.0f, poleBottomY, 4.0f, poleTopY - poleBottomY);

    glColor3f(0.92f, 0.12f, 0.12f);
    glBegin(GL_TRIANGLES);
    glVertex2f(screenX + 2.0f, poleTopY - 4.0f);
    glVertex2f(screenX + 2.0f, poleTopY - 28.0f);
    glVertex2f(flagTipX, poleTopY - 11.0f);
    glEnd();

    glColor3f(1.0f, 0.85f, 0.30f);
    drawCircle(screenX, poleTopY + 3.0f, 5.0f, 14);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(screenX - 30.0f, poleTopY + 18.0f, "SUMMIT");
}
