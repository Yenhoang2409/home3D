#include "GroundFloorWC.h"

// 1. LÝ THUYẾT CHIẾU SÁNG & TÔ BÓNG (SHADING)
// Ánh sáng phản xạ nhận được phụ thuộc vào loại nguồn sáng và góc tạo bởi mặt phản xạ với tia sáng[cite: 1030, 1031].
// Hàm này thiết lập vật liệu (Material) để Gouraud Shading của OpenGL tính toán nội suy màu sắc[cite: 1049, 1050].
void setWCMaterial(float r, float g, float b, float shininess, float specularIntensity) {
    GLfloat ambient[] = { r * 0.4f, g * 0.4f, b * 0.4f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { specularIntensity, specularIntensity, specularIntensity, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

// 2. LÝ THUYẾT BIẾN ĐỔI (TRANSFORMATIONS)
// Sử dụng các phép Tịnh tiến (Translation) và Biến đổi tỉ lệ (Scaling) để đưa vật thể từ tọa độ cục bộ ra thế giới thực[cite: 556, 557, 781, 786].

void drawModernToilet(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z); // Tịnh tiến T=(tx, ty, tz) [cite: 785]

    // Vật liệu gốm sứ trắng bóng (Độ phản xạ cao)
    setWCMaterial(0.92f, 0.95f, 0.95f, 90.0f, 0.7f);

    // Bệ ngồi (Bowl)
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.2f);
    glScalef(0.7f, 0.5f, 0.9f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Két nước (Tank)
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, -0.2f);
    glScalef(0.7f, 0.7f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Nút xả (Inox)
    setWCMaterial(0.8f, 0.8f, 0.8f, 128.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.15f, -0.2f);
    glScalef(0.15f, 0.02f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void drawSink(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Tủ dưới bồn rửa (Vật liệu nhám, độ phản xạ thấp)
    setWCMaterial(0.25f, 0.25f, 0.25f, 10.0f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f);
    glScalef(1.2f, 0.7f, 0.8f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Mặt đá/Chậu rửa gốm sứ
    setWCMaterial(0.95f, 0.95f, 0.95f, 100.0f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glScalef(1.3f, 0.1f, 0.9f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Vòi nước inox (Phản quang mạnh)
    setWCMaterial(0.7f, 0.7f, 0.75f, 128.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.9f, -0.25f);
    glScalef(0.05f, 0.25f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.0f, -0.15f);
    glScalef(0.05f, 0.05f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void drawMirror(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Viền gương mỏng tối màu
    setWCMaterial(0.1f, 0.1f, 0.1f, 5.0f, 0.1f);
    glPushMatrix();
    glScalef(1.25f, 1.05f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Mặt gương
    // Để tạo cảm giác mặt gương trong OpenGL cơ bản, ta dùng màu xanh nhạt và đẩy Specular lên tối đa để hứng ánh sáng[cite: 1030].
    setWCMaterial(0.75f, 0.85f, 0.95f, 128.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.015f); // Đẩy mặt gương ra trước viền một chút
    glScalef(1.2f, 1.0f, 0.01f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void drawGroundFloorWC(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // TẮT GL_COLOR_MATERIAL để fix dứt điểm lỗi cháy sáng
    // Ép OpenGL sử dụng hoàn toàn thông số phản quang (Specular) của setWCMaterial
    glDisable(GL_COLOR_MATERIAL);

    // Bố trí vật dụng trong không gian WC
    drawModernToilet(0.0f, 0.0f, 0.0f);       // Bồn cầu
    drawSink(-1.5f, 0.0f, 0.2f);              // Bồn rửa tay
    drawMirror(-1.5f, 1.5f, -0.2f);           // Gương treo tường

    // BẬT LẠI để không ảnh hưởng đến các phòng khác trong nhà
    glEnable(GL_COLOR_MATERIAL);

    glPopMatrix();
}