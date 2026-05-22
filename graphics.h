#ifndef GRAPHICS_H
#define GRAPHICS_H

// Graphics Algorithms
void drawLineDDA(int x1, int y1, int x2, int y2);
void drawLineBresenham(int x1, int y1, int x2, int y2);
void drawCircleMidpoint(int xc, int yc, int r);
void drawEllipseMidpoint(int xc, int yc, int rx, int ry);
void drawRect(float x, float y, float w, float h);
void drawCircle(float cx, float cy, float r, int segments);
void drawText(float x, float y, char *string);
void drawGrassClump(float x, float y, float scale, float light);

// Drawing functions
void drawSky();
void drawSun();
void drawMoon();
void drawStars();
void drawClouds();
void drawRain();
void drawBirds();
void drawLightning();
void drawThunderFlash();
void drawMountain();
void drawObstacles();
void drawSummitMarker();
void drawMan();
void drawRock();
void drawParticles();
void drawUI();
void drawLightning();
void drawThunderFlash();
void drawCollectibles();
void drawRiver();
void drawTrees();
void drawVillage();
void drawForegroundGrass();
void drawFarMountains();
void drawGrassLand();
void drawBackgroundTrees();
void drawBackgroundTrees();
void drawFarMountains();
float getDistantLandY(float worldX);
void drawSummitMarker();

#endif
