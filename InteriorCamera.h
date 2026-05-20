#pragma once
// ================================================================
// InteriorCamera.h  -  He thong camera noi that
// Thuan OpenGL 1.x + GLUT + GLU, khong them thu vien ngoai
// ================================================================

#define NOMINMAX
#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>

// ==================== CHE DO CAMERA ====================
enum CameraMode {
    CAM_FREE = 0,  // Bay tu do (giong code goc)
    CAM_INTERIOR = 1,  // Di bo trong nha, co collision AABB
};

// ==================== THONG TIN MOI PHONG ====================
struct Room {
    const char* name;
    // Gioi han AABB  (don vi LOGIC, truoc khi nhan GLOBAL_SCALE)
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
    // Spawn point (don vi logic)
    float spawnX, spawnY, spawnZ;
    float spawnYaw;   // Huong nhin (do) khi teleport vao phong
};

// ==================== TRANG THAI CAMERA ====================
struct CameraState {
    float x, y, z;           // Vi tri world (da nhan scale)
    float yaw, pitch;         // Goc xoay (do)
    float moveSpeed;
    float mouseSensitivity;
    CameraMode mode;
    int   currentRoom;        // -1 = ngoai troi
    bool  keys[256];
    int   lastMouseX, lastMouseY;
    bool  showHUD;
};

// ==================== BIEN GLOBAL ====================
extern CameraState  g_cam;
extern const Room   g_rooms[];
extern const int    ROOM_COUNT;

// ==================== GIAO DIEN PUBLIC ====================

// Khoi tao - goi 1 lan trong init()
// startX/Y/Z: vi tri bat dau (toa do world = logic * scale)
void CameraInit(float startX, float startY, float startZ);

// Cap nhat vi tri & collision - goi trong update()
// scale = GLOBAL_SCALE cua ban (4.5f)
void CameraUpdate(float scale);

// Ap dung camera - thay the gluLookAt() trong renderScene()
void CameraApply();

// Callback GLUT - noi vao ham tuong ung cua ban
void CameraKeyDown(unsigned char key, int x, int y);
void CameraKeyUp(unsigned char key, int x, int y);
void CameraMouseMotion(int x, int y);

// Teleport ngay vao phong (0-based index)
void CameraTeleportToRoom(int roomIdx);

// Ve panel HUD noi that - goi cuoi renderScene() truoc glutSwapBuffers
void CameraDrawHUD(int winW, int winH);