#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <string>
#include <iostream>
#include <gl/GLU.h>
using namespace std;

// ==================== BIEN TOAN CUC ====================
// Camera
float camX = 0.0f, camY = 2.0f, camZ = 8.0f;
float camYaw = 0.0f, camPitch = 0.0f;
float moveSpeed = 0.1f;
float mouseSensitivity = 0.005f;

// Trang thai phim
bool keys[256] = {false};
bool firstMouse = true;
int lastMouseX = 400, lastMouseY = 300;

// Quat tran
float fanAngle = 0.0f;
bool fanOn = true;

// Cua
float doorAngle = 0.0f;
bool doorOpen = false;

// Den
bool lightOn = true;

// ==================== THUAT TOAN BRESENHAM 2D ====================
// Ham ve duong thang bang thuat toan Bresenham (tu lab)
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
// Ham ve duong tron bang thuat toan Midpoint (bai tap mo rong)
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

// ==================== VE HINH 3D CO BAN ====================
void drawCube(float x, float y, float z, float w, float h, float d, 
              float r, float g, float b, float a = 1.0f) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);
    glColor4f(r, g, b, a);

    glutSolidCube(1.0);
    glPopMatrix();
}

void drawWireCube(float x, float y, float z, float w, float h, float d,
                  float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);
    glColor3f(r, g, b);
    glutWireCube(1.0);
    glPopMatrix();
}

// ==================== VE NHA ====================
void drawFloor() {
    // Nen nha - gach vuong
    glColor3f(0.8f, 0.7f, 0.6f);
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, 0.0f, -5.0f);
    glVertex3f(5.0f, 0.0f, -5.0f);
    glVertex3f(5.0f, 0.0f, 5.0f);
    glVertex3f(-5.0f, 0.0f, 5.0f);
    glEnd();

    // Vien gach
    glColor3f(0.6f, 0.5f, 0.4f);
    for (int i = -5; i <= 5; i++) {
        glBegin(GL_LINES);
        glVertex3f(i, 0.01f, -5.0f);
        glVertex3f(i, 0.01f, 5.0f);
        glVertex3f(-5.0f, 0.01f, i);
        glVertex3f(5.0f, 0.01f, i);
        glEnd();
    }
}

void drawWalls() {
    // Tuong sau
    drawCube(0.0f, 2.0f, -4.9f, 10.0f, 4.0f, 0.2f, 0.9f, 0.9f, 0.85f);

    // Tuong trai
    drawCube(-4.9f, 2.0f, 0.0f, 0.2f, 4.0f, 10.0f, 0.9f, 0.9f, 0.85f);

    // Tuong phai (co cua so)
    // Phan tren
    drawCube(4.9f, 3.0f, -2.0f, 0.2f, 2.0f, 6.0f, 0.9f, 0.9f, 0.85f);
    // Phan duoi
    drawCube(4.9f, 0.5f, -2.0f, 0.2f, 1.0f, 6.0f, 0.9f, 0.9f, 0.85f);
    // Phan giua trai
    drawCube(4.9f, 1.75f, -4.5f, 0.2f, 2.5f, 1.0f, 0.9f, 0.9f, 0.85f);
    // Phan giua phai
    drawCube(4.9f, 1.75f, 0.5f, 0.2f, 2.5f, 1.0f, 0.9f, 0.9f, 0.85f);

    // Cua so (kinh)
    glColor4f(0.6f, 0.8f, 1.0f, 0.4f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex3f(4.85f, 1.0f, -1.5f);
    glVertex3f(4.85f, 2.5f, -1.5f);
    glVertex3f(4.85f, 2.5f, 0.0f);
    glVertex3f(4.85f, 1.0f, 0.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Tran nha
    drawCube(0.0f, 4.0f, 0.0f, 10.0f, 0.2f, 10.0f, 1.0f, 1.0f, 0.95f);
}

void drawRoof() {
    // Mai nha hinh chop
    glColor3f(0.6f, 0.3f, 0.2f);
    glBegin(GL_TRIANGLES);
    // Mat truoc
    glVertex3f(-5.5f, 4.0f, 5.5f);
    glVertex3f(5.5f, 4.0f, 5.5f);
    glVertex3f(0.0f, 6.5f, 0.0f);

    // Mat sau
    glVertex3f(-5.5f, 4.0f, -5.5f);
    glVertex3f(5.5f, 4.0f, -5.5f);
    glVertex3f(0.0f, 6.5f, 0.0f);

    // Mat trai
    glVertex3f(-5.5f, 4.0f, -5.5f);
    glVertex3f(-5.5f, 4.0f, 5.5f);
    glVertex3f(0.0f, 6.5f, 0.0f);

    // Mat phai
    glVertex3f(5.5f, 4.0f, -5.5f);
    glVertex3f(5.5f, 4.0f, 5.5f);
    glVertex3f(0.0f, 6.5f, 0.0f);
    glEnd();

    // Vien mai
    glColor3f(0.5f, 0.25f, 0.15f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-5.5f, 4.0f, 5.5f);
    glVertex3f(5.5f, 4.0f, 5.5f);
    glVertex3f(0.0f, 6.5f, 0.0f);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex3f(-5.5f, 4.0f, -5.5f);
    glVertex3f(5.5f, 4.0f, -5.5f);
    glVertex3f(0.0f, 6.5f, 0.0f);
    glEnd();
}

// ==================== VE CUA ====================
void drawDoor() {
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 4.9f);
    glRotatef(doorAngle, 0.0f, 1.0f, 0.0f);

    // Khung cua
    drawCube(0.0f, 0.0f, 0.0f, 1.5f, 3.0f, 0.1f, 0.6f, 0.4f, 0.2f);

    // Tam cua
    drawCube(0.0f, 0.0f, 0.05f, 1.3f, 2.8f, 0.05f, 0.7f, 0.5f, 0.3f);

    // Tay nam
    glColor3f(0.8f, 0.7f, 0.2f);
    glPushMatrix();
    glTranslatef(0.5f, 0.0f, 0.1f);
    glutSolidSphere(0.08, 16, 16);
    glPopMatrix();

    glPopMatrix();

    // Khung cua co dinh
    drawCube(-0.85f, 1.5f, 4.9f, 0.2f, 3.2f, 0.15f, 0.5f, 0.3f, 0.15f);
    drawCube(0.85f, 1.5f, 4.9f, 0.2f, 3.2f, 0.15f, 0.5f, 0.3f, 0.15f);
    drawCube(0.0f, 3.15f, 4.9f, 1.9f, 0.2f, 0.15f, 0.5f, 0.3f, 0.15f);
}

// ==================== VE QUAT TRAN ====================
void drawFan() {
    glPushMatrix();
    glTranslatef(0.0f, 3.8f, 0.0f);

    // Truc quat
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, -0.3f, 0.0f);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cyl = gluNewQuadric();
    gluCylinder(cyl, 0.05, 0.05, 0.3, 16, 16);
    gluDeleteQuadric(cyl);
    glPopMatrix();

    // Dau quat
    glColor3f(0.2f, 0.2f, 0.2f);
    glutSolidSphere(0.15, 16, 16);

    // Canh quat xoay
    glRotatef(fanAngle, 0.0f, 1.0f, 0.0f);

    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glRotatef(i * 120.0f, 0.0f, 1.0f, 0.0f);

        // Canh quat
        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_QUADS);
        glVertex3f(-0.1f, 0.0f, 0.0f);
        glVertex3f(0.1f, 0.0f, 0.0f);
        glVertex3f(0.05f, 0.0f, 0.8f);
        glVertex3f(-0.05f, 0.0f, 0.8f);
        glEnd();

        // Vien canh
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.1f, 0.0f, 0.0f);
        glVertex3f(0.1f, 0.0f, 0.0f);
        glVertex3f(0.05f, 0.0f, 0.8f);
        glVertex3f(-0.05f, 0.0f, 0.8f);
        glEnd();

        glPopMatrix();
    }

    glPopMatrix();
}

// ==================== VE NOI THAT ====================
void drawTable() {
    // Mat ban
    drawCube(-2.0f, 0.8f, -2.0f, 1.5f, 0.1f, 1.0f, 0.6f, 0.4f, 0.2f);

    // Chan ban
    drawCube(-2.5f, 0.4f, -2.3f, 0.1f, 0.8f, 0.1f, 0.5f, 0.3f, 0.15f);
    drawCube(-1.5f, 0.4f, -2.3f, 0.1f, 0.8f, 0.1f, 0.5f, 0.3f, 0.15f);
    drawCube(-2.5f, 0.4f, -1.7f, 0.1f, 0.8f, 0.1f, 0.5f, 0.3f, 0.15f);
    drawCube(-1.5f, 0.4f, -1.7f, 0.1f, 0.8f, 0.1f, 0.5f, 0.3f, 0.15f);
}

void drawChair() {
    // Ghe 1
    // Ngoi
    drawCube(-2.0f, 0.5f, -1.0f, 0.6f, 0.1f, 0.6f, 0.7f, 0.3f, 0.2f);
    // Lung ghe
    drawCube(-2.0f, 0.9f, -1.25f, 0.6f, 0.8f, 0.1f, 0.7f, 0.3f, 0.2f);
    // Chan ghe
    drawCube(-2.25f, 0.25f, -1.25f, 0.1f, 0.5f, 0.1f, 0.5f, 0.2f, 0.1f);
    drawCube(-1.75f, 0.25f, -1.25f, 0.1f, 0.5f, 0.1f, 0.5f, 0.2f, 0.1f);
    drawCube(-2.25f, 0.25f, -0.75f, 0.1f, 0.5f, 0.1f, 0.5f, 0.2f, 0.1f);
    drawCube(-1.75f, 0.25f, -0.75f, 0.1f, 0.5f, 0.1f, 0.5f, 0.2f, 0.1f);
}

void drawBed() {
    // Khung giuong
    drawCube(2.5f, 0.4f, -3.0f, 1.8f, 0.3f, 2.5f, 0.5f, 0.3f, 0.2f);

    // Nem
    drawCube(2.5f, 0.6f, -3.0f, 1.7f, 0.15f, 2.4f, 0.9f, 0.9f, 0.95f);

    // Goi
    drawCube(2.5f, 0.75f, -3.8f, 1.2f, 0.2f, 0.5f, 1.0f, 1.0f, 1.0f);

    // Dau giuong
    drawCube(2.5f, 1.0f, -4.3f, 1.9f, 1.0f, 0.2f, 0.6f, 0.4f, 0.25f);

    // Chan giuong
    drawCube(2.5f, 0.6f, -1.7f, 1.9f, 0.6f, 0.2f, 0.6f, 0.4f, 0.25f);
}

void drawLamp() {
    // Chan den
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(3.5f, 0.1f, 2.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cyl = gluNewQuadric();
    gluCylinder(cyl, 0.15, 0.15, 0.05, 16, 16);
    gluDeleteQuadric(cyl);
    glPopMatrix();

    // Than den
    drawCube(3.5f, 0.6f, 2.0f, 0.05f, 1.0f, 0.05f, 0.3f, 0.3f, 0.3f);

    // Bong den
    glColor3f(1.0f, 1.0f, 0.8f);
    glPushMatrix();
    glTranslatef(3.5f, 1.2f, 2.0f);
    glutSolidSphere(0.15, 16, 16);
    glPopMatrix();

    // Chup den
    glColor3f(0.8f, 0.7f, 0.5f);
    glPushMatrix();
    glTranslatef(3.5f, 1.3f, 2.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cone = gluNewQuadric();
    gluCylinder(cone, 0.25, 0.05, 0.3, 16, 16);
    gluDeleteQuadric(cone);
    glPopMatrix();
}

// ==================== VE NGOAI CANH ====================
void drawGround() {
    // Co
    glColor3f(0.3f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-20.0f, -0.01f, -20.0f);
    glVertex3f(20.0f, -0.01f, -20.0f);
    glVertex3f(20.0f, -0.01f, 20.0f);
    glVertex3f(-20.0f, -0.01f, 20.0f);
    glEnd();

    // Duong di
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
    glVertex3f(-1.0f, 0.0f, 5.0f);
    glVertex3f(1.0f, 0.0f, 5.0f);
    glVertex3f(1.0f, 0.0f, 15.0f);
    glVertex3f(-1.0f, 0.0f, 15.0f);
    glEnd();
}

void drawTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // Than cay
    glColor3f(0.4f, 0.25f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    GLUquadric* cyl = gluNewQuadric();
    gluCylinder(cyl, 0.2, 0.15, 2.0, 16, 16);
    gluDeleteQuadric(cyl);
    glPopMatrix();

    // La cay
    glColor3f(0.1f, 0.5f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.0f);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, 3.0f, 0.0f);
    glutSolidSphere(0.7, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.5f, 3.0f, 0.0f);
    glutSolidSphere(0.7, 16, 16);
    glPopMatrix();

    glPopMatrix();
}

// ==================== HAM KHOI TAO ====================
void init() {
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Anh sang moi truong
    GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    // Anh sang khuech tan
    GLfloat diffuseLight[] = {0.8f, 0.8f, 0.8f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    // Vi tri nguon sang
    GLfloat lightPosition[] = {0.0f, 5.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glEnable(GL_NORMALIZE);
}

// ==================== HAM VE SCENE ====================
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camera
    float radYaw = camYaw * 3.14159f / 180.0f;
    float radPitch = camPitch * 3.14159f / 180.0f;

    float lookX = camX + cos(radPitch) * sin(radYaw);
    float lookY = camY + sin(radPitch);
    float lookZ = camZ - cos(radPitch) * cos(radYaw);

    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0.0f, 1.0f, 0.0f);

    // Cap nhat nguon sang
    if (lightOn) {
        glEnable(GL_LIGHT0);
    } else {
        glDisable(GL_LIGHT0);
    }

    // Ve ngoai canh
    drawGround();
    drawTree(-8.0f, -5.0f);
    drawTree(8.0f, -5.0f);
    drawTree(-8.0f, 5.0f);
    drawTree(8.0f, 5.0f);

    // Ve nha
    drawFloor();
    drawWalls();
    drawRoof();
    drawDoor();
    drawFan();

    // Ve noi that
    drawTable();
    drawChair();
    drawBed();
    drawLamp();

    // Ve huong dan tren man hinh (2D overlay)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Ve khung huong dan
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(10, 580);
    string info = "=== DIEU KHIEN ===";
    for (char c : info) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

    glRasterPos2f(10, 565);
    info = "WASD: Di chuyen | Chuot: Xoay nhin";
    for (char c : info) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

    glRasterPos2f(10, 550);
    info = "Q/E: Len/Xuong | O: Mo/Dong cua | F: Bat/Tat quat";
    for (char c : info) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

    glRasterPos2f(10, 535);
    info = "L: Bat/Tat den | ESC: Thoat";
    for (char c : info) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

    glRasterPos2f(10, 520);
    info = "Toa do: (" + to_string((int)camX) + ", " + to_string((int)camY) + ", " + to_string((int)camZ) + ")";
    for (char c : info) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glutSwapBuffers();
}

// ==================== XU LY SU KIEN ====================
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // ESC
            exit(0);
            break;
        case 'w': case 'W':
            keys['w'] = true;
            break;
        case 's': case 'S':
            keys['s'] = true;
            break;
        case 'a': case 'A':
            keys['a'] = true;
            break;
        case 'd': case 'D':
            keys['d'] = true;
            break;
        case 'q': case 'Q':
            keys['q'] = true;
            break;
        case 'e': case 'E':
            keys['e'] = true;
            break;
        case 'o': case 'O':
            doorOpen = !doorOpen;
            break;
        case 'f': case 'F':
            fanOn = !fanOn;
            break;
        case 'l': case 'L':
            lightOn = !lightOn;
            break;
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
    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
    }

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    camYaw += dx * mouseSensitivity * 50;
    camPitch -= dy * mouseSensitivity * 50;

    // Gioi han pitch
    if (camPitch > 89.0f) camPitch = 89.0f;
    if (camPitch < -89.0f) camPitch = -89.0f;

    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();
}

void mousePassiveMotion(int x, int y) {
    mouseMotion(x, y);
}

void update(int value) {
    // Di chuyen camera
    float radYaw = camYaw * 3.14159f / 180.0f;
    float forwardX = sin(radYaw);
    float forwardZ = -cos(radYaw);
    float rightX = cos(radYaw);
    float rightZ = sin(radYaw);

    if (keys['w']) {
        camX += forwardX * moveSpeed;
        camZ += forwardZ * moveSpeed;
    }
    if (keys['s']) {
        camX -= forwardX * moveSpeed;
        camZ -= forwardZ * moveSpeed;
    }
    if (keys['a']) {
        camX -= rightX * moveSpeed;
        camZ -= rightZ * moveSpeed;
    }
    if (keys['d']) {
        camX += rightX * moveSpeed;
        camZ += rightZ * moveSpeed;
    }
    if (keys['q']) camY += moveSpeed;
    if (keys['e']) camY -= moveSpeed;

    // Xoay quat
    if (fanOn) {
        fanAngle += 5.0f;
        if (fanAngle > 360.0f) fanAngle -= 360.0f;
    }

    // Mo cua
    if (doorOpen && doorAngle < 90.0f) {
        doorAngle += 2.0f;
    } else if (!doorOpen && doorAngle > 0.0f) {
        doorAngle -= 2.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

// ==================== MAIN ====================
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("3D House - Computer Graphics Lab");

    init();

    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(mousePassiveMotion);
    glutTimerFunc(16, update, 0);

    // An con tro chuot
    glutSetCursor(GLUT_CURSOR_NONE);

    cout << "=== 3D HOUSE - COMPUTER GRAPHICS ===" << endl;
    cout << "Controls:" << endl;
    cout << "  WASD: Move around" << endl;
    cout << "  Q/E: Move up/down" << endl;
    cout << "  Mouse: Look around" << endl;
    cout << "  O: Open/Close door" << endl;
    cout << "  F: Turn fan ON/OFF" << endl;
    cout << "  L: Turn light ON/OFF" << endl;
    cout << "  ESC: Exit" << endl;

    glutMainLoop();
    return 0;
}
