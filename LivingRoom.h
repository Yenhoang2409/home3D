#pragma once
// ==========================================================
// LivingRoom.h  -  Module Phong Khach 3D
// Tuong thich 100% voi DoAn_DoHoa_Fixed.cpp
//
// Cac bien sau da duoc DINH NGHIA o file chinh, khong dinh
// nghia lai o day:
//   bool   isTransparentPass
//   GLuint texWood
//   bool   tvOn
//   float  tvR, tvG, tvB
// ==========================================================

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>

// ------ Bien tu file chinh (chi extern, khong dinh nghia) ------
extern bool   isTransparentPass;
extern GLuint texWood;
extern bool   tvOn;
extern float  tvR, tvG, tvB;

// ------ Bien RIENG cua module nay (defined trong LivingRoom.cpp) ------
// Vi tri & goc xoay Sofa
extern float sofaX, sofaZ, sofaAngle;
// Vi tri & goc xoay Ban tra
extern float tableX, tableZ, tableAngle;
// Doi tuong dang chon: 0=khong, 1=sofa, 2=ban
extern int   lvSelectedObj;

// ------ Giao dien cong khai ------

// Ham ve tong the phong khach (goi tu drawGroundFloor)
void drawLivingRoomInterior();

// Xu ly phim: goi trong keyboard() cua file chinh
// Phim dieu khien phong khach:
//   1        : Chon/bo chon Sofa
//   2        : Chon/bo chon Ban tra
//   3        : Bo chon tat ca
//   I/K      : Di chuyen doi tuong len/xuong (truc Z)
//   J/L      : Di chuyen doi tuong trai/phai (truc X)
//   U/O      : Xoay doi tuong trai/phai
void LivingRoomKeyboard(unsigned char key, int x, int y);

// Cap nhat animation (goi trong update() cua file chinh moi frame)
// dt: thoi gian giua 2 frame (giay), thuong la 0.016f
void LivingRoomUpdate(float dt);