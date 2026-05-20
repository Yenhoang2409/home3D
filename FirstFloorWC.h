#ifndef FIRST_FLOOR_WC_H
#define FIRST_FLOOR_WC_H

#include <windows.h>
#include <GL/glut.h>

// Hàm vẽ không gian WC chính trên lầu
void drawFirstFloorWC(float x, float y, float z);

// Hàm xử lý Click chuột để xả nước
void FirstFloorWCMouse(int button, int state, int x, int y);

// Hàm cập nhật animation nước chảy
void FirstFloorWCUpdate();

#endif