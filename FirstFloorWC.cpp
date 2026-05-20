#include "FirstFloorWC.h"
#include <cmath>
#include <cstdlib>

extern bool isTransparentPass;

// Các biến trạng thái của Vòi hoa sen
bool isShowerOn = false;
float distToShower = 999.0f; // Khoảng cách từ Camera đến vòi sen
float waterOffset = 0.0f;    // Tạo hiệu ứng nước rơi

// Hàm animation nước chảy (Gọi trong update)
void FirstFloorWCUpdate() {
    if (isShowerOn) {
        waterOffset += 0.15f; // Tốc độ nước rơi
    }
}

// Xử lý Click chuột trái khi đứng gần vòi sen
void FirstFloorWCMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Nếu đứng cách vòi sen dưới 15.0f thì click sẽ bật/tắt nước
        if (distToShower < 15.0f) {
            isShowerOn = !isShowerOn;
        }
    }
}

// Hàm gán vật liệu
static void wc_mat(float r, float g, float b, float shininess, float specular) {
    GLfloat ambient[] = { r * 0.5f, g * 0.5f, b * 0.5f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat spec[] = { specular, specular, specular, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glColor3f(r, g, b);
}

// 1. TOILET
static void drawUpstairsToilet(float x, float y, float z) {
    if (isTransparentPass) return;
    glPushMatrix(); glTranslatef(x, y, z);
    wc_mat(0.95f, 0.95f, 0.95f, 90.0f, 0.8f);
    glPushMatrix(); glTranslatef(0.0f, 0.3f, 0.2f); glScalef(0.7f, 0.5f, 0.9f); glutSolidCube(1.0); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.8f, -0.2f); glScalef(0.7f, 0.7f, 0.3f); glutSolidCube(1.0); glPopMatrix();
    wc_mat(0.8f, 0.8f, 0.8f, 128.0f, 1.0f);
    glPushMatrix(); glTranslatef(0.0f, 1.15f, -0.2f); glScalef(0.15f, 0.02f, 0.05f); glutSolidCube(1.0); glPopMatrix();
    glPopMatrix();
}

// 2. BỒN RỬA MẶT TO RÕ HƠN
static void drawUpstairsSink(float x, float y, float z) {
    if (isTransparentPass) return;
    glPushMatrix(); glTranslatef(x, y, z);

    // Tủ dưới (Màu gỗ tối)
    wc_mat(0.3f, 0.2f, 0.15f, 10.0f, 0.1f);
    glPushMatrix(); glTranslatef(0.0f, 0.35f, 0.0f); glScalef(1.4f, 0.7f, 1.0f); glutSolidCube(1.0); glPopMatrix();

    // Bồn rửa (Sứ trắng sáng)
    wc_mat(1.0f, 1.0f, 1.0f, 100.0f, 0.9f);
    glPushMatrix(); glTranslatef(0.0f, 0.75f, 0.0f); glScalef(1.5f, 0.1f, 1.1f); glutSolidCube(1.0); glPopMatrix();

    // Gương soi (Xanh nhạt, bóng loáng)
    wc_mat(0.7f, 0.85f, 0.95f, 128.0f, 1.0f);
    glPushMatrix(); glTranslatef(0.0f, 1.6f, -0.45f); glScalef(1.2f, 1.2f, 0.02f); glutSolidCube(1.0); glPopMatrix();

    // Vòi nước (Inox bự và rõ)
    wc_mat(0.9f, 0.9f, 0.9f, 128.0f, 1.0f);
    glPushMatrix(); glTranslatef(0.0f, 0.9f, -0.3f); glScalef(0.06f, 0.3f, 0.06f); glutSolidCube(1.0); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 1.0f, -0.2f); glScalef(0.06f, 0.06f, 0.25f); glutSolidCube(1.0); glPopMatrix();
    glPopMatrix();
}

// 3. VÒI HOA SEN + KÍNH + HIỆU ỨNG NƯỚC
static void drawUpstairsShower(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // TÍNH KHOẢNG CÁCH TỪ CAMERA ĐẾN VÒI SEN ĐỂ BẬT HUD TƯƠNG TÁC
    GLfloat mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    distToShower = std::sqrt(mv[12] * mv[12] + mv[13] * mv[13] + mv[14] * mv[14]);

    if (!isTransparentPass) {
        // Củ sen và Vòi sen (Inox)
        wc_mat(0.9f, 0.9f, 0.9f, 128.0f, 1.0f);
        glPushMatrix(); glTranslatef(0.0f, 1.2f, -0.7f); glScalef(0.05f, 2.4f, 0.05f); glutSolidCube(1.0); glPopMatrix(); // Ống đứng
        glPushMatrix(); glTranslatef(0.0f, 2.4f, -0.4f); glScalef(0.05f, 0.05f, 0.6f); glutSolidCube(1.0); glPopMatrix(); // Ống ngang

        // Bát sen (Tròn to)
        glPushMatrix(); glTranslatef(0.0f, 2.35f, -0.1f); glScalef(0.3f, 0.05f, 0.3f); glutSolidSphere(1.0, 16, 16); glPopMatrix();

        // Củ sen và Núm vặn (Nơi để người dùng hiểu là chỗ bật nước)
        wc_mat(1.0f, 0.3f, 0.3f, 128.0f, 1.0f); // Núm vặn màu đỏ cho nổi bật
        glPushMatrix(); glTranslatef(0.0f, 1.0f, -0.65f); glScalef(0.2f, 0.15f, 0.2f); glutSolidCube(1.0); glPopMatrix();

        // Sàn đá tắm
        wc_mat(0.2f, 0.2f, 0.2f, 10.0f, 0.1f);
        glPushMatrix(); glTranslatef(0.0f, 0.02f, 0.0f); glScalef(1.5f, 0.04f, 1.5f); glutSolidCube(1.0); glPopMatrix();
    }

    if (isTransparentPass) {
        // Vách kính cường lực
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE);
        GLfloat dif[] = { 0.7f, 0.85f, 0.95f, 0.35f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif); glColor4f(0.7f, 0.85f, 0.95f, 0.35f);
        glPushMatrix(); glTranslatef(0.0f, 1.2f, 0.75f); glScalef(1.5f, 2.4f, 0.02f); glutSolidCube(1.0); glPopMatrix();
        glPushMatrix(); glTranslatef(-0.75f, 1.2f, 0.0f); glScalef(0.02f, 2.4f, 1.5f); glutSolidCube(1.0); glPopMatrix();

        // HIỆU ỨNG NƯỚC RƠI (Particle Lines)
        if (isShowerOn) {
            // FIX LỖI: Tạm thời vô hiệu hóa ánh sáng (Lighting) khi vẽ nước 
            // để đảm bảo các hạt nước được ép buộc vẽ bằng màu tĩnh glColor4f
            glDisable(GL_LIGHTING);

            glColor4f(0.6f, 0.8f, 1.0f, 0.8f); // Nước xanh lơ, độ đục 80% để dễ nhìn hơn
            glLineWidth(2.5f); // Làm dòng nước to hơn một chút

            glBegin(GL_LINES);
            for (int i = 0; i < 60; i++) {
                // Tọa độ tỏa ra từ bát sen: Bát sen nằm ở Z = -0.1f
                float px = (rand() % 100 - 50) / 250.0f;
                float pz = -0.1f + (rand() % 100 - 50) / 250.0f;

                // Nước rơi liên tục từ độ cao 2.3m xuống mặt sàn 0.0m
                float py = 2.3f - fmod((waterOffset + (rand() % 100) / 50.0f), 2.3f);

                // Tránh việc vạch kẻ nước đâm xuyên qua sàn (Y < 0.0)
                if (py - 0.2f > 0.0f) {
                    glVertex3f(px, py, pz);
                    glVertex3f(px, py - 0.2f, pz); // Chiều dài 1 giọt nước là 0.2f
                }
            }
            glEnd();

            // FIX LỖI: Trả lại trạng thái chiếu sáng cho các vật thể khác
            glEnable(GL_LIGHTING);
        }
        glDepthMask(GL_TRUE); glDisable(GL_BLEND);
    }

    // HUD GỢI Ý CLICK CHUỘT
    if (distToShower < 15.0f && !isTransparentPass) {
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
        glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);

        const char* hint = isShowerOn ? "CLICK CHUOT TRAI de TAT voi sen" : "CLICK CHUOT TRAI de BAT voi sen";
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(282, 100);
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        glColor3f(0.2f, 1.0f, 0.4f); // Xanh lá cây nổi bật
        glRasterPos2f(280, 102);
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    }

    glPopMatrix();
}

// 4. HÀM TỔNG HỢP PHÒNG TẮM
void drawFirstFloorWC(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // TẠO ĐÈN TRẦN ĐỂ PHÒNG SÁNG RỰC RỠ (Hết bị tối)
    if (!isTransparentPass) {
        glEnable(GL_LIGHT2);
        GLfloat light2Pos[] = { 0.0f, 4.8f, 0.0f, 1.0f }; // Đèn đặt trên trần nhà tắm
        GLfloat light2Dif[] = { 0.85f, 0.85f, 0.85f, 1.0f }; // Ánh sáng trắng
        glLightfv(GL_LIGHT2, GL_POSITION, light2Pos);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, light2Dif);
    }

    glDisable(GL_COLOR_MATERIAL);

    drawUpstairsToilet(-1.5f, 0.0f, -2.0f);
    drawUpstairsSink(1.5f, 0.0f, -2.0f);    // Bồn rửa mặt
    drawUpstairsShower(1.2f, 0.0f, 1.2f);   // Vòi hoa sen có núm đỏ

    glEnable(GL_COLOR_MATERIAL);

    if (!isTransparentPass) glDisable(GL_LIGHT2); // Tắt đèn phòng tắm để khỏi rọi sang phòng ngủ

    glPopMatrix();
}