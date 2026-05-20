#include "Stairs.h"

extern bool isTransparentPass; // Đồng bộ cơ chế Multi-Pass với file main

// Hàm thiết lập vật liệu mô phỏng thực tế
static void setStairsMaterial(float r, float g, float b, float shininess, float specIntensity) {
    GLfloat ambient[] = { r * 0.4f, g * 0.4f, b * 0.4f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { specIntensity, specIntensity, specIntensity, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glColor3f(r, g, b);
}

// Vẽ một khối hộp cơ bản để lắp ráp các bộ phận
static void stairs_drawCube(float x, float y, float z, float w, float h, float d) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(w, h, d);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawStairsWithBalustrade() {
    // -----------------------------------------------------------
    // PASS 1: VẼ BẬC THANG VÀ KHUNG ĐẶC (isTransparentPass == false)
    // -----------------------------------------------------------
    if (!isTransparentPass) {
        // Vẽ 9 bậc thang đoạn 1 (từ sàn lên chiếu nghỉ)
        setStairsMaterial(0.6f, 0.4f, 0.2f, 32.0f, 0.2f); // Vật liệu gỗ bậc thang
        for (int i = 0; i < 9; i++) {
            stairs_drawCube(1.5f - i * 0.4f, 0.6f + i * 0.25f, -3.0f, 0.4f, 0.1f, 2.5f);

            // Đèn LED chỉ hướng dưới mỗi bậc (Phát xạ năng lượng cao)
            setStairsMaterial(1.0f, 0.8f, 0.2f, 128.0f, 1.0f);
            GLfloat emission[] = { 1.0f, 0.8f, 0.2f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
            stairs_drawCube(1.5f - i * 0.4f, 0.5f + i * 0.25f, -1.7f, 0.4f, 0.05f, 0.05f);
            GLfloat no_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, no_emission);
            setStairsMaterial(0.6f, 0.4f, 0.2f, 32.0f, 0.2f);
        }

        // Vẽ mặt sàn Chiếu nghỉ trung tâm
        stairs_drawCube(-2.0f, 2.85f, -3.0f, 1.5f, 0.1f, 2.5f);

        // Vẽ 9 bậc thang đoạn 2 (từ chiếu nghỉ ngược lên lầu 1)
        for (int i = 0; i < 9; i++) {
            stairs_drawCube(-2.0f, 3.1f + i * 0.25f, -1.5f + i * 0.4f, 1.5f, 0.1f, 0.4f);
        }

        // --- VẼ THANH TAY VỊN INOX AN TOÀN ---
        setStairsMaterial(0.85f, 0.85f, 0.88f, 128.0f, 1.0f); // Vật liệu Inox gương bóng bóng

        // Trụ đứng đỡ tay vịn đoạn 1
        stairs_drawCube(1.5f, 1.3f, -1.75f, 0.04f, 1.4f, 0.04f);   // Trụ tại chân thang
        stairs_drawCube(-1.7f, 3.3f, -1.75f, 0.04f, 1.4f, 0.04f);  // Trụ tại chiếu nghỉ

        // FIX THANH TAY VỊN ĐOẠN 1: Đổi góc quay từ 32.0f sang -32.0f để chạy dốc xuống chuẩn theo bậc thang
        glPushMatrix();
        glTranslatef(-0.1f, 2.3f, -1.75f);
        glRotatef(-32.0f, 0.0f, 0.0f, 1.0f);
        stairs_drawCube(0.0f, 0.0f, 0.0f, 3.8f, 0.05f, 0.05f);
        glPopMatrix();

        // Trụ đứng đỡ tay vịn đoạn 2
        stairs_drawCube(-1.3f, 3.55f, -1.5f, 0.04f, 1.4f, 0.04f);
        stairs_drawCube(-1.3f, 5.55f, 1.7f, 0.04f, 1.4f, 0.04f);

        // Thanh tay vịn chéo dọc đoạn 2
        glPushMatrix();
        glTranslatef(-1.3f, 4.55f, 0.1f);
        glRotatef(-32.0f, 1.0f, 0.0f, 0.0f);
        stairs_drawCube(0.0f, 0.0f, 0.0f, 0.05f, 0.05f, 3.8f);
        glPopMatrix();
    }

    // -----------------------------------------------------------
    // PASS 2: VẼ VÁCH KÍNH CƯỜNG LỰC TRONG SUỐT (isTransparentPass == true)
    // -----------------------------------------------------------
    if (isTransparentPass) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        // Cấu hình kính cường lực
        GLfloat diffuse[] = { 0.7f, 0.9f, 1.0f, 0.25f }; // Alpha 25%
        GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 120.0f);
        glColor4f(0.7f, 0.9f, 1.0f, 0.25f);

        // FIX VÁCH KÍNH ĐOẠN 1: Đổi góc quay từ 32.0f sang -32.0f để vách kính song song với độ dốc thực tế
        glPushMatrix();
        glTranslatef(-0.1f, 1.85f, -1.75f);
        glRotatef(-32.0f, 0.0f, 0.0f, 1.0f);
        stairs_drawCube(0.0f, 0.0f, 0.0f, 3.6f, 0.7f, 0.02f);
        glPopMatrix();

        // Vách kính bảo vệ đoạn 2
        glPushMatrix();
        glTranslatef(-1.3f, 4.1f, 0.1f);
        glRotatef(-32.0f, 1.0f, 0.0f, 0.0f);
        stairs_drawCube(0.0f, 0.0f, 0.0f, 0.02f, 0.7f, 3.6f);
        glPopMatrix();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}