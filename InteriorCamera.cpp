// ================================================================
// InteriorCamera.cpp  -  He thong camera noi that
// Thuan OpenGL 1.x + GLUT + GLU, khong them thu vien ngoai
//
// CACH DUNG:
//   1. Them 2 file nay vao Visual Studio project (cung thu muc .cpp)
//   2. Trong DoAn_DoHoa_Fixed.cpp, them: #include "InteriorCamera.h"
//   3. Xem phan "TICH HOP" o cuoi file nay de biet sua o dau
// ================================================================

#define NOMINMAX
#include "InteriorCamera.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==================== DANH SACH PHONG ====================
// Toa do logic (CHUA nhan GLOBAL_SCALE = 4.5f)
// Dua tren ban do trong drawGroundFloor() & drawFirstFloor()
const Room g_rooms[] = {
    //  ten              minX   maxX   minY  maxY   minZ   maxZ   spX    spY   spZ    yaw
    { "Phong Khach",    -11.0f,  0.0f,  0.0f, 5.5f,  -4.0f,  8.5f,  -5.0f, 1.8f,  3.0f,   0.0f },
    { "Bep & An uong",   0.0f, 11.0f,  0.0f, 5.5f,  -4.0f,  8.5f,   5.5f, 2.2f,  4.0f, -90.0f },
    { "WC Tang Tret",    2.0f, 10.0f,  0.0f, 5.5f,  -8.5f, -3.5f,   6.0f, 1.8f, -5.5f, 180.0f },
    { "Phong Ngu 1",    -11.0f,  0.0f,  5.0f,11.0f,   0.0f,  9.0f,  -5.0f, 7.0f,  4.0f, 180.0f },
    { "Phong Ngu 2",    -11.0f,  0.0f,  5.0f,11.0f,  -9.0f,  0.0f,  -5.0f, 7.0f, -5.0f,  90.0f },
    { "Phong Ngu 3",     0.0f, 11.0f,  5.0f,11.0f,   0.0f,  9.0f,   5.5f, 7.0f,  3.5f, 180.0f },
    { "Toilet Lau",      2.0f,  8.0f,  5.0f,11.0f,  -7.5f, -1.5f,   5.0f, 7.0f, -4.5f, 180.0f },
};
const int ROOM_COUNT = (int)(sizeof(g_rooms) / sizeof(g_rooms[0]));

// ==================== BIEN TOAN CUC ====================
CameraState g_cam;

// ==================== KHOI TAO ====================
void CameraInit(float startX, float startY, float startZ) {
    g_cam.x = startX;
    g_cam.y = startY;
    g_cam.z = startZ;
    g_cam.yaw = -90.0f;
    g_cam.pitch = -10.0f;

    g_cam.moveSpeed = 0.35f;
    g_cam.mouseSensitivity = 0.12f;
    g_cam.mode = CAM_FREE;
    g_cam.currentRoom = -1;
    g_cam.lastMouseX = -1;
    g_cam.lastMouseY = -1;
    g_cam.showHUD = true;

    for (int i = 0; i < 256; i++) g_cam.keys[i] = false;
}

// ==================== COLLISION - PHAT HIEN PHONG ====================
static int DetectRoom(float scale) {
    // Doi ve toa do logic
    float lx = g_cam.x / scale;
    float ly = g_cam.y / scale;
    float lz = g_cam.z / scale;

    for (int i = 0; i < ROOM_COUNT; i++) {
        if (lx >= g_rooms[i].minX && lx <= g_rooms[i].maxX &&
            ly >= g_rooms[i].minY && ly <= g_rooms[i].maxY &&
            lz >= g_rooms[i].minZ && lz <= g_rooms[i].maxZ) {
            return i;
        }
    }
    return -1;
}

// Giu camera trong AABB phong (chi dung o che do INTERIOR)
static void ClampToRoom(int ri, float scale) {
    float mg = 0.4f;  // margin (don vi logic)
    float minX = (g_rooms[ri].minX + mg) * scale, maxX = (g_rooms[ri].maxX - mg) * scale;
    float minZ = (g_rooms[ri].minZ + mg) * scale, maxZ = (g_rooms[ri].maxZ - mg) * scale;
    float floorY = (g_rooms[ri].minY + 1.7f) * scale;  // chieu cao mat nguoi

    if (g_cam.x < minX) g_cam.x = minX;
    if (g_cam.x > maxX) g_cam.x = maxX;
    if (g_cam.z < minZ) g_cam.z = minZ;
    if (g_cam.z > maxZ) g_cam.z = maxZ;
    if (g_cam.y < floorY) g_cam.y = floorY;
}

// ==================== CAP NHAT CAMERA ====================
void CameraUpdate(float scale) {
    float rad = (float)(g_cam.yaw * M_PI / 180.0);

    // Vector huong tien (chi theo mat phang XZ)
    float fwX = cosf(rad);
    float fwZ = sinf(rad);
    // Vector sang phai
    float rtX = cosf(rad - (float)M_PI / 2.0f);
    float rtZ = sinf(rad - (float)M_PI / 2.0f);

    // Toc do: Shift => chay nhanh
    float spd = g_cam.moveSpeed;
    if (g_cam.keys[16]) spd *= 2.5f;  // VK_SHIFT (ascii 16 = Shift trong mot so truong hop)
    // Shift tren mot so keyboard co the la ascii khac, nen kiem tra ca:
    // glutGetModifiers() khong co san o day, dung phim Z lam "chay":
    if (g_cam.keys['z'] || g_cam.keys['Z']) spd *= 2.5f;

    if (g_cam.keys['w'] || g_cam.keys['W']) { g_cam.x += fwX * spd; g_cam.z += fwZ * spd; }
    if (g_cam.keys['s'] || g_cam.keys['S']) { g_cam.x -= fwX * spd; g_cam.z -= fwZ * spd; }
    if (g_cam.keys['a'] || g_cam.keys['A']) { g_cam.x -= rtX * spd; g_cam.z -= rtZ * spd; }
    if (g_cam.keys['d'] || g_cam.keys['D']) { g_cam.x += rtX * spd; g_cam.z += rtZ * spd; }
    if (g_cam.keys['q'] || g_cam.keys['Q']) g_cam.y += spd;
    if (g_cam.keys['e'] || g_cam.keys['E']) g_cam.y -= spd;

    // Clamp pitch
    if (g_cam.pitch > 89.0f) g_cam.pitch = 89.0f;
    if (g_cam.pitch < -89.0f) g_cam.pitch = -89.0f;

    // Phat hien phong
    int prev = g_cam.currentRoom;
    g_cam.currentRoom = DetectRoom(scale);

    // In console khi buoc vao phong moi
    if (g_cam.currentRoom != prev && g_cam.currentRoom >= 0) {
        printf("[Camera] Vao phong: %s\n", g_rooms[g_cam.currentRoom].name);
    }

    // Che do NOI THAT: khoa cao do mat nguoi + collision tuong
    if (g_cam.mode == CAM_INTERIOR && g_cam.currentRoom >= 0) {
        ClampToRoom(g_cam.currentRoom, scale);
    }
}

// ==================== AP DUNG CAMERA (thay gluLookAt cu) ====================
void CameraApply() {
    float ry = (float)(g_cam.yaw * M_PI / 180.0);
    float rp = (float)(g_cam.pitch * M_PI / 180.0);

    float lookX = g_cam.x + cosf(rp) * cosf(ry);
    float lookY = g_cam.y + sinf(rp);
    float lookZ = g_cam.z + cosf(rp) * sinf(ry);

    gluLookAt(
        g_cam.x, g_cam.y, g_cam.z,
        lookX, lookY, lookZ,
        0.0f, 1.0f, 0.0f
    );
}

// ==================== XU LY CHUOT ====================
void CameraMouseMotion(int x, int y) {
    if (g_cam.lastMouseX == -1) {
        g_cam.lastMouseX = x;
        g_cam.lastMouseY = y;
        return;
    }
    float dx = (float)(x - g_cam.lastMouseX);
    float dy = (float)(y - g_cam.lastMouseY);

    g_cam.yaw += dx * g_cam.mouseSensitivity * 20.0f;
    g_cam.pitch -= dy * g_cam.mouseSensitivity * 20.0f;

    g_cam.lastMouseX = x;
    g_cam.lastMouseY = y;
}

// ==================== XU LY PHIM ====================
void CameraKeyDown(unsigned char key, int /*x*/, int /*y*/) {
    g_cam.keys[(unsigned char)key] = true;

    // Tab: chuyen che do
    if (key == '\t') {
        if (g_cam.mode == CAM_FREE) {
            g_cam.mode = CAM_INTERIOR;
            printf("[Camera] Che do: NOI THAT (collision bat)\n");
        }
        else {
            g_cam.mode = CAM_FREE;
            printf("[Camera] Che do: TU DO (fly)\n");
        }
    }

    // H: an/hien HUD
    if (key == 'h' || key == 'H') {
        g_cam.showHUD = !g_cam.showHUD;
    }

    // Phim so 1-7: teleport nhanh vao tung phong
    if (key >= '1' && key <= '9') {
        int idx = (int)(key - '1');
        if (idx < ROOM_COUNT) {
            CameraTeleportToRoom(idx);
        }
    }
}

void CameraKeyUp(unsigned char key, int /*x*/, int /*y*/) {
    g_cam.keys[(unsigned char)key] = false;
}

// ==================== TELEPORT ====================
void CameraTeleportToRoom(int ri) {
    if (ri < 0 || ri >= ROOM_COUNT) return;
    float scale = 4.5f;  // GLOBAL_SCALE

    g_cam.x = g_rooms[ri].spawnX * scale;
    g_cam.y = g_rooms[ri].spawnY * scale;
    g_cam.z = g_rooms[ri].spawnZ * scale;
    g_cam.yaw = g_rooms[ri].spawnYaw;
    g_cam.pitch = -5.0f;
    g_cam.mode = CAM_INTERIOR;
    g_cam.currentRoom = ri;

    printf("[Camera] Teleport -> %s\n", g_rooms[ri].name);
}

// ==================== HUD NOI THAT (thuan OpenGL 2D) ====================
// Ve chu bang GLUT bitmap font
static void glPrint(float x, float y, const char* text, void* font = GLUT_BITMAP_8_BY_13) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void CameraDrawHUD(int winW, int winH) {
    if (!g_cam.showHUD) return;

    // ---- Chuyen sang 2D Ortho ----
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // ---- Ve nen panel (goc phai duoi) ----
    int pw = 270, ph = 185;
    int px = winW - pw - 8;
    int py = 8;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Nen toi mo duc
    glColor4f(0.0f, 0.05f, 0.12f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f((float)px, (float)py);
    glVertex2f((float)(px + pw), (float)py);
    glVertex2f((float)(px + pw), (float)(py + ph));
    glVertex2f((float)px, (float)(py + ph));
    glEnd();

    // Vien sang
    glColor4f(0.3f, 0.7f, 1.0f, 0.9f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f((float)px, (float)py);
    glVertex2f((float)(px + pw), (float)py);
    glVertex2f((float)(px + pw), (float)(py + ph));
    glVertex2f((float)px, (float)(py + ph));
    glEnd();

    glDisable(GL_BLEND);

    // ---- In chu ----
    char buf[128];
    int tx = px + 8;
    int ty = py + ph - 18;

    // Tieu de
    glColor3f(0.4f, 0.9f, 1.0f);
    glPrint((float)tx, (float)ty, "== CAMERA NOI THAT ==", GLUT_BITMAP_9_BY_15);
    ty -= 18;

    // Che do
    glColor3f(1.0f, 0.75f, 0.2f);
    if (g_cam.mode == CAM_INTERIOR)
        glPrint((float)tx, (float)ty, "Mode: NOI THAT  [Tab de doi]");
    else
        glPrint((float)tx, (float)ty, "Mode: TU DO     [Tab de doi]");
    ty -= 15;

    // Phong hien tai
    if (g_cam.currentRoom >= 0) {
        snprintf(buf, sizeof(buf), "Phong: %s", g_rooms[g_cam.currentRoom].name);
        glColor3f(0.4f, 1.0f, 0.5f);
    }
    else {
        snprintf(buf, sizeof(buf), "Vi tri: Ngoai troi");
        glColor3f(1.0f, 0.85f, 0.3f);
    }
    glPrint((float)tx, (float)ty, buf);
    ty -= 15;

    // Toa do
    snprintf(buf, sizeof(buf), "Pos: (%.1f, %.1f, %.1f)", g_cam.x, g_cam.y, g_cam.z);
    glColor3f(0.75f, 0.75f, 0.75f);
    glPrint((float)tx, (float)ty, buf);
    ty -= 18;

    // Huong dan phim teleport
    glColor3f(0.9f, 0.9f, 0.9f);
    glPrint((float)tx, (float)ty, "-- Teleport nhanh --");
    ty -= 14;

    const char* shortcuts[] = {
        "1: Phong Khach",
        "2: Bep & An uong",
        "3: WC Tang Tret",
        "4: Phong Ngu 1",
        "5: Phong Ngu 2",
        "6: Phong Ngu 3",
        "7: Toilet Lau",
    };
    glColor3f(0.8f, 0.85f, 1.0f);
    for (int i = 0; i < ROOM_COUNT; i++) {
        // 2 cot de tiet kiem chieu cao
        if (i % 2 == 0) {
            glPrint((float)tx, (float)ty, shortcuts[i]);
        }
        else {
            glPrint((float)(tx + 130), (float)ty, shortcuts[i]);
            ty -= 13;
        }
    }
    ty -= 13;

    glColor3f(0.6f, 0.6f, 0.6f);
    glPrint((float)tx, (float)ty, "Z+WASD: chay | H: an/hien HUD");

    // ---- Khoi phuc trang thai OpenGL ----
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ================================================================
// HUONG DAN TICH HOP VAO DoAn_DoHoa_Fixed.cpp
// ================================================================
//
// BUOC 1 - Them include (sau #include "stb_image.h"):
//   #include "InteriorCamera.h"
//
// BUOC 2 - Comment out cac bien camera cu (dong ~18-26):
//   // float camX=0,camY=30,camZ=55; float camYaw=-90,camPitch=-15;
//   // float moveSpeed=0.8f; float mouseSensitivity=0.1f;
//   // bool keys[256]={false}; int lastMouseX=-1,lastMouseY=-1;
//
// BUOC 3 - Sua ham init(), them o cuoi:
//   CameraInit(0.0f, 2.5f*4.5f, 12.0f*4.5f);
//
// BUOC 4 - Trong renderScene(), thay 7 dong gluLookAt cu:
//   // float radYaw = camYaw * 3.14159f / 180.0f; ...
//   // gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, ...);
//   CameraApply();  // <- thay the
//   // Them o cuoi truoc glutSwapBuffers():
//   CameraDrawHUD(1280, 720);
//
// BUOC 5 - Trong update(), thay phan xu ly phim di chuyen:
//   // if (keys['w']) { camX += ... }  ... xoa het doan nay
//   CameraUpdate(GLOBAL_SCALE);  // <- thay the
//
// BUOC 6 - Cac ham callback:
//   void keyboard(unsigned char key, int x, int y) {
//       CameraKeyDown(key, x, y);  // <- THEM DONG NAY DAU TIEN
//       switch(key) { ...giu nguyen... }
//   }
//   void keyboardUp(unsigned char key, int x, int y) {
//       CameraKeyUp(key, x, y);    // <- THAY THE TOAN BO
//   }
//   void mouseMotion(int x, int y)  { CameraMouseMotion(x, y); }
//   void mousePassive(int x, int y) { CameraMouseMotion(x, y); }
//
// ================================================================
// PHIM DIEU KHIEN SAU KHI TICH HOP:
// ================================================================
//  WASD        : Di chuyen (giu nguyen)
//  Q / E       : Bay len / xuong (giu nguyen)
//  Z + WASD    : Chay nhanh (MOI)
//  Tab         : Doi che do Tu Do <-> Noi That (MOI)
//  1           : Teleport Phong Khach (MOI)
//  2           : Teleport Bep & An uong (MOI)
//  3           : Teleport WC Tang Tret (MOI)
//  4           : Teleport Phong Ngu 1 (MOI)
//  5           : Teleport Phong Ngu 2 (MOI)
//  6           : Teleport Phong Ngu 3 (MOI)
//  7           : Teleport Toilet Lau (MOI)
//  H           : An / hien HUD noi that (MOI)
//  O/K/C/F/T/G/R : Giu nguyen nhu cu
//  ESC         : Thoat
// ================================================================