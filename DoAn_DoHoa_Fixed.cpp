#define NOMINMAX // Phải đặt trước windows.h để chặn xung đột hàm max()
#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "InteriorCamera.h"
#include "Kitchen.h"
#include "GroundFloorWC.h"
#include "FirstFloorWC.h"

using namespace std;

extern void drawKitchenInterior();
extern void drawLivingRoomInterior();
extern void LivingRoomKeyboard(unsigned char key, int x, int y);
extern void FirstFloorWCMouse(int button, int state, int x, int y);
extern void FirstFloorWCUpdate();
// ==================== BIẾN TOÀN CỤC & CAMERA ====================
GLuint texWood, texStone, texFloor, texTV;

bool keys[256] = { false };
int lastMouseX = -1, lastMouseY = -1;
float GLOBAL_SCALE = 4.5f;

// ==================== BIẾN ANIMATION ĐÃ ĐƯỢC KHAI BÁO ĐẦY ĐỦ ====================
float fanAngle = 0.0f;
float fanSpeed = 4.0f; // Khai báo tốc độ quạt
bool fanOn = true;

float mainDoorAngle = 0.0f;
bool mainDoorOpen = false;

float windowSlide = 0.0f;
bool windowOpen = false; // Khai báo biến trượt cửa sổ kính

float curtainOffset = 0.0f;
bool curtainOpen = false; // Khai báo biến kéo rèm

bool tvOn = true;
float tvR = 0.1f, tvG = 0.5f, tvB = 0.8f;

float fridgeAngle = 0.0f;
bool fridgeOpen = false; // Khai báo biến mở tủ lạnh

bool isRaining = false;

// Động cơ Multi-Pass
bool isTransparentPass = false;

// ==================== HÀM TẢI TEXTURE ====================
GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);
    return textureID;
}

// ==================== THUẬT TOÁN HỌC THUẬT ====================
void ddaLine(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1; int dy = y2 - y1;
    int steps = std::max(std::abs(dx), std::abs(dy)); // Đã fix lỗi xung đột hàm max
    float xIncrement = dx / (float)steps;
    float yIncrement = dy / (float)steps;
    float x = (float)x1, y = (float)y1;
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2i((int)round(x), (int)round(y));
        x += xIncrement; y += yIncrement;
    }
    glEnd();
}

void bresenhamLine(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x2 - x1); int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1; int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1); if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
    glEnd();
}

// ==================== VẬT LIỆU & VẼ HÌNH CƠ BẢN ====================
void setMaterial(float r, float g, float b, float shininess = 64.0f, bool isEmissive = false) {
    GLfloat ambient[] = { r * 0.6f, g * 0.6f, b * 0.6f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat emission[] = { r * 0.8f, g * 0.8f, b * 0.8f, 1.0f };
    GLfloat no_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    if (isEmissive) glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    else glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, no_emission);
}

void drawCube(float x, float y, float z, float w, float h, float d, float r, float g, float b, bool isLED = false) {
    if (isTransparentPass) return;
    glPushMatrix();
    glTranslatef(x, y, z); glScalef(w, h, d);
    setMaterial(r, g, b, 32.0f, isLED); glColor3f(r, g, b);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawGlassCube(float x, float y, float z, float w, float h, float d, float r, float g, float b, float alpha = 0.3f) {
    if (!isTransparentPass) return;
    glPushMatrix();
    glTranslatef(x, y, z); glScalef(w, h, d);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, alpha); setMaterial(r, g, b, 100.0f);
    glutSolidCube(1.0);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawFrostedGlass(float x, float y, float z, float w, float h, float d, float r, float g, float b, float alpha = 0.85f) {
    if (!isTransparentPass) return;
    glPushMatrix();
    glTranslatef(x, y, z); glScalef(w, h, d);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, alpha); setMaterial(r, g, b, 10.0f);
    glutSolidCube(1.0);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawCylinder(float x, float y, float z, float radius, float height, float r, float g, float b) {
    if (isTransparentPass) return;
    glPushMatrix();
    glTranslatef(x, y, z); glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(r, g, b); setMaterial(r, g, b);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radius, radius, height, 16, 16);
    gluDisk(quad, 0.0, radius, 16, 1);
    glTranslatef(0.0f, 0.0f, height); gluDisk(quad, 0.0, radius, 16, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

// Đã fix lỗi "truncation from double to GLfloat" (Thêm 100% hậu tố 'f' cho các số thập phân)
void drawTexturedCube(float x, float y, float z, float w, float h, float d, GLuint tex) {
    if (isTransparentPass) return;
    glPushMatrix(); glTranslatef(x, y, z); glScalef(w, h, d);
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1.0f, 1.0f, 1.0f);
    float sX = w / 2.0f; float sY = h / 2.0f; float sZ = d / 2.0f;
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); glTexCoord2f(sX, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); glTexCoord2f(sX, sY); glVertex3f(0.5f, 0.5f, 0.5f); glTexCoord2f(0.0f, sY); glVertex3f(-0.5f, 0.5f, 0.5f);
    glNormal3f(0.0f, 0.0f, -1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); glTexCoord2f(sX, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); glTexCoord2f(sX, sY); glVertex3f(-0.5f, 0.5f, -0.5f); glTexCoord2f(0.0f, sY); glVertex3f(0.5f, 0.5f, -0.5f);
    glNormal3f(0.0f, 1.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f); glTexCoord2f(sX, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f); glTexCoord2f(sX, sZ); glVertex3f(0.5f, 0.5f, -0.5f); glTexCoord2f(0.0f, sZ); glVertex3f(-0.5f, 0.5f, -0.5f);
    glNormal3f(0.0f, -1.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); glTexCoord2f(sX, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); glTexCoord2f(sX, sZ); glVertex3f(0.5f, -0.5f, 0.5f); glTexCoord2f(0.0f, sZ); glVertex3f(-0.5f, -0.5f, 0.5f);
    glNormal3f(1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); glTexCoord2f(sZ, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); glTexCoord2f(sZ, sY); glVertex3f(0.5f, 0.5f, -0.5f); glTexCoord2f(0.0f, sY); glVertex3f(0.5f, 0.5f, 0.5f);
    glNormal3f(-1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); glTexCoord2f(sZ, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); glTexCoord2f(sZ, sY); glVertex3f(-0.5f, 0.5f, 0.5f); glTexCoord2f(0.0f, sY); glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd(); glDisable(GL_TEXTURE_2D); glPopMatrix();
}

// ==================== MODULES NỘI THẤT ====================
void drawToilet(float x, float y, float z) {
    drawCube(x, y + 0.4f, z, 0.8f, 0.6f, 1.2f, 0.95f, 0.95f, 0.95f); // Bệ ngồi
    drawCube(x, y + 1.2f, z - 0.4f, 0.8f, 0.8f, 0.4f, 0.95f, 0.95f, 0.95f); // Két nước
    if (!isTransparentPass) { glPushMatrix(); glTranslatef(x, y + 0.7f, z + 0.1f); glScalef(1.0f, 0.1f, 1.2f); glColor3f(0.95f, 0.95f, 0.95f); glutSolidSphere(0.4, 16, 16); glPopMatrix(); }
}

void drawModernBed(float x, float y, float z) {
    drawCube(x, y + 0.28f, z, 2.8f, 0.42f, 3.2f, 0.3f, 0.3f, 0.3f);
    drawCube(x, y + 0.58f, z, 2.6f, 0.22f, 3.0f, 0.85f, 0.85f, 0.85f);
    drawCube(x, y + 0.74f, z + 0.8f, 2.4f, 0.12f, 1.4f, 0.6f, 0.6f, 0.6f);
    drawCube(x, y + 1.2f, z - 1.4f, 2.8f, 1.2f, 0.2f, 0.4f, 0.3f, 0.2f);
    drawCube(x - 2.0f, y + 0.6f, z - 1.2f, 0.8f, 0.3f, 0.8f, 0.4f, 0.3f, 0.2f);
    drawCube(x + 2.0f, y + 0.6f, z - 1.2f, 0.8f, 0.3f, 0.8f, 0.4f, 0.3f, 0.2f);
}

// ==================== TẦNG 1 (TRỆT) - KHÔNG GIAN SINH HOẠT CHUNG ====================
void drawGroundFloor() {
    // 1. Lối vào & Hiên 
    for (int i = 0; i < 3; i++) {
        drawCube(5.0f, 0.1f + i * 0.15f, 10.0f - i * 0.4f, 4.0f, 0.15f, 0.4f, 0.4f, 0.4f, 0.4f);
        drawCube(5.0f, 0.1f + i * 0.15f - 0.05f, 10.2f - i * 0.4f, 3.8f, 0.05f, 0.02f, 1.0f, 0.8f, 0.2f, true);
    }
    drawCube(5.0f, 0.6f, 8.5f, 4.0f, 0.05f, 2.0f, 0.3f, 0.3f, 0.3f);
    drawCube(3.0f, 2.8f, 7.5f, 0.2f, 4.5f, 0.2f, 0.1f, 0.1f, 0.1f);
    drawCube(7.0f, 2.8f, 7.5f, 0.2f, 4.5f, 0.2f, 0.1f, 0.1f, 0.1f);
    glPushMatrix(); glTranslatef(3.1f, 2.8f, 7.5f); glRotatef(mainDoorAngle, 0.0f, 1.0f, 0.0f);
    drawGlassCube(1.9f, 0.0f, 0.0f, 3.8f, 4.3f, 0.05f, 0.7f, 0.8f, 0.9f, 0.4f);
    glPopMatrix();

    // =========================================================
    // 2. Phòng Khách
    // =========================================================
    drawTexturedCube(-5.5f, 0.6f, 1.5f, 11.0f, 0.1f, 11.0f, texWood); // Sàn gỗ

    // Hệ vách kính & Cửa sổ trượt
    drawGlassCube(-5.5f, 2.8f, 7.0f, 11.0f, 4.5f, 0.1f, 0.7f, 0.85f, 0.95f, 0.3f);
    drawGlassCube(-11.0f, 2.8f, -1.0f, 0.1f, 4.5f, 6.0f, 0.7f, 0.85f, 0.95f, 0.3f);
    drawGlassCube(-10.9f, 2.8f, 4.5f - windowSlide, 0.1f, 4.5f, 5.0f, 0.7f, 0.85f, 0.95f, 0.3f);

    // Rèm âm trần
    drawCube(-10.8f, 2.8f, 1.5f + curtainOffset, 0.05f, 4.5f, 11.0f, 0.9f, 0.9f, 0.9f);
    drawCube(-5.5f + curtainOffset, 2.8f, 6.8f, 11.0f, 4.5f, 0.05f, 0.9f, 0.9f, 0.9f);

    // GỌI HÀM VẼ NỘI THẤT PHÒNG KHÁCH MỚI (Sofa, Bàn kính, Kệ, TV)
    glPushMatrix();
    glTranslatef(0.0f, 0.65f, 0.0f);
    drawLivingRoomInterior();
    glPopMatrix();

    // ==========================================================
    // 3. Khu vực Bếp & Đảo Bếp
    // ==========================================================
    drawTexturedCube(5.5f, 0.6f, 1.5f, 11.0f, 0.1f, 11.0f, texWood); // Sàn gỗ khu vực bếp

    // --- XÂY DỰNG HỆ THỐNG VÁCH NGĂN ---
    // 1. Vách ngăn bên phải
    drawCube(10.0f, 2.8f, -3.5f, 0.5f, 4.5f, 6.0f, 0.85f, 0.85f, 0.85f);

    // 2. Bức tường ốp đá ngay sau lưng bếp (Ngăn cách hoàn toàn với Toilet)
    drawTexturedCube(6.0f, 2.8f, -3.25f, 8.0f, 4.5f, 0.2f, texStone);

    // --- GỌI GIAN BẾP ---
    glPushMatrix();
    drawKitchenInterior(); // Dàn bếp sẽ tựa lưng hoàn hảo vào bức tường đá vừa xây
    glPopMatrix();

    // --- ĐẢO BẾP VÀ GHẾ BAR ---
    drawCube(5.0f, 1.2f, 0.0f, 5.0f, 1.2f, 1.5f, 0.2f, 0.2f, 0.2f);
    drawCube(5.0f, 1.85f, 0.0f, 5.2f, 0.1f, 1.8f, 0.95f, 0.95f, 0.95f);
    for (int i = 0; i < 4; i++) {
        drawCylinder(3.0f + i * 1.3f, 0.6f, 1.5f, 0.05f, 1.0f, 0.1f, 0.1f, 0.1f);
        drawCube(3.0f + i * 1.3f, 1.6f, 1.5f, 0.6f, 0.1f, 0.6f, 0.5f, 0.5f, 0.5f);
    }

    // 4. Khu Vực Ăn Uống 
    drawCube(0.0f, 1.4f, 2.5f, 4.0f, 0.1f, 2.0f, 0.6f, 0.4f, 0.2f);
    drawCube(-1.5f, 0.6f, 2.5f, 0.1f, 0.8f, 1.0f, 0.1f, 0.1f, 0.1f);
    drawCube(1.5f, 0.6f, 2.5f, 0.1f, 0.8f, 1.0f, 0.1f, 0.1f, 0.1f);
    for (int i = 0; i < 3; i++) {
        drawCube(-1.0f + i * 1.0f, 1.0f, 1.5f, 0.6f, 0.1f, 0.6f, 0.85f, 0.8f, 0.7f);
        drawCube(-1.0f + i * 1.0f, 1.0f, 3.5f, 0.6f, 0.1f, 0.6f, 0.85f, 0.8f, 0.7f);
    }
    drawCylinder(-0.5f, 5.0f, 2.5f, 0.02f, 2.0f, 0.1f, 0.1f, 0.1f);
    drawCylinder(0.5f, 5.0f, 2.5f, 0.02f, 2.0f, 0.1f, 0.1f, 0.1f);
    drawCube(-0.5f, 2.9f, 2.5f, 0.6f, 0.2f, 0.6f, 1.0f, 0.8f, 0.4f, true);
    drawCube(0.5f, 2.9f, 2.5f, 0.6f, 0.2f, 0.6f, 1.0f, 0.8f, 0.4f, true);

    // 5. Cầu Thang Trung Tâm 
    for (int i = 0; i < 9; i++) {
        drawCube(1.5f - i * 0.4f, 0.6f + i * 0.25f, -3.0f, 0.4f, 0.1f, 2.5f, 0.6f, 0.4f, 0.2f);
        drawCube(1.5f - i * 0.4f, 0.5f + i * 0.25f, -1.7f, 0.4f, 0.05f, 0.05f, 1.0f, 0.8f, 0.2f, true);
    }
    drawCube(-2.0f, 2.85f, -3.0f, 1.5f, 0.1f, 2.5f, 0.6f, 0.4f, 0.2f);
    for (int i = 0; i < 9; i++) {
        drawCube(-2.0f, 3.1f + i * 0.25f, -1.5f + i * 0.4f, 1.5f, 0.1f, 0.4f, 0.6f, 0.4f, 0.2f);
    }
    drawGlassCube(0.0f, 2.0f, -1.7f, 4.0f, 1.5f, 0.05f, 0.7f, 0.9f, 1.0f, 0.3f);
    drawCube(0.0f, 2.8f, -1.7f, 4.0f, 0.1f, 0.1f, 0.2f, 0.2f, 0.2f);

    // 6. Phòng Vệ Sinh Khách
    drawCube(6.0f, 2.8f, -6.5f, 8.0f, 4.5f, 0.2f, 0.75f, 0.75f, 0.75f);
    drawCube(2.0f, 2.8f, -5.0f, 0.2f, 4.5f, 3.0f, 0.75f, 0.75f, 0.75f);

    // SÀN WC MÀU TỐI (Xám đen) giúp chống chói và làm nổi bật thiết bị vệ sinh
    drawCube(6.0f, 0.65f, -5.0f, 8.0f, 0.05f, 3.0f, 0.15f, 0.15f, 0.15f);
    drawCube(3.0f, 1.6f, -3.5f, 1.4f, 2.0f, 0.1f, 0.3f, 0.2f, 0.1f);

    drawCube(4.0f, 1.4f, -6.0f, 1.2f, 0.2f, 0.8f, 0.9f, 0.9f, 0.9f);
    drawCube(4.0f, 2.5f, -6.3f, 1.0f, 1.0f, 0.05f, 0.6f, 0.8f, 0.9f);
    drawCube(4.0f, 2.5f, -6.35f, 1.1f, 1.1f, 0.02f, 1.0f, 0.9f, 0.6f, true);

    // GỌI HÀM VẼ WC: Đã nâng Y = 0.68f để bồn cầu/rửa tay nằm đúng trên mặt sàn
    drawGroundFloorWC(7.0f, 0.68f, -5.5f);
}

// ==================== TẦNG LẦU (KHÔNG GIAN NGHỈ NGƠI) ====================
void drawFirstFloor() {
    drawTexturedCube(0.0f, 5.0f, 0.0f, 22.0f, 0.2f, 18.0f, texWood);
    drawCube(0.0f, 7.5f, -8.5f, 22.0f, 5.0f, 0.5f, 0.85f, 0.85f, 0.85f);
    drawCube(0.0f, 10.2f, 0.0f, 23.0f, 0.4f, 19.0f, 0.15f, 0.15f, 0.15f);

    // 1. Khối Vươn (Cantilever) bọc lam gỗ
    drawTexturedCube(5.5f, 5.0f, 4.5f, 11.0f, 0.2f, 9.0f, texWood);
    for (int i = 0; i < 30; i++) {
        drawCube(0.0f + i * 0.35f, 7.5f, 9.0f, 0.1f, 5.0f, 0.2f, 0.6f, 0.4f, 0.2f);
    }
    drawGlassCube(5.5f, 7.5f, 7.0f, 11.0f, 4.8f, 0.1f, 0.7f, 0.85f, 0.95f, 0.3f);
    drawModernBed(6.0f, 5.0f, 2.0f);
    drawCube(6.0f, 6.5f, -0.5f, 4.0f, 3.0f, 1.0f, 0.2f, 0.2f, 0.2f);
    drawCube(6.0f, 5.4f, 4.0f, 2.0f, 0.3f, 0.6f, 0.4f, 0.4f, 0.4f);

    drawCube(5.5f, 5.1f, 8.0f, 11.0f, 0.05f, 2.0f, 0.4f, 0.3f, 0.2f);
    drawGlassCube(5.5f, 5.6f, 8.8f, 11.0f, 1.2f, 0.05f, 0.7f, 0.9f, 1.0f, 0.3f);
    drawCylinder(4.0f, 5.1f, 7.5f, 0.3f, 0.5f, 0.8f, 0.8f, 0.8f);

    // 2. Khối Bê Tông (Bedroom 1) 
    drawTexturedCube(-5.5f, 7.5f, 4.5f, 11.0f, 5.0f, 9.0f, texStone);
    drawGlassCube(-5.5f, 7.5f, 9.0f, 11.0f, 4.8f, 0.1f, 0.7f, 0.85f, 0.95f, 0.3f);
    drawModernBed(-5.0f, 5.0f, 3.0f);
    drawCube(-10.5f, 6.5f, 2.0f, 0.5f, 3.0f, 4.0f, 0.9f, 0.9f, 0.9f);

    // 3. Bedroom 2 
    drawCube(-5.5f, 7.5f, -4.5f, 11.0f, 5.0f, 9.0f, 0.85f, 0.85f, 0.85f);
    drawGlassCube(-11.0f, 7.0f, -4.5f, 0.1f, 2.0f, 3.0f, 0.7f, 0.85f, 0.95f, 0.3f);
    drawModernBed(-5.0f, 5.0f, -5.0f);
    drawCube(-8.0f, 5.8f, -8.0f, 2.0f, 0.1f, 1.0f, 0.8f, 0.8f, 0.8f);

    // 4. Main Bath 
    drawCube(5.0f, 7.5f, -4.5f, 6.0f, 5.0f, 6.0f, 0.85f, 0.85f, 0.85f);
    drawCube(5.0f, 5.1f, -4.5f, 6.0f, 0.05f, 6.0f, 0.5f, 0.5f, 0.5f);
    drawGlassCube(6.5f, 7.5f, -3.0f, 3.0f, 5.0f, 0.1f, 0.7f, 0.9f, 1.0f, 0.3f);
    drawCylinder(7.0f, 9.5f, -5.0f, 0.02f, 1.0f, 0.8f, 0.8f, 0.8f);
    drawCube(7.0f, 9.5f, -5.0f, 0.4f, 0.05f, 0.4f, 0.8f, 0.8f, 0.8f);
    drawFirstFloorWC(5.0f, 5.15f, -4.5f);
    drawCube(3.0f, 5.8f, -2.5f, 1.5f, 0.1f, 1.0f, 0.2f, 0.2f, 0.2f);
    if (!isTransparentPass) { glPushMatrix(); glTranslatef(3.0f, 6.0f, -2.5f); glScalef(0.6f, 0.3f, 0.6f); glColor3f(0.9f, 0.9f, 0.9f); glutSolidSphere(0.5f, 16, 16); glPopMatrix(); }

    // 5. Hành lang 
    drawGlassCube(0.0f, 5.6f, -1.7f, 4.0f, 1.2f, 0.05f, 0.7f, 0.9f, 1.0f, 0.3f);
}

// ==================== CẤU TRÚC NGOẠI CẢNH ====================
void drawGround() {
    glColor3f(0.2f, 0.4f, 0.15f);
    glBegin(GL_QUADS);
    glVertex3f(-150.0f, -0.01f, -150.0f); glVertex3f(150.0f, -0.01f, -150.0f);
    glVertex3f(150.0f, -0.01f, 150.0f); glVertex3f(-150.0f, -0.01f, 150.0f);
    glEnd();
}

void drawRain() {
    if (!isRaining) return;
    glColor4f(0.7f, 0.8f, 0.9f, 0.8f);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 2000; i++) {
        // Phủ rộng khu vực sân vườn: [-100.0f, 100.0f]
        float x = (rand() % 2000 - 1000) / 10.0f;
        float z = (rand() % 2000 - 1000) / 10.0f;

        // THUẬT TOÁN LỌC MƯA TRONG NHÀ:
        if (x > -52.0f && x < 52.0f && z > -43.0f && z < 43.0f) {
            continue;
        }

        // Mưa rơi từ trên mây (Cao hơn đỉnh mái nhà 46.0f)
        float y = 50.0f + (rand() % 500) / 10.0f; // Y ngẫu nhiên từ 50.0f đến 100.0f
        float len = 2.0f + (rand() % 20) / 10.0f;

        glVertex3f(x, y, z);
        glVertex3f(x - 0.5f, y - len, z); // Trừ đi 0.5f ở trục X để tạo hiệu ứng mưa bay xiên theo gió
    }
    glEnd(); glDisable(GL_BLEND);
}

void drawFullHouse() {
    drawCube(0.0f, -0.1f, 0.0f, 40.0f, 0.1f, 40.0f, 0.2f, 0.4f, 0.2f);

    drawTexturedCube(0.0f, 2.5f, -9.0f, 22.0f, 5.0f, 0.5f, texStone);
    drawTexturedCube(11.0f, 2.5f, 0.0f, 0.5f, 5.0f, 18.0f, texStone);

    drawGroundFloor();
    drawFirstFloor();
}

// ==================== GIAO DIỆN HUD ====================
void drawHUD() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 800, 0, 600); glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);
    glColor4f(0.0f, 0.05f, 0.1f, 0.72f); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS); glVertex2f(0, 600); glVertex2f(285, 600); glVertex2f(285, 472); glVertex2f(0, 472); glEnd(); glDisable(GL_BLEND);
    int y = 575; auto drawText = [&](const char* text) { glRasterPos2f(12, y); for (const char* c = text; *c; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c); y -= 16; };
    glColor3f(1.0f, 0.8f, 0.2f); drawText("CUBISM VILLA BIM DATA - PERFECT");
    glColor3f(1.0f, 1.0f, 1.0f); drawText("WASD move | Mouse look | Q/E up/down");
    drawText("O door | K window | C curtain");
    drawText("F fan | T TV | R rain | G fridge");
    glColor3f(0.5f, 1.0f, 0.5f); drawText(isRaining ? "Weather: Rain" : "Weather: Clear");
    glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

// ==================== VÒNG LẶP & UPDATE ====================
void renderScene() {
    if (isRaining) glClearColor(0.4f, 0.45f, 0.5f, 1.0f);
    else glClearColor(0.6f, 0.8f, 0.95f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 1. ÁP DỤNG CAMERA MỚI (Thay thế toàn bộ cụm tính toán và gluLookAt cũ)
    CameraApply();

    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    GLfloat lightPos[] = { 10.0f, 50.0f, 20.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    drawGround();

    glPushMatrix();
    glScalef(GLOBAL_SCALE, GLOBAL_SCALE, GLOBAL_SCALE);
    isTransparentPass = false;
    drawFullHouse();
    isTransparentPass = true;
    glDepthMask(GL_FALSE);
    drawFullHouse();
    glDepthMask(GL_TRUE);
    glPopMatrix();

    drawRain();
    drawHUD();

    // 2. VẼ GIAO DIỆN CAMERA NỘI THẤT (Chèn ngay trước khi Swap Buffers)
    // Lưu ý: Đổi 1280, 720 thành chiều rộng/chiều cao cửa sổ của bạn nếu cần.
    CameraDrawHUD(1280, 720);

    glutSwapBuffers();
}

void update(int value) {
    // 3. CẬP NHẬT DI CHUYỂN CAMERA (Thay thế toàn bộ cục logic WASD cũ)
    CameraUpdate(GLOBAL_SCALE);

    // Các logic đồ vật trong nhà được giữ nguyên vẹn
    if (fanOn) fanAngle += fanSpeed;
    if (mainDoorOpen && mainDoorAngle < 90.0f) mainDoorAngle += 2.0f;
    if (!mainDoorOpen && mainDoorAngle > 0.0f) mainDoorAngle -= 2.0f;
    if (windowOpen && windowSlide < 5.0f) windowSlide += 0.05f;
    if (!windowOpen && windowSlide > 0.0f) windowSlide -= 0.05f;
    if (curtainOpen && curtainOffset < 5.0f) curtainOffset += 0.05f;
    if (!curtainOpen && curtainOffset > 0.0f) curtainOffset -= 0.05f;
    if (fridgeOpen && fridgeAngle < 90.0f) fridgeAngle += 2.0f;
    if (!fridgeOpen && fridgeAngle > 0.0f) fridgeAngle -= 2.0f;
    if (tvOn) { tvR = (rand() % 10) / 10.0f; tvG = (rand() % 10) / 10.0f; tvB = (rand() % 10) / 10.0f; }

    FirstFloorWCUpdate();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    // 4. CHUYỂN GIAO PHÍM BẤM CHO CAMERA MỚI
    CameraKeyDown(key, x, y);

    LivingRoomKeyboard(key, x, y);

    switch (key) {
        case 27: exit(0); break;
        case 'o': mainDoorOpen = !mainDoorOpen; break;
        case 'k': windowOpen = !windowOpen; break;
        case 'c': curtainOpen = !curtainOpen; break;
        case 'f': fanOn = !fanOn; break;
        case 't': tvOn = !tvOn; break;
        case 'g': fridgeOpen = !fridgeOpen; break;
        case 'r': isRaining = !isRaining; break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    // 5. CHUYỂN GIAO SỰ KIỆN NHẢ PHÍM
    CameraKeyUp(key, x, y);
}

void mouseMotion(int x, int y) {
    // 6. THAY THẾ TOÀN BỘ LOGIC QUAY CHUỘT BẰNG HÀM MỚI
    CameraMouseMotion(x, y);
}

void mousePassive(int x, int y) {
    CameraMouseMotion(x, y);
}

// Hàm Click chuột truyền tới phòng WC
void mouseClick(int button, int state, int x, int y) {
    FirstFloorWCMouse(button, state, x, y);
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    // 1. BẬT CHUẨN HÓA PHÁP TUYẾN (CỰC KỲ QUAN TRỌNG)
    // Giúp OpenGL tự động tính toán lại vector pháp tuyến sau khi bạn dùng lệnh glScalef. 
    glEnable(GL_NORMALIZE);

    // 2. GIẢM ÁNH SÁNG MÔI TRƯỜNG XUỐNG
    // Giảm từ 0.8f xuống 0.3f để tạo ra độ tương phản (có mảng sáng, mảng tối).
    GLfloat ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // Kích hoạt ánh sáng khuếch tán (Diffuse) cho Light0 để chiếu rõ khối 3D
    GLfloat light0_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);

    texWood = loadTexture("wood.jpg");
    texStone = loadTexture("stone.jpg");
    texFloor = loadTexture("floor.jpg");

    // Khởi tạo vị trí Camera ban đầu 
    CameraInit(0.0f, 2.5f * 4.5f, 12.0f * 4.5f);

    // Ảnh nhà bếp
    initKitchenTextures();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(90.0, (float)w / h, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Do An 3D - PERFECT VILLA CUBISM");
    init();
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mousePassive);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(16, update, 0);
    glutMouseFunc(mouseClick);
    glutMainLoop();
    return 0;
}
