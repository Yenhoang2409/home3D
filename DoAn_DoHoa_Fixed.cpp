#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>

using namespace std;

// ==================== BIEN TOAN CUC ====================
float camX = 0.0f, camY = 3.0f, camZ = 12.0f;
float camYaw = 0.0f, camPitch = -10.0f;
float moveSpeed = 0.15f;
float mouseSensitivity = 0.005f;

bool keys[256] = {false};
bool firstMouse = true;
int lastMouseX = 400, lastMouseY = 300;

// Quat tran
float fanAngle = 0.0f;
bool fanOn = true;
float fanSpeed = 3.0f;

// Cua ben trai (nhu thuc te) - CAO BANG TUONG
float doorAngle = 0.0f;
bool doorOpen = false;

// Cua so
float windowAngle = 0.0f;
bool windowOpen = false;

// Cua tang 2
float bedroomDoorAngle = 0.0f;
bool bedroomDoorOpen = false;
float balconyDoorAngle = 0.0f;
bool balconyDoorOpen = false;
float bathroomDoorAngle = 0.0f;
bool bathroomDoorOpen = false;
float toiletDoorAngle = 0.0f;
bool toiletDoorOpen = false;

// Den
bool lightOn = true;
bool spotLightOn = true;

// Tu lanh
bool fridgeOpen = false;
float fridgeDoorAngle = 0.0f;

// TV
bool tvOn = true;
float tvChannel = 0.0f;

// Thoi tiet
bool isRaining = false;
float rainOffset = 0.0f;

// Texture/Color mode
int renderMode = 0; // 0: Solid, 1: Wireframe, 2: Point

// ==================== THUAT TOAN DDA ====================
void ddaLine(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));

    float xIncrement = dx / (float)steps;
    float yIncrement = dy / (float)steps;

    float x = x1, y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2i(round(x), round(y));
        x += xIncrement;
        y += yIncrement;
    }
    glEnd();
}

// ==================== THUAT TOAN BRESENHAM ====================
void bresenhamLine(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
    glEnd();
}

// ==================== THUAT TOAN MIDPOINT CIRCLE ====================
void midpointCircle(int xc, int yc, int r) {
    int x = 0, y = r;
    int p = 1 - r;

    glBegin(GL_POINTS);
    auto plotPoints = [&](int x, int y) {
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);
        glVertex2i(xc + y, yc + x);
        glVertex2i(xc - y, yc + x);
        glVertex2i(xc + y, yc - x);
        glVertex2i(xc - y, yc - x);
    };

    plotPoints(x, y);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plotPoints(x, y);
    }
    glEnd();
}

// ==================== THUAT TOAN MIDPOINT ELLIPSE ====================
void midpointEllipse(int xc, int yc, int rx, int ry) {
    float dx, dy, d1, d2, x, y;
    x = 0; y = ry;

    d1 = (ry * ry) - (rx * rx * ry) + (0.25f * rx * rx);
    dx = 2 * ry * ry * x;
    dy = 2 * rx * rx * y;

    glBegin(GL_POINTS);
    auto plotPoints = [&](int x, int y) {
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);
    };

    while (dx < dy) {
        plotPoints(x, y);
        if (d1 < 0) {
            x++;
            dx += 2 * ry * ry;
            d1 += dx + (ry * ry);
        } else {
            x++; y--;
            dx += 2 * ry * ry;
            dy -= 2 * rx * rx;
            d1 += dx - dy + (ry * ry);
        }
    }

    d2 = ((ry * ry) * ((x + 0.5f) * (x + 0.5f))) + 
         ((rx * rx) * ((y - 1) * (y - 1))) - (rx * rx * ry * ry);

    while (y >= 0) {
        plotPoints(x, y);
        if (d2 > 0) {
            y--;
            dy -= 2 * rx * rx;
            d2 += (rx * rx) - dy;
        } else {
            y--; x++;
            dx += 2 * ry * ry;
            dy -= 2 * rx * rx;
            d2 += dx - dy + (rx * rx);
        }
    }
    glEnd();
}

// ==================== THUAT TOAN SCANLINE FILL ====================
struct Edge {
    int yMax;
    float xMin;
    float slopeInverse;
};

void scanlineFill(vector<pair<int,int>>& vertices) {
    int n = vertices.size();
    if (n < 3) return;

    int yMin = vertices[0].second, yMax = vertices[0].second;
    for (auto& v : vertices) {
        yMin = min(yMin, v.second);
        yMax = max(yMax, v.second);
    }

    vector<vector<Edge>> edgeTable(yMax + 1);

    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int x1 = vertices[i].first, y1 = vertices[i].second;
        int x2 = vertices[j].first, y2 = vertices[j].second;

        if (y1 == y2) continue;
        if (y1 > y2) { swap(x1, x2); swap(y1, y2); }

        Edge e;
        e.yMax = y2;
        e.xMin = x1;
        e.slopeInverse = (float)(x2 - x1) / (y2 - y1);
        edgeTable[y1].push_back(e);
    }

    vector<Edge> activeEdgeList;

    glBegin(GL_POINTS);
    for (int y = yMin; y <= yMax; y++) {
        for (auto& e : edgeTable[y]) {
            activeEdgeList.push_back(e);
        }

        activeEdgeList.erase(
            remove_if(activeEdgeList.begin(), activeEdgeList.end(),
                [y](Edge& e) { return e.yMax <= y; }),
            activeEdgeList.end()
        );

        sort(activeEdgeList.begin(), activeEdgeList.end(),
            [](Edge& a, Edge& b) { return a.xMin < b.xMin; });

        for (size_t i = 0; i + 1 < activeEdgeList.size(); i += 2) {
            int xStart = ceil(activeEdgeList[i].xMin);
            int xEnd = floor(activeEdgeList[i + 1].xMin);
            for (int x = xStart; x <= xEnd; x++) {
                glVertex2i(x, y);
            }
        }

        for (auto& e : activeEdgeList) {
            e.xMin += e.slopeInverse;
        }
    }
    glEnd();
}

// ==================== THUAT TOAN COHEN-SUTHERLAND CLIPPING ====================
const int INSIDE = 0;
const int LEFT = 1;
const int RIGHT = 2;
const int BOTTOM = 4;
const int TOP = 8;

int computeCode(float x, float y, float xMin, float yMin, float xMax, float yMax) {
    int code = INSIDE;
    if (x < xMin) code |= LEFT;
    else if (x > xMax) code |= RIGHT;
    if (y < yMin) code |= BOTTOM;
    else if (y > yMax) code |= TOP;
    return code;
}

bool cohenSutherlandClip(float& x1, float& y1, float& x2, float& y2,
                         float xMin, float yMin, float xMax, float yMax) {
    int code1 = computeCode(x1, y1, xMin, yMin, xMax, yMax);
    int code2 = computeCode(x2, y2, xMin, yMin, xMax, yMax);
    bool accept = false;

    while (true) {
        if ((code1 == 0) && (code2 == 0)) {
            accept = true;
            break;
        } else if (code1 & code2) {
            break;
        } else {
            float x, y;
            int codeOut = code1 ? code1 : code2;

            if (codeOut & TOP) {
                x = x1 + (x2 - x1) * (yMax - y1) / (y2 - y1);
                y = yMax;
            } else if (codeOut & BOTTOM) {
                x = x1 + (x2 - x1) * (yMin - y1) / (y2 - y1);
                y = yMin;
            } else if (codeOut & RIGHT) {
                y = y1 + (y2 - y1) * (xMax - x1) / (x2 - x1);
                x = xMax;
            } else {
                y = y1 + (y2 - y1) * (xMin - x1) / (x2 - x1);
                x = xMin;
            }

            if (codeOut == code1) {
                x1 = x; y1 = y;
                code1 = computeCode(x1, y1, xMin, yMin, xMax, yMax);
            } else {
                x2 = x; y2 = y;
                code2 = computeCode(x2, y2, xMin, yMin, xMax, yMax);
            }
        }
    }
    return accept;
}

// ==================== DEPTH BUFFER (Z-BUFFER) ====================
float zBuffer[800][600];

void initZBuffer() {
    for (int i = 0; i < 800; i++)
        for (int j = 0; j < 600; j++)
            zBuffer[i][j] = 1.0f;
}

// ==================== VE HINH 3D NANG CAO ====================
void setMaterial(float r, float g, float b, float shininess = 32.0f) {
    GLfloat ambient[] = {r * 0.3f, g * 0.3f, b * 0.3f, 1.0f};
    GLfloat diffuse[] = {r, g, b, 1.0f};
    GLfloat specular[] = {0.8f, 0.8f, 0.8f, 1.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void drawCube(float x, float y, float z, float w, float h, float d, 
              float r, float g, float b, float a = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);
    setMaterial(r, g, b);

    if (renderMode == 0) glutSolidCube(1.0);
    else if (renderMode == 1) glutWireCube(1.0);
    else {
        glPointSize(3.0f);
        glutSolidCube(1.0);
    }
    glPopMatrix();
}

void drawGlassCube(float x, float y, float z, float w, float h, float d,
                   float r, float g, float b, float a = 0.45f) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    setMaterial(r, g, b, 80.0f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);
    glutSolidCube(1.0);
    glPopMatrix();
    glDisable(GL_BLEND);
}

void drawCylinder(float x, float y, float z, float radius, float height,
                  float r, float g, float b, int slices = 24) {
    glColor3f(r, g, b);
    setMaterial(r, g, b, 40.0f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cyl = gluNewQuadric();
    gluCylinder(cyl, radius, radius, height, slices, 12);
    gluDisk(cyl, 0.0, radius, slices, 1);
    glTranslatef(0.0f, 0.0f, height);
    gluDisk(cyl, 0.0, radius, slices, 1);
    gluDeleteQuadric(cyl);
    glPopMatrix();
}

void drawDoorLeaf(float hingeX, float hingeY, float hingeZ, float width, float height,
                  float angle, bool glass, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(hingeX, hingeY, hingeZ);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    glTranslatef(width / 2.0f, height / 2.0f, 0.0f);
    if (glass) {
        drawGlassCube(0.0f, 0.0f, 0.0f, width, height, 0.08f, 0.55f, 0.78f, 0.95f, 0.42f);
        drawCube(0.0f, 0.0f, 0.045f, width, 0.08f, 0.04f, r, g, b);
        drawCube(0.0f, height * 0.25f, 0.045f, width, 0.06f, 0.04f, r, g, b);
        drawCube(0.0f, -height * 0.25f, 0.045f, width, 0.06f, 0.04f, r, g, b);
        drawCube(-width * 0.35f, 0.0f, 0.045f, 0.06f, height, 0.04f, r, g, b);
        drawCube(width * 0.35f, 0.0f, 0.045f, 0.06f, height, 0.04f, r, g, b);
    } else {
        drawCube(0.0f, 0.0f, 0.0f, width, height, 0.10f, r, g, b);
        drawCube(0.0f, height * 0.24f, 0.06f, width * 0.72f, height * 0.22f, 0.04f, r * 0.75f, g * 0.75f, b * 0.75f);
        drawCube(0.0f, -height * 0.18f, 0.06f, width * 0.72f, height * 0.34f, 0.04f, r * 0.75f, g * 0.75f, b * 0.75f);
    }
    glColor3f(0.92f, 0.78f, 0.35f);
    glPushMatrix();
    glTranslatef(width * 0.33f, 0.0f, 0.09f);
    glutSolidSphere(0.07f, 12, 12);
    glPopMatrix();
    glPopMatrix();
}

void drawDoorFrame(float x, float y, float z, float width, float height,
                   float r = 0.36f, float g = 0.22f, float b = 0.13f) {
    drawCube(x, y + height, z, width + 0.18f, 0.12f, 0.18f, r, g, b);
    drawCube(x - width / 2.0f, y + height / 2.0f, z, 0.12f, height, 0.18f, r, g, b);
    drawCube(x + width / 2.0f, y + height / 2.0f, z, 0.12f, height, 0.18f, r, g, b);
}

void drawPitchedRoof() {
    glColor3f(0.34f, 0.12f, 0.08f);
    setMaterial(0.34f, 0.12f, 0.08f, 28.0f);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.6f, 0.8f);
    glVertex3f(-8.8f, 10.0f, 8.7f);
    glVertex3f(8.8f, 10.0f, 8.7f);
    glVertex3f(0.0f, 12.1f, 8.7f);

    glNormal3f(0.0f, 0.6f, -0.8f);
    glVertex3f(8.8f, 10.0f, -8.7f);
    glVertex3f(-8.8f, 10.0f, -8.7f);
    glVertex3f(0.0f, 12.1f, -8.7f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.8f, 0.5f);
    glVertex3f(-8.8f, 10.0f, 8.7f);
    glVertex3f(0.0f, 12.1f, 8.7f);
    glVertex3f(0.0f, 12.1f, -8.7f);
    glVertex3f(-8.8f, 10.0f, -8.7f);

    glNormal3f(0.0f, 0.8f, -0.5f);
    glVertex3f(8.8f, 10.0f, 8.7f);
    glVertex3f(8.8f, 10.0f, -8.7f);
    glVertex3f(0.0f, 12.1f, -8.7f);
    glVertex3f(0.0f, 12.1f, 8.7f);
    glEnd();

    for (int i = -7; i <= 7; i += 2) {
        drawCube((float)i, 10.12f, 8.95f, 0.12f, 0.18f, 0.25f, 0.18f, 0.08f, 0.06f);
        drawCube((float)i, 10.12f, -8.95f, 0.12f, 0.18f, 0.25f, 0.18f, 0.08f, 0.06f);
    }
}

// ==================== NEN GACH VA SAN GO ====================
void drawFloor() {
    for (int i = -8; i < 8; i++) {
        for (int j = -8; j < 8; j++) {
            bool livingArea = j > -1;
            bool lightTile = ((i + j) % 2 == 0);
            float r = livingArea ? (lightTile ? 0.70f : 0.58f) : (lightTile ? 0.86f : 0.78f);
            float g = livingArea ? (lightTile ? 0.48f : 0.36f) : (lightTile ? 0.82f : 0.74f);
            float b = livingArea ? (lightTile ? 0.30f : 0.22f) : (lightTile ? 0.72f : 0.66f);

            drawCube(i * 1.0f + 0.5f, 0.02f, j * 1.0f + 0.5f, 
                     1.0f, 0.04f, 1.0f, r, g, b);
        }
    }

    for (int i = -8; i < 8; i++) {
        for (int j = -8; j < 8; j++) {
            bool bedroom = i < 2;
            bool lightTile = ((i + j) % 2 == 0);
            float r = bedroom ? (lightTile ? 0.66f : 0.52f) : (lightTile ? 0.88f : 0.80f);
            float g = bedroom ? (lightTile ? 0.43f : 0.33f) : (lightTile ? 0.86f : 0.78f);
            float b = bedroom ? (lightTile ? 0.25f : 0.18f) : (lightTile ? 0.80f : 0.72f);
            drawCube(i * 1.0f + 0.5f, 5.42f, j * 1.0f + 0.5f,
                     1.0f, 0.04f, 1.0f, r, g, b);
        }
    }

    glColor3f(0.36f, 0.28f, 0.20f);
    glLineWidth(1.5f);
    for (int i = -8; i <= 8; i++) {
        glBegin(GL_LINES);
        glVertex3f(i, 0.05f, -8.0f);
        glVertex3f(i, 0.05f, 8.0f);
        glVertex3f(-8.0f, 0.05f, i);
        glVertex3f(8.0f, 0.05f, i);
        glEnd();
    }

    glColor3f(0.42f, 0.30f, 0.18f);
    for (int i = -8; i <= 8; i++) {
        glBegin(GL_LINES);
        glVertex3f(i, 5.47f, -8.0f);
        glVertex3f(i, 5.47f, 8.0f);
        glVertex3f(-8.0f, 5.47f, i);
        glVertex3f(8.0f, 5.47f, i);
        glEnd();
    }
}

// ==================== TUONG, VACH PHONG VA MAT TIEN ====================
void drawWalls() {
    float wallR = 0.94f, wallG = 0.91f, wallB = 0.84f;
    drawCube(0.0f, 2.5f, -7.55f, 16.2f, 5.0f, 0.25f, wallR, wallG, wallB);
    drawCube(-7.55f, 2.5f, 0.0f, 0.25f, 5.0f, 15.2f, wallR, wallG, wallB);
    drawCube(7.55f, 2.5f, 0.0f, 0.25f, 5.0f, 15.2f, wallR, wallG, wallB);

    drawCube(-3.7f, 2.5f, 7.55f, 7.4f, 5.0f, 0.25f, wallR, wallG, wallB);
    drawCube(5.0f, 2.5f, 7.55f, 6.0f, 5.0f, 0.25f, wallR, wallG, wallB);
    drawCube(0.6f, 4.4f, 7.55f, 2.0f, 1.2f, 0.25f, wallR, wallG, wallB);

    drawCube(0.0f, 5.2f, 0.0f, 16.2f, 0.22f, 15.2f, 0.88f, 0.82f, 0.72f);

    drawCube(0.0f, 7.7f, -7.55f, 16.2f, 4.6f, 0.25f, wallR, wallG, wallB);
    drawCube(-7.55f, 7.7f, 0.0f, 0.25f, 4.6f, 15.2f, wallR, wallG, wallB);
    drawCube(7.55f, 7.7f, 0.0f, 0.25f, 4.6f, 15.2f, wallR, wallG, wallB);
    drawCube(0.0f, 7.7f, 7.55f, 16.2f, 4.6f, 0.25f, wallR, wallG, wallB);

    drawCube(2.0f, 7.65f, 0.0f, 0.18f, 4.2f, 15.0f, 0.90f, 0.88f, 0.82f);
    drawCube(4.8f, 7.65f, -1.5f, 5.5f, 4.2f, 0.18f, 0.90f, 0.88f, 0.82f);
    drawCube(4.8f, 7.65f, 2.7f, 5.5f, 4.2f, 0.18f, 0.90f, 0.88f, 0.82f);
    drawCube(5.9f, 7.65f, 0.6f, 0.16f, 4.2f, 4.1f, 0.88f, 0.86f, 0.80f);

    drawCube(0.0f, 10.05f, 0.0f, 16.2f, 0.18f, 15.2f, 0.95f, 0.92f, 0.86f);

    drawCube(0.0f, 5.1f, -7.7f, 16.6f, 0.22f, 0.35f, 0.46f, 0.30f, 0.18f);
    drawCube(0.0f, 0.05f, -7.7f, 16.6f, 0.12f, 0.35f, 0.46f, 0.30f, 0.18f);
    drawCube(0.0f, 10.0f, 7.8f, 16.6f, 0.18f, 0.35f, 0.46f, 0.30f, 0.18f);

    drawPitchedRoof();

    glColor3f(1.0f, 0.9f, 0.7f);
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            glPushMatrix();
            glTranslatef(i * 4.0f, 4.8f, j * 4.0f);
            glutSolidSphere(0.15f, 16, 16);
            glPopMatrix();
        }
    }

    drawGlassCube(-4.5f, 8.0f, 7.72f, 2.6f, 2.0f, 0.05f, 0.55f, 0.78f, 0.95f, 0.42f);
    drawGlassCube(4.4f, 8.0f, 7.72f, 2.2f, 2.0f, 0.05f, 0.55f, 0.78f, 0.95f, 0.42f);
    drawCube(-4.5f, 8.0f, 7.78f, 2.8f, 2.15f, 0.08f, 0.35f, 0.22f, 0.13f);
    drawCube(4.4f, 8.0f, 7.78f, 2.4f, 2.15f, 0.08f, 0.35f, 0.22f, 0.13f);
}

// ==================== CUA CAO BANG TUONG, TRONG SUOT KHI MO ====================
void drawDoor() {
    // Cua ben trai tuong - CAO BANG TUONG (tu nen den tran)
    float doorX = 0.6f;
    float doorZ = 7.4f;
    float doorHeight = 5.0f;  // CAO BANG TUONG
    float doorWidth = 2.0f;

    // Khung cua co dinh
    drawCube(doorX, doorHeight / 2.0f, doorZ, doorWidth + 0.2f, doorHeight + 0.2f, 0.15f, 0.5f, 0.35f, 0.2f);

    // Canh cua (xoay quanh truc ben trai)
    glPushMatrix();
    glTranslatef(doorX - doorWidth / 2.0f + 0.1f, doorHeight / 2.0f, doorZ + 0.05f);
    glRotatef(doorAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(doorWidth / 2.0f - 0.1f, 0.0f, 0.0f);

    // Tam cua - TRONG SUOT khi mo
    if (doorOpen) {
        // Khi mo: cua trong suot nhin thay ben trong
        glColor4f(0.85f, 0.75f, 0.55f, 0.35f); // Trong suot
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        // Khi dong: cua go dac
        glColor4f(0.75f, 0.55f, 0.35f, 1.0f);
    }

    glPushMatrix();
    glScalef(doorWidth - 0.1f, doorHeight - 0.1f, 0.08f);
    glutSolidCube(1.0);
    glPopMatrix();

    if (doorOpen) glDisable(GL_BLEND);

    // O kinh cua - LUON TRONG SUOT
    glColor4f(0.7f, 0.85f, 0.95f, 0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.05f);
    glScalef(1.3f, 2.5f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();
    glDisable(GL_BLEND);

    // Tay nam cua
    glColor3f(0.9f, 0.8f, 0.3f);
    glPushMatrix();
    glTranslatef(0.6f, 0.0f, 0.1f);
    glutSolidSphere(0.08f, 16, 16);
    glPopMatrix();

    glPopMatrix();
}

// ==================== CUA SO ====================
void drawWindow() {
    float winX = 7.4f;
    float winY = 3.0f;
    float winZ = 0.0f;

    // Khung cua so
    drawCube(winX, winY, winZ, 0.15f, 2.0f, 2.5f, 0.6f, 0.4f, 0.25f);

    // Canh cua so mo ra ngoai
    glPushMatrix();
    glTranslatef(winX + 0.1f, winY, winZ + 1.0f);
    glRotatef(windowAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -1.0f);

    // Kinh cua so
    glColor4f(0.5f, 0.7f, 0.9f, 0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(0.05f, 1.8f, 2.0f);
    glutSolidCube(1.0);
    glPopMatrix();
    glDisable(GL_BLEND);

    glPopMatrix();

    // Thanh chan cua so
    drawCube(winX + 0.1f, winY, winZ, 0.05f, 1.8f, 0.05f, 0.7f, 0.5f, 0.3f);
    drawCube(winX + 0.1f, winY, winZ, 0.05f, 0.05f, 2.0f, 0.7f, 0.5f, 0.3f);
}

// ==================== QUAT TRAN CAO CAP ====================
void drawFan() {
    glPushMatrix();
    glTranslatef(0.0f, 4.8f, 0.0f);

    // Den trang tri quat
    glColor3f(1.0f, 0.95f, 0.8f);
    glutSolidSphere(0.2f, 16, 16);

    // Truc quat
    glColor3f(0.2f, 0.2f, 0.25f);
    glPushMatrix();
    glTranslatef(0.0f, -0.2f, 0.0f);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cyl = gluNewQuadric();
    gluCylinder(cyl, 0.06, 0.04, 0.4, 16, 16);
    gluDeleteQuadric(cyl);
    glPopMatrix();

    // Dong co
    glColor3f(0.3f, 0.3f, 0.35f);
    glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glutSolidSphere(0.18f, 16, 16);
    glPopMatrix();

    // Canh quat xoay
    glRotatef(fanAngle, 0.0f, 1.0f, 0.0f);

    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        glRotatef(i * 72.0f, 0.0f, 1.0f, 0.0f);

        // Canh quat trong suot
        glColor4f(0.8f, 0.9f, 0.95f, 0.7f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex3f(-0.08f, 0.0f, 0.1f);
        glVertex3f(0.08f, 0.0f, 0.1f);
        glVertex3f(0.04f, 0.0f, 1.0f);
        glVertex3f(-0.04f, 0.0f, 1.0f);
        glEnd();
        glDisable(GL_BLEND);

        // Vien canh
        glColor3f(0.6f, 0.7f, 0.8f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.08f, 0.0f, 0.1f);
        glVertex3f(0.08f, 0.0f, 0.1f);
        glVertex3f(0.04f, 0.0f, 1.0f);
        glVertex3f(-0.04f, 0.0f, 1.0f);
        glEnd();

        glPopMatrix();
    }

    glPopMatrix();
}

// ==================== NOI THAT SANG TRONG ====================
void drawSofa() {
    // Ghe sofa hien dai
    drawCube(-4.0f, 0.6f, 3.0f, 3.0f, 0.5f, 1.2f, 0.4f, 0.3f, 0.5f);
    drawCube(-4.0f, 1.2f, 2.4f, 3.0f, 0.8f, 0.2f, 0.4f, 0.3f, 0.5f);
    drawCube(-5.2f, 0.4f, 3.0f, 0.2f, 0.4f, 1.2f, 0.35f, 0.25f, 0.45f);
    drawCube(-2.8f, 0.4f, 3.0f, 0.2f, 0.4f, 1.2f, 0.35f, 0.25f, 0.45f);
    drawCube(-4.0f, 0.2f, 3.0f, 2.8f, 0.2f, 1.0f, 0.3f, 0.2f, 0.2f);
}

void drawCoffeeTable() {
    // Ban tra kinh
    glColor4f(0.8f, 0.9f, 1.0f, 0.6f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glTranslatef(-4.0f, 0.6f, 4.5f);
    glScalef(1.5f, 0.05f, 1.0f);
    glutSolidCube(1.0);
    glPopMatrix();
    glDisable(GL_BLEND);

    drawCube(-4.5f, 0.3f, 4.2f, 0.05f, 0.6f, 0.05f, 0.8f, 0.7f, 0.5f);
    drawCube(-3.5f, 0.3f, 4.2f, 0.05f, 0.6f, 0.05f, 0.8f, 0.7f, 0.5f);
    drawCube(-4.5f, 0.3f, 4.8f, 0.05f, 0.6f, 0.05f, 0.8f, 0.7f, 0.5f);
    drawCube(-3.5f, 0.3f, 4.8f, 0.05f, 0.6f, 0.05f, 0.8f, 0.7f, 0.5f);
}

void drawTV() {
    // Ke tivi
    drawCube(0.0f, 0.8f, -6.8f, 4.0f, 1.2f, 0.4f, 0.2f, 0.15f, 0.1f);

    if (tvOn) {
        glColor3f(0.1f, 0.15f, 0.3f);
    } else {
        glColor3f(0.05f, 0.05f, 0.05f);
    }
    glPushMatrix();
    glTranslatef(0.0f, 1.2f, -6.55f);
    glScalef(3.5f, 2.0f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    if (tvOn) {
        glColor3f(0.3f + 0.1f * sin(tvChannel), 0.4f + 0.1f * cos(tvChannel), 0.8f);
        glPushMatrix();
        glTranslatef(0.0f, 1.2f, -6.52f);
        glScalef(3.3f, 1.8f, 0.01f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

void drawFridge() {
    drawCube(5.4f, 1.55f, -5.2f, 1.55f, 3.1f, 1.25f, 0.78f, 0.82f, 0.86f);
    drawCube(5.4f, 3.15f, -4.55f, 1.45f, 0.08f, 0.06f, 0.45f, 0.48f, 0.52f);
    drawCube(5.4f, 1.55f, -4.55f, 1.45f, 0.08f, 0.06f, 0.45f, 0.48f, 0.52f);
    drawCube(4.63f, 1.55f, -4.55f, 0.06f, 3.0f, 0.06f, 0.48f, 0.50f, 0.54f);
    drawCube(6.17f, 1.55f, -4.55f, 0.06f, 3.0f, 0.06f, 0.48f, 0.50f, 0.54f);
    drawCube(5.4f, 2.55f, -4.50f, 1.35f, 0.04f, 0.04f, 0.30f, 0.32f, 0.35f);
    drawCube(5.95f, 2.0f, -4.46f, 0.08f, 1.1f, 0.08f, 0.18f, 0.20f, 0.22f);
    drawCube(5.95f, 3.2f, -4.46f, 0.08f, 0.65f, 0.08f, 0.18f, 0.20f, 0.22f);
    drawCube(4.9f, 2.1f, -4.44f, 0.42f, 0.34f, 0.04f, 0.12f, 0.15f, 0.18f);
    drawCube(5.4f, 0.05f, -5.2f, 1.3f, 0.1f, 1.0f, 0.28f, 0.28f, 0.30f);

    glPushMatrix();
    glTranslatef(5.5f + 0.6f, 1.5f, -5.0f + 0.6f);
    glRotatef(fridgeDoorAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(-0.6f, 0.0f, -0.6f);

    drawCube(0.0f, 0.0f, 0.0f, 1.18f, 2.85f, 0.1f, 0.82f, 0.86f, 0.90f);
    drawCube(0.0f, 0.65f, 0.08f, 0.95f, 0.06f, 0.05f, 0.70f, 0.74f, 0.78f);
    drawCube(0.0f, -0.45f, 0.08f, 0.95f, 0.06f, 0.05f, 0.70f, 0.74f, 0.78f);

    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(-0.4f, 0.5f, 0.1f);
    glutSolidSphere(0.05f, 8, 8);
    glPopMatrix();

    glPopMatrix();
}

void drawDiningTable() {
    drawCube(3.0f, 0.8f, 2.0f, 2.5f, 0.1f, 1.5f, 0.7f, 0.5f, 0.3f);
    drawCube(2.0f, 0.4f, 1.5f, 0.1f, 0.8f, 0.1f, 0.6f, 0.4f, 0.25f);
    drawCube(4.0f, 0.4f, 1.5f, 0.1f, 0.8f, 0.1f, 0.6f, 0.4f, 0.25f);
    drawCube(2.0f, 0.4f, 2.5f, 0.1f, 0.8f, 0.1f, 0.6f, 0.4f, 0.25f);
    drawCube(4.0f, 0.4f, 2.5f, 0.1f, 0.8f, 0.1f, 0.6f, 0.4f, 0.25f);

    for (int i = 0; i < 4; i++) {
        float x = 2.0f + i * 0.7f;
        drawCube(x, 0.5f, 3.0f, 0.4f, 0.1f, 0.4f, 0.5f, 0.3f, 0.2f);
        drawCube(x, 0.8f, 2.8f, 0.4f, 0.6f, 0.05f, 0.5f, 0.3f, 0.2f);
    }
}

void drawLamp() {
    drawCube(-6.5f, 0.1f, -4.0f, 0.3f, 0.1f, 0.3f, 0.3f, 0.3f, 0.3f);
    drawCube(-6.5f, 0.6f, -4.0f, 0.05f, 1.0f, 0.05f, 0.3f, 0.3f, 0.3f);

    glColor3f(1.0f, 1.0f, 0.8f);
    glPushMatrix();
    glTranslatef(-6.5f, 1.2f, -4.0f);
    glutSolidSphere(0.2f, 16, 16);
    glPopMatrix();

    glColor3f(0.8f, 0.7f, 0.5f);
    glPushMatrix();
    glTranslatef(-6.5f, 1.3f, -4.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cone = gluNewQuadric();
    gluCylinder(cone, 0.25, 0.1, 0.3, 16, 16);
    gluDeleteQuadric(cone);
    glPopMatrix();
}

void drawRugAndDecor() {
    drawCube(-4.0f, 0.08f, 4.4f, 3.4f, 0.035f, 2.2f, 0.58f, 0.16f, 0.16f);
    drawCube(-4.0f, 0.11f, 4.4f, 3.0f, 0.02f, 1.8f, 0.82f, 0.68f, 0.45f);
    drawCube(-5.4f, 1.45f, 2.95f, 0.35f, 0.35f, 0.35f, 0.85f, 0.82f, 0.72f);
    drawCube(-4.1f, 1.45f, 2.95f, 0.35f, 0.35f, 0.35f, 0.92f, 0.88f, 0.76f);
    drawCube(0.0f, 2.6f, -7.22f, 2.6f, 1.3f, 0.08f, 0.14f, 0.12f, 0.10f);
    drawCube(0.0f, 2.6f, -7.16f, 2.3f, 1.0f, 0.04f, 0.70f, 0.48f, 0.28f);
    drawCylinder(-6.6f, 0.05f, 5.6f, 0.32f, 0.35f, 0.55f, 0.30f, 0.18f);
    glColor3f(0.12f, 0.48f, 0.22f);
    glPushMatrix();
    glTranslatef(-6.6f, 0.75f, 5.6f);
    glutSolidSphere(0.45f, 14, 14);
    glPopMatrix();
}

void drawKitchen() {
    drawCube(6.7f, 0.45f, -2.0f, 1.1f, 0.9f, 3.2f, 0.48f, 0.32f, 0.20f);
    drawCube(6.65f, 1.0f, -2.0f, 1.2f, 0.12f, 3.35f, 0.20f, 0.20f, 0.18f);
    drawCube(6.08f, 1.08f, -2.0f, 0.05f, 0.04f, 1.1f, 0.72f, 0.72f, 0.70f);
    drawCylinder(6.08f, 1.12f, -2.0f, 0.18f, 0.05f, 0.65f, 0.68f, 0.70f);
    drawCube(6.8f, 2.55f, -2.0f, 0.85f, 1.0f, 2.8f, 0.55f, 0.36f, 0.22f);
    for (int i = 0; i < 3; i++) {
        drawCube(6.04f, 0.45f, -3.0f + i, 0.04f, 0.08f, 0.45f, 0.88f, 0.80f, 0.62f);
        drawCube(6.35f, 2.55f, -3.0f + i, 0.04f, 0.08f, 0.42f, 0.88f, 0.80f, 0.62f);
    }
}

void drawStairs() {
    drawCube(-6.2f, 5.45f, 0.0f, 2.5f, 0.16f, 4.5f, 0.62f, 0.48f, 0.32f);
    drawCube(-3.0f, 5.48f, -0.25f, 8.8f, 0.12f, 1.65f, 0.68f, 0.55f, 0.38f);
    drawCube(3.8f, 5.49f, -0.25f, 6.5f, 0.12f, 1.65f, 0.68f, 0.55f, 0.38f);

    for (int i = 0; i < 16; i++) {
        float y = 0.18f + i * 0.33f;
        float z = -6.35f + i * 0.40f;
        float treadDepth = 0.46f;
        drawCube(-6.2f, y, z, 2.15f, 0.16f, treadDepth, 0.66f, 0.46f, 0.28f);
        drawCube(-6.2f, y - 0.10f, z - 0.21f, 2.15f, 0.20f, 0.05f, 0.42f, 0.28f, 0.18f);
    }

    drawCube(-7.4f, 2.7f, -3.2f, 0.10f, 4.9f, 6.0f, 0.14f, 0.11f, 0.09f);
    drawCube(-5.0f, 2.7f, -3.2f, 0.10f, 4.9f, 6.0f, 0.14f, 0.11f, 0.09f);
    drawCube(-6.2f, 5.05f, -0.1f, 2.6f, 0.10f, 0.10f, 0.12f, 0.10f, 0.08f);
    for (int i = 0; i < 9; i++) {
        float z = -6.2f + i * 0.72f;
        float y = 0.75f + i * 0.52f;
        drawCylinder(-7.4f, y, z, 0.035f, 0.85f, 0.16f, 0.13f, 0.10f);
        drawCylinder(-5.0f, y, z, 0.035f, 0.85f, 0.16f, 0.13f, 0.10f);
    }
}

void drawUpperHallAndDoors() {
    drawCube(-0.1f, 5.55f, -0.25f, 12.4f, 0.08f, 1.3f, 0.80f, 0.67f, 0.48f);
    drawCube(-0.1f, 5.60f, -0.25f, 11.6f, 0.035f, 0.9f, 0.72f, 0.28f, 0.22f);
    drawCube(-0.1f, 7.75f, 0.50f, 12.0f, 0.10f, 0.12f, 0.34f, 0.22f, 0.14f);
    for (int i = -5; i <= 5; i++) {
        drawCylinder((float)i, 5.7f, 0.50f, 0.035f, 1.7f, 0.20f, 0.16f, 0.12f);
    }

    drawDoorFrame(-0.35f, 5.5f, 0.68f, 1.45f, 2.25f);
    drawDoorLeaf(-1.08f, 5.5f, 0.78f, 1.38f, 2.18f, bedroomDoorAngle, false, 0.58f, 0.36f, 0.22f);

    drawCube(3.2f, 7.65f, -1.62f, 1.45f, 0.12f, 0.16f, 0.36f, 0.22f, 0.13f);
    drawCube(5.2f, 7.50f, 2.58f, 1.30f, 0.12f, 0.16f, 0.36f, 0.22f, 0.13f);
}

void drawBedroom() {
    drawCube(-3.9f, 5.53f, 2.35f, 5.2f, 0.04f, 3.7f, 0.88f, 0.78f, 0.58f);
    drawCube(-3.9f, 5.56f, 2.35f, 4.4f, 0.03f, 2.7f, 0.78f, 0.36f, 0.32f);

    // Giuong doi: khung go, nem sang mau, chan ga goi ro rang
    drawCube(-4.1f, 5.82f, 2.35f, 4.25f, 0.45f, 3.05f, 0.55f, 0.34f, 0.20f);
    drawCylinder(-6.05f, 5.85f, 0.95f, 0.08f, 0.45f, 0.38f, 0.24f, 0.16f);
    drawCylinder(-2.15f, 5.85f, 0.95f, 0.08f, 0.45f, 0.38f, 0.24f, 0.16f);
    drawCylinder(-6.05f, 5.85f, 3.75f, 0.08f, 0.45f, 0.38f, 0.24f, 0.16f);
    drawCylinder(-2.15f, 5.85f, 3.75f, 0.08f, 0.45f, 0.38f, 0.24f, 0.16f);
    drawCube(-4.1f, 6.12f, 2.35f, 3.95f, 0.34f, 2.72f, 0.96f, 0.93f, 0.84f);
    drawCube(-4.1f, 6.34f, 2.82f, 4.05f, 0.18f, 1.75f, 0.74f, 0.86f, 0.96f);
    drawCube(-4.1f, 6.45f, 1.35f, 4.0f, 0.16f, 0.82f, 0.98f, 0.98f, 0.92f);
    drawCube(-5.15f, 6.62f, 1.05f, 0.92f, 0.22f, 0.52f, 0.98f, 0.96f, 0.90f);
    drawCube(-4.05f, 6.62f, 1.05f, 0.92f, 0.22f, 0.52f, 0.98f, 0.96f, 0.90f);
    drawCube(-3.0f, 6.62f, 1.05f, 0.92f, 0.22f, 0.52f, 0.98f, 0.96f, 0.90f);
    drawCube(-4.1f, 6.9f, 3.88f, 4.45f, 1.25f, 0.30f, 0.48f, 0.29f, 0.18f);

    // Tu dau giuong va den ngu
    for (int side = -1; side <= 1; side += 2) {
        float x = side < 0 ? -6.7f : -1.5f;
        drawCube(x, 5.9f, 1.15f, 0.9f, 0.58f, 0.78f, 0.50f, 0.32f, 0.20f);
        drawCube(x, 6.22f, 1.15f, 0.72f, 0.05f, 0.58f, 0.78f, 0.68f, 0.46f);
        drawCube(x, 6.45f, 1.15f, 0.42f, 0.34f, 0.42f, 0.96f, 0.82f, 0.52f);
        drawCylinder(x, 6.22f, 1.15f, 0.04f, 0.35f, 0.72f, 0.62f, 0.42f);
    }

    // Tu quan ao lon co canh, guong dung va tay nam
    drawCube(-6.35f, 7.2f, -4.95f, 2.9f, 3.25f, 0.72f, 0.42f, 0.27f, 0.17f);
    drawCube(-6.85f, 7.2f, -4.55f, 0.05f, 3.0f, 0.05f, 0.22f, 0.15f, 0.10f);
    drawCube(-5.85f, 7.2f, -4.55f, 0.05f, 3.0f, 0.05f, 0.22f, 0.15f, 0.10f);
    drawGlassCube(-6.35f, 7.25f, -4.50f, 0.62f, 2.45f, 0.04f, 0.75f, 0.86f, 0.92f, 0.55f);
    drawCube(-7.25f, 7.35f, -4.47f, 0.06f, 0.50f, 0.05f, 0.90f, 0.78f, 0.42f);
    drawCube(-5.45f, 7.35f, -4.47f, 0.06f, 0.50f, 0.05f, 0.90f, 0.78f, 0.42f);

    // Ban hoc, ghe, may tinh
    drawCube(-0.45f, 6.02f, -5.45f, 2.25f, 0.16f, 0.88f, 0.54f, 0.36f, 0.22f);
    drawCube(-1.35f, 5.75f, -5.7f, 0.12f, 0.70f, 0.12f, 0.34f, 0.24f, 0.18f);
    drawCube(0.45f, 5.75f, -5.7f, 0.12f, 0.70f, 0.12f, 0.34f, 0.24f, 0.18f);
    drawCube(-0.45f, 6.38f, -5.80f, 1.08f, 0.72f, 0.08f, 0.10f, 0.12f, 0.14f);
    drawCube(-0.45f, 6.38f, -5.74f, 0.92f, 0.56f, 0.04f, 0.25f, 0.42f, 0.72f);
    drawCube(-0.45f, 5.83f, -4.75f, 0.78f, 0.28f, 0.75f, 0.36f, 0.28f, 0.24f);
    drawCube(-0.45f, 6.20f, -4.95f, 0.85f, 0.70f, 0.10f, 0.32f, 0.24f, 0.20f);

    // Ban trang diem va guong tron
    drawCube(-0.45f, 6.00f, 5.15f, 2.0f, 0.18f, 0.72f, 0.58f, 0.38f, 0.25f);
    drawCube(-1.2f, 5.75f, 5.3f, 0.10f, 0.62f, 0.10f, 0.36f, 0.25f, 0.18f);
    drawCube(0.3f, 5.75f, 5.3f, 0.10f, 0.62f, 0.10f, 0.36f, 0.25f, 0.18f);
    drawGlassCube(-0.45f, 6.92f, 5.55f, 1.0f, 1.0f, 0.04f, 0.80f, 0.90f, 0.96f, 0.55f);
    drawCylinder(-0.45f, 6.35f, 5.50f, 0.04f, 0.55f, 0.70f, 0.60f, 0.42f);
    drawCube(-0.95f, 6.15f, 5.58f, 0.18f, 0.28f, 0.16f, 0.86f, 0.40f, 0.42f);
    drawCube(-0.65f, 6.15f, 5.58f, 0.14f, 0.35f, 0.12f, 0.35f, 0.45f, 0.78f);

    // Dieu hoa, den phong, tranh va cua so
    drawCube(-6.6f, 8.72f, 6.50f, 1.25f, 0.34f, 0.20f, 0.86f, 0.88f, 0.90f);
    drawCube(-6.6f, 8.52f, 6.58f, 1.0f, 0.05f, 0.07f, 0.56f, 0.58f, 0.60f);
    drawCylinder(-4.1f, 9.25f, 1.4f, 0.10f, 0.38f, 0.75f, 0.64f, 0.42f);
    glColor3f(1.0f, 0.92f, 0.68f);
    glPushMatrix();
    glTranslatef(-4.1f, 8.85f, 1.4f);
    glutSolidSphere(0.35f, 18, 18);
    glPopMatrix();
    drawCube(-6.9f, 7.65f, 6.55f, 0.08f, 1.45f, 1.25f, 0.20f, 0.16f, 0.12f);
    drawGlassCube(-6.86f, 7.65f, 6.55f, 0.04f, 1.15f, 0.92f, 0.55f, 0.78f, 0.95f, 0.44f);

    // Cua kinh ra ban cong va ban cong dung duoc
    drawDoorFrame(-4.2f, 5.55f, 7.82f, 2.0f, 2.3f, 0.34f, 0.22f, 0.14f);
    drawDoorLeaf(-5.2f, 5.55f, 7.95f, 1.0f, 2.22f, balconyDoorAngle, true, 0.36f, 0.22f, 0.13f);
    drawDoorLeaf(-3.2f, 5.55f, 7.95f, -1.0f, 2.22f, -balconyDoorAngle, true, 0.36f, 0.22f, 0.13f);
    drawCube(-4.2f, 5.58f, 9.2f, 4.9f, 0.16f, 1.65f, 0.56f, 0.50f, 0.42f);
    drawCube(-4.2f, 6.15f, 10.05f, 5.1f, 1.1f, 0.12f, 0.28f, 0.22f, 0.18f);
    drawCube(-6.7f, 6.15f, 9.2f, 0.12f, 1.1f, 1.7f, 0.28f, 0.22f, 0.18f);
    drawCube(-1.7f, 6.15f, 9.2f, 0.12f, 1.1f, 1.7f, 0.28f, 0.22f, 0.18f);
    for (int i = 0; i < 8; i++) {
        drawCylinder(-6.2f + i * 0.58f, 5.65f, 10.02f, 0.025f, 0.9f, 0.22f, 0.18f, 0.14f);
    }
}

void drawBathroom() {
    drawCube(4.85f, 5.54f, -4.65f, 5.0f, 0.04f, 4.8f, 0.78f, 0.86f, 0.88f);
    drawCube(4.85f, 5.58f, -4.65f, 4.3f, 0.035f, 4.1f, 0.88f, 0.92f, 0.90f);

    drawCube(3.15f, 5.62f, -5.4f, 2.1f, 0.18f, 1.25f, 0.95f, 0.95f, 0.92f);
    drawCube(3.15f, 5.85f, -5.4f, 1.75f, 0.26f, 0.95f, 0.78f, 0.88f, 0.94f);
    drawCube(3.15f, 6.02f, -5.4f, 1.55f, 0.04f, 0.78f, 0.70f, 0.84f, 0.92f);
    drawGlassCube(3.95f, 6.55f, -5.4f, 0.05f, 1.45f, 1.05f, 0.72f, 0.90f, 0.96f, 0.35f);
    drawCylinder(2.2f, 6.15f, -4.85f, 0.05f, 1.55f, 0.72f, 0.74f, 0.76f);
    drawCylinder(2.2f, 7.65f, -4.85f, 0.25f, 0.06f, 0.72f, 0.74f, 0.76f);
    for (int i = 0; i < 5; i++) {
        drawCylinder(2.05f + i * 0.08f, 7.48f, -4.85f, 0.012f, 0.25f, 0.45f, 0.65f, 0.90f, 8);
    }

    drawCylinder(5.55f, 5.62f, -5.45f, 0.45f, 0.18f, 0.96f, 0.96f, 0.94f);
    drawCylinder(5.55f, 5.8f, -5.45f, 0.34f, 0.10f, 0.74f, 0.86f, 0.92f);
    drawCylinder(5.55f, 5.9f, -5.45f, 0.04f, 0.24f, 0.70f, 0.72f, 0.74f);
    drawGlassCube(5.55f, 7.05f, -7.35f, 1.1f, 1.1f, 0.04f, 0.78f, 0.90f, 0.96f, 0.55f);
    drawCube(5.55f, 6.05f, -7.30f, 0.62f, 0.12f, 0.08f, 0.90f, 0.88f, 0.82f);
    drawCylinder(4.45f, 5.58f, -7.0f, 0.08f, 0.55f, 0.96f, 0.96f, 0.90f);
    drawCylinder(4.75f, 5.58f, -7.0f, 0.08f, 0.42f, 0.30f, 0.45f, 0.75f);
    drawCylinder(5.05f, 5.58f, -7.0f, 0.08f, 0.35f, 0.86f, 0.30f, 0.34f);
    drawCube(5.25f, 6.35f, -7.05f, 0.32f, 0.12f, 0.12f, 0.92f, 0.92f, 0.86f);
    drawCube(5.60f, 6.35f, -7.05f, 0.28f, 0.16f, 0.12f, 0.30f, 0.60f, 0.35f);
    drawCylinder(6.65f, 5.65f, -6.65f, 0.14f, 0.70f, 0.60f, 0.50f, 0.42f);
    drawCylinder(6.65f, 6.30f, -6.65f, 0.16f, 0.08f, 0.86f, 0.86f, 0.82f);
    drawDoorFrame(3.2f, 5.5f, -1.62f, 1.25f, 2.1f);
    drawDoorLeaf(2.58f, 5.5f, -1.50f, 1.18f, 2.02f, bathroomDoorAngle, false, 0.70f, 0.62f, 0.50f);
}

void drawToiletRoom() {
    drawCube(4.8f, 5.54f, 1.0f, 2.7f, 0.04f, 2.6f, 0.84f, 0.84f, 0.80f);
    drawCylinder(4.7f, 5.62f, 1.05f, 0.38f, 0.28f, 0.95f, 0.95f, 0.92f);
    drawCube(4.7f, 5.95f, 0.70f, 0.75f, 0.45f, 0.25f, 0.94f, 0.94f, 0.90f);
    drawCylinder(4.7f, 5.9f, 1.05f, 0.30f, 0.08f, 0.70f, 0.85f, 0.92f);
    drawCube(4.7f, 6.55f, 0.42f, 0.9f, 0.75f, 0.18f, 0.95f, 0.95f, 0.92f);
    drawCube(3.35f, 6.35f, 2.45f, 0.28f, 0.28f, 0.18f, 0.86f, 0.86f, 0.80f);
    drawCube(3.36f, 6.30f, 2.72f, 0.06f, 0.06f, 0.55f, 0.80f, 0.80f, 0.76f);
    drawCube(5.6f, 6.75f, 2.55f, 0.75f, 0.08f, 0.12f, 0.76f, 0.76f, 0.72f);
    drawCube(5.6f, 6.45f, 2.58f, 0.56f, 0.46f, 0.05f, 0.88f, 0.88f, 0.84f);
    drawCylinder(3.85f, 6.20f, 1.35f, 0.16f, 0.12f, 0.96f, 0.96f, 0.92f);
    drawCube(3.85f, 6.20f, 1.15f, 0.42f, 0.08f, 0.08f, 0.70f, 0.70f, 0.66f);
    drawCube(5.65f, 7.75f, 0.55f, 0.70f, 0.10f, 0.10f, 0.90f, 0.88f, 0.78f);
    drawCube(5.65f, 7.50f, 0.55f, 0.55f, 0.35f, 0.06f, 0.90f, 0.90f, 0.86f);
    drawDoorFrame(5.2f, 5.5f, 2.58f, 1.10f, 2.0f);
    drawDoorLeaf(4.65f, 5.5f, 2.70f, 1.04f, 1.92f, toiletDoorAngle, false, 0.68f, 0.60f, 0.50f);
}

void drawPorchAndGarden() {
    drawCube(0.0f, 0.12f, 8.85f, 4.6f, 0.22f, 2.0f, 0.64f, 0.60f, 0.54f);
    drawCube(0.0f, 0.35f, 7.95f, 2.7f, 0.25f, 0.45f, 0.54f, 0.50f, 0.44f);
    drawCylinder(-2.0f, 0.22f, 8.15f, 0.13f, 3.5f, 0.78f, 0.72f, 0.62f);
    drawCylinder(2.0f, 0.22f, 8.15f, 0.13f, 3.5f, 0.78f, 0.72f, 0.62f);
    drawCube(0.0f, 3.65f, 8.15f, 4.8f, 0.25f, 1.35f, 0.42f, 0.26f, 0.16f);

    for (int i = 0; i < 7; i++) {
        float z = 10.5f + i * 2.0f;
        drawCube(-0.8f, 0.03f, z, 1.1f, 0.05f, 0.85f, 0.62f, 0.62f, 0.58f);
        drawCube(0.8f, 0.03f, z, 1.1f, 0.05f, 0.85f, 0.62f, 0.62f, 0.58f);
    }

    for (int side = -1; side <= 1; side += 2) {
        for (int i = 0; i < 7; i++) {
            float x = side * (2.8f + i * 0.45f);
            float z = 9.2f + i * 0.55f;
            drawCylinder(x, 0.03f, z, 0.18f, 0.22f, 0.36f, 0.22f, 0.12f, 12);
            glColor3f(0.18f, 0.48f, 0.18f);
            glPushMatrix();
            glTranslatef(x, 0.36f, z);
            glutSolidSphere(0.32f, 10, 10);
            glPopMatrix();
            glColor3f(0.95f, 0.38f + 0.08f * i, 0.42f);
            glPushMatrix();
            glTranslatef(x + 0.12f, 0.55f, z);
            glutSolidSphere(0.08f, 8, 8);
            glPopMatrix();
        }
    }
}

// ==================== NGOAI CANH ====================
void drawGround() {
    glColor3f(0.25f, 0.45f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-30.0f, -0.01f, -30.0f);
    glVertex3f(30.0f, -0.01f, -30.0f);
    glVertex3f(30.0f, -0.01f, 30.0f);
    glVertex3f(-30.0f, -0.01f, 30.0f);
    glEnd();

    glColor3f(0.55f, 0.55f, 0.55f);
    glBegin(GL_QUADS);
    glVertex3f(-1.5f, 0.0f, 7.5f);
    glVertex3f(1.5f, 0.0f, 7.5f);
    glVertex3f(1.5f, 0.0f, 25.0f);
    glVertex3f(-1.5f, 0.0f, 25.0f);
    glEnd();

    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-1.5f, 0.01f, 7.5f);
    glVertex3f(1.5f, 0.01f, 7.5f);
    glVertex3f(1.5f, 0.01f, 25.0f);
    glVertex3f(-1.5f, 0.01f, 25.0f);
    glEnd();
}

void drawTree(float x, float z, float scale = 1.0f) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glScalef(scale, scale, scale);

    glColor3f(0.35f, 0.25f, 0.15f);
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cyl = gluNewQuadric();
    gluCylinder(cyl, 0.25, 0.2, 3.0, 12, 12);
    gluDeleteQuadric(cyl);
    glPopMatrix();

    glColor3f(0.15f, 0.45f, 0.15f);
    glPushMatrix();
    glTranslatef(0.0f, 3.5f, 0.0f);
    glutSolidSphere(1.2f, 12, 12);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.6f, 4.0f, 0.0f);
    glutSolidSphere(0.8f, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.6f, 4.0f, 0.0f);
    glutSolidSphere(0.8f, 10, 10);
    glPopMatrix();

    glPopMatrix();
}

void drawFence() {
    glColor3f(0.6f, 0.5f, 0.35f);
    for (int i = -10; i <= 10; i++) {
        glPushMatrix();
        glTranslatef(i * 1.5f, 0.6f, 10.0f);
        glScalef(0.1f, 1.2f, 0.1f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 10.0f);
    glScalef(30.0f, 0.08f, 0.08f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 10.0f);
    glScalef(30.0f, 0.08f, 0.08f);
    glutSolidCube(1.0);
    glPopMatrix();
}

// ==================== MUA ====================
void drawRain() {
    if (!isRaining) return;

    glColor4f(0.6f, 0.7f, 0.9f, 0.6f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
    for (int i = 0; i < 200; i++) {
        float x = (rand() % 400 - 200) / 10.0f;
        float z = (rand() % 400 - 200) / 10.0f;
        float y = 15.0f + (rand() % 100) / 10.0f;
        float len = 0.5f + (rand() % 10) / 10.0f;

        glVertex3f(x, y, z);
        glVertex3f(x - 0.2f, y - len, z);
    }
    glEnd();
    glDisable(GL_BLEND);
}

// ==================== HUD ====================
void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex2f(0, 600);
    glVertex2f(300, 600);
    glVertex2f(300, 365);
    glVertex2f(0, 365);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(1.0f, 1.0f, 1.0f);
    int y = 580;
    auto drawText = [&](const char* text) {
        glRasterPos2f(10, y);
        for (const char* c = text; *c; c++) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
        y -= 18;
    };

    drawText("=== DO AN DO HOA MAY TINH ===");
    drawText("WASD: Di chuyen");
    drawText("Q/E: Len/Xuong");
    drawText("Chuot: Xoay nhin");
    drawText("O: Mo/Dong cua");
    drawText("K: Mo/Dong cua so");
    drawText("1: Cua phong ngu");
    drawText("2: Cua ban cong");
    drawText("3: Cua nha tam");
    drawText("4: Cua toilet");
    drawText("F: Quat ON/OFF");
    drawText("R: Mua ON/OFF");
    drawText("T: TV ON/OFF");
    drawText("G: Tu lanh ON/OFF");
    drawText("L: Den ON/OFF");
    drawText("M: Doi che do ve");
    drawText("ESC: Thoat");

    y -= 10;
    glColor3f(0.3f, 1.0f, 0.3f);
    drawText(fanOn ? "Quat: ON" : "Quat: OFF");
    drawText(doorOpen ? "Cua: MO" : "Cua: DONG");
    drawText(windowOpen ? "Cua so: MO" : "Cua so: DONG");
    drawText(bedroomDoorOpen ? "Phong ngu: MO" : "Phong ngu: DONG");
    drawText(balconyDoorOpen ? "Ban cong: MO" : "Ban cong: DONG");
    drawText(lightOn ? "Den: ON" : "Den: OFF");
    drawText(isRaining ? "Mua: DANG MUA" : "Mua: KHONG");

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ==================== HAM KHOI TAO ====================
void init() {
    glClearColor(0.6f, 0.75f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat ambientLight[] = {0.4f, 0.4f, 0.4f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    GLfloat diffuseLight[] = {0.9f, 0.9f, 0.85f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    GLfloat lightPosition[] = {5.0f, 8.0f, 5.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    GLfloat spotDir[] = {0.0f, -1.0f, 0.0f};
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0f);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0f);

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    srand(time(NULL));
}

// ==================== HAM VE SCENE ====================
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float radYaw = camYaw * 3.14159f / 180.0f;
    float radPitch = camPitch * 3.14159f / 180.0f;

    float lookX = camX + cos(radPitch) * sin(radYaw);
    float lookY = camY + sin(radPitch);
    float lookZ = camZ - cos(radPitch) * cos(radYaw);

    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0.0f, 1.0f, 0.0f);

    if (lightOn) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
    if (spotLightOn) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);

    drawGround();
    drawFence();
    drawTree(-12.0f, -8.0f, 1.2f);
    drawTree(12.0f, -8.0f, 1.0f);
    drawTree(-10.0f, 8.0f, 0.9f);
    drawTree(10.0f, 8.0f, 1.1f);
    drawTree(-15.0f, 0.0f, 1.3f);
    drawTree(15.0f, 0.0f, 1.0f);
    drawPorchAndGarden();

    drawFloor();
    drawWalls();
    drawDoor();
    drawWindow();
    drawFan();

    drawSofa();
    drawCoffeeTable();
    drawTV();
    drawFridge();
    drawDiningTable();
    drawLamp();
    drawRugAndDecor();
    drawKitchen();
    drawStairs();
    drawUpperHallAndDoors();
    drawBedroom();
    drawBathroom();
    drawToiletRoom();

    drawRain();
    drawHUD();

    glutSwapBuffers();
}

// ==================== XU LY SU KIEN ====================
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: exit(0); break;
        case 'w': case 'W': keys['w'] = true; break;
        case 's': case 'S': keys['s'] = true; break;
        case 'a': case 'A': keys['a'] = true; break;
        case 'd': case 'D': keys['d'] = true; break;
        case 'q': case 'Q': keys['q'] = true; break;
        case 'e': case 'E': keys['e'] = true; break;
        case 'o': case 'O': doorOpen = !doorOpen; break;
        case 'k': case 'K': windowOpen = !windowOpen; break;
        case '1': bedroomDoorOpen = !bedroomDoorOpen; break;
        case '2': balconyDoorOpen = !balconyDoorOpen; break;
        case '3': bathroomDoorOpen = !bathroomDoorOpen; break;
        case '4': toiletDoorOpen = !toiletDoorOpen; break;
        case 'f': case 'F': fanOn = !fanOn; break;
        case 'l': case 'L': lightOn = !lightOn; break;
        case 'r': case 'R': isRaining = !isRaining; break;
        case 't': case 'T': tvOn = !tvOn; break;
        case 'g': case 'G': fridgeOpen = !fridgeOpen; break;
        case 'm': case 'M': renderMode = (renderMode + 1) % 3; break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W': keys['w'] = false; break;
        case 's': case 'S': keys['s'] = false; break;
        case 'a': case 'A': keys['a'] = false; break;
        case 'd': case 'D': keys['d'] = false; break;
        case 'q': case 'Q': keys['q'] = false; break;
        case 'e': case 'E': keys['e'] = false; break;
    }
}

void mouseMotion(int x, int y) {
    if (firstMouse) { lastMouseX = x; lastMouseY = y; firstMouse = false; }
    int dx = x - lastMouseX, dy = y - lastMouseY;
    camYaw += dx * mouseSensitivity * 50;
    camPitch -= dy * mouseSensitivity * 50;
    if (camPitch > 89.0f) camPitch = 89.0f;
    if (camPitch < -89.0f) camPitch = -89.0f;
    lastMouseX = x; lastMouseY = y;
    glutPostRedisplay();
}

void mousePassiveMotion(int x, int y) { mouseMotion(x, y); }

void update(int value) {
    float radYaw = camYaw * 3.14159f / 180.0f;
    float forwardX = sin(radYaw), forwardZ = -cos(radYaw);
    float rightX = cos(radYaw), rightZ = sin(radYaw);

    if (keys['w']) { camX += forwardX * moveSpeed; camZ += forwardZ * moveSpeed; }
    if (keys['s']) { camX -= forwardX * moveSpeed; camZ -= forwardZ * moveSpeed; }
    if (keys['a']) { camX -= rightX * moveSpeed; camZ -= rightZ * moveSpeed; }
    if (keys['d']) { camX += rightX * moveSpeed; camZ += rightZ * moveSpeed; }
    if (keys['q']) camY += moveSpeed;
    if (keys['e']) camY -= moveSpeed;

    if (fanOn) { fanAngle += fanSpeed; if (fanAngle > 360.0f) fanAngle -= 360.0f; }
    if (doorOpen && doorAngle < 110.0f) doorAngle += 2.0f;
    else if (!doorOpen && doorAngle > 0.0f) doorAngle -= 2.0f;
    if (windowOpen && windowAngle < 45.0f) windowAngle += 1.5f;
    else if (!windowOpen && windowAngle > 0.0f) windowAngle -= 1.5f;
    if (bedroomDoorOpen && bedroomDoorAngle < 95.0f) bedroomDoorAngle += 2.0f;
    else if (!bedroomDoorOpen && bedroomDoorAngle > 0.0f) bedroomDoorAngle -= 2.0f;
    if (balconyDoorOpen && balconyDoorAngle < 85.0f) balconyDoorAngle += 2.0f;
    else if (!balconyDoorOpen && balconyDoorAngle > 0.0f) balconyDoorAngle -= 2.0f;
    if (bathroomDoorOpen && bathroomDoorAngle < 90.0f) bathroomDoorAngle += 2.0f;
    else if (!bathroomDoorOpen && bathroomDoorAngle > 0.0f) bathroomDoorAngle -= 2.0f;
    if (toiletDoorOpen && toiletDoorAngle < 90.0f) toiletDoorAngle += 2.0f;
    else if (!toiletDoorOpen && toiletDoorAngle > 0.0f) toiletDoorAngle -= 2.0f;
    if (fridgeOpen && fridgeDoorAngle < 120.0f) fridgeDoorAngle += 2.0f;
    else if (!fridgeOpen && fridgeDoorAngle > 0.0f) fridgeDoorAngle -= 2.0f;

    if (!keys['q'] && !keys['e']) {
        float targetY = camY;
        if (camX > -7.45f && camX < -5.0f && camZ > -6.55f && camZ < -0.1f) {
            float t = (camZ + 6.55f) / 6.45f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            targetY = 2.2f + t * 5.1f;
        } else if (camY > 5.4f && camX > -7.6f && camX < 7.6f && camZ > -7.6f && camZ < 10.2f) {
            targetY = 7.25f;
        } else if (camY < 5.2f && camX > -7.6f && camX < 7.6f && camZ > -7.6f && camZ < 8.8f) {
            targetY = 2.35f;
        }
        camY += (targetY - camY) * 0.12f;
    }

    tvChannel += 0.05f;
    rainOffset += 0.1f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ==================== MAIN ====================
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1200, 800);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("DO AN DO HOA MAY TINH - NHA 3D SANG TRONG");

    init();

    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutTimerFunc(16, update, 0);

    glutSetCursor(GLUT_CURSOR_NONE);

    cout << "=== DO AN DO HOA MAY TINH - NHOM 5 NGUOI ===" << endl;
    cout << "Nha 3D sang trong voi day du thuat toan do hoa" << endl;
    cout << "Phim dieu khien:" << endl;
    cout << "  WASD: Di chuyen | Q/E: Len/Xuong" << endl;
    cout << "  Chuot: Xoay nhin | O: Cua | K: Cua so" << endl;
    cout << "  1: Cua phong ngu | 2: Cua ban cong | 3: Cua nha tam | 4: Cua toilet" << endl;
    cout << "  F: Quat | R: Mua | T: TV | G: Tu lanh" << endl;
    cout << "  L: Den | M: Che do ve | ESC: Thoat" << endl;

    glutMainLoop();
    return 0;
}
