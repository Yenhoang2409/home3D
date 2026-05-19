#include "Kitchen.h"
#include <iostream>

extern GLuint loadTexture(const char* filename);

// ==========================================================
// HÀM VẼ KHỐI HỘP CƠ BẢN
// ==========================================================
void drawTexturedBox(float w, float h, float d) {
    w /= 2.0f; h /= 2.0f; d /= 2.0f;

    glBegin(GL_QUADS);
    // Mặt trước (Z = d)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h, d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h, d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, d);
    // Mặt sau (Z = -d)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, -h, -d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, h, -d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(w, h, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(w, -h, -d);
    // Mặt trên (Y = h)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, h, d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, h, d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d);
    // Mặt dưới (Y = -h)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, -h, -d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(w, -h, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(w, -h, d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, -h, d);
    // Mặt phải (X = w)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h, -d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(w, h, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(w, -h, d);
    // Mặt trái (X = -w)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h, -d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, -h, d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, h, d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d);
    glEnd();
}

GLuint texMarble;
GLuint texCabinet;

void initKitchenTextures() {
    texMarble = loadTexture("img/marble.jpg");
    texCabinet = loadTexture("img/cabinet.png");
}

// ==========================================================
// CÁC MODULE CHÍNH CỦA BẾP
// ==========================================================
void drawKitchenCounter() {
    // 1. Tủ bếp dưới (Ốp ảnh)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texCabinet);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    drawTexturedBox(4.0f, 0.8f, 1.0f);
    glPopMatrix();

    // 2. Mặt bàn đá hoa cương
    glBindTexture(GL_TEXTURE_2D, texMarble);
    glPushMatrix();
    glTranslatef(0.0f, 0.85f, 0.0f);
    drawTexturedBox(4.2f, 0.1f, 1.2f);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawHangingCabinets() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texCabinet);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 2.2f, -0.1f);
    drawTexturedBox(4.0f, 0.8f, 0.6f);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

// THÊM MỚI: BỒN RỬA CHÉN VÀ VÒI NƯỚC (Vật liệu Inox)
void drawSink() {
    glPushMatrix();
    glTranslatef(-1.0f, 0.9f, 0.2f); // Phép Tịnh tiến (Translation)

    // Cấu hình vật liệu Inox (Phong Reflection)
    GLfloat inox_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, inox_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80.0f);

    // 1. Thành bồn rửa (Sử dụng Phép Tỉ lệ - Scaling)
    glColor3f(0.6f, 0.65f, 0.7f);
    glPushMatrix();
    // Kéo giãn khối 1x1x1 thành kích thước bồn
    glScalef(0.9f, 0.02f, 0.6f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // 2. Đáy bồn thụt xuống
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f);
    glColor3f(0.5f, 0.5f, 0.55f);
    glScalef(0.8f, 0.2f, 0.5f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // 3. Vòi nước (Kết hợp Tịnh tiến, Tỉ lệ và Phép Quay)
    glColor3f(0.8f, 0.85f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.2f); // Đưa gốc vòi ra mép bồn

    // Đoạn thân vòi đứng thẳng
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glScalef(0.06f, 0.4f, 0.06f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // Đoạn cổ vòi (Áp dụng Phép Quay - Rotation nghiêng 45 độ)
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f); // Dời lên đỉnh đoạn thẳng
    glRotatef(45.0f, 1.0f, 0.0f, 0.0f); // Quay 45 độ quanh trục X
    glTranslatef(0.0f, 0.0f, 0.1f); // Tịnh tiến dọc theo trục tọa độ mới
    glScalef(0.06f, 0.06f, 0.2f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // Đoạn đầu vòi chúi xuống (Quay tiếp)
    glPushMatrix();
    glTranslatef(0.0f, 0.47f, 0.14f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Quay 90 độ gập xuống
    glScalef(0.07f, 0.07f, 0.15f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glPopMatrix(); // Kết thúc cụm vòi nước

    // Reset vật liệu
    GLfloat default_spec[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, default_spec);
    glPopMatrix();
}

// ==========================================================
// MODULE 2: BẾP TỪ & HÚT MÙI (ÁP DỤNG EMISSION & SHEARING)
// ==========================================================
void drawStoveAndHood() {
    glPushMatrix();
    glTranslatef(1.2f, 0.91f, 0.2f);

    // 1. Mặt kính bếp từ
    GLfloat glass_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glass_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 120.0f);

    glColor3f(0.05f, 0.05f, 0.05f);
    glPushMatrix();
    glScalef(0.8f, 0.02f, 0.6f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // 2. Vòng nhiệt bếp từ (Vật liệu phát quang - Emission)
    GLfloat heat_emission[] = { 0.9f, 0.2f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, heat_emission);
    glColor3f(1.0f, 0.2f, 0.0f);

    glPushMatrix();
    glTranslatef(-0.2f, 0.02f, 0.0f);
    glScalef(0.25f, 0.01f, 0.25f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.2f, 0.02f, 0.0f);
    glScalef(0.2f, 0.01f, 0.2f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    GLfloat default_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, default_emission);

    // 3. Máy hút mùi (ÁP DỤNG MA TRẬN BIẾN DẠNG - SHEARING)
    GLfloat inox_specular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, inox_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0f);

    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    glColor3f(0.6f, 0.65f, 0.7f);

    // Đoạn vát chéo của máy hút mùi bằng Ma trận Shearing
    // Cột của ma trận (Column-major order trong OpenGL)
    GLfloat shearMatrix[16] = {
        1.0f,  0.0f,  0.0f,  0.0f, // Cột 1
        0.0f,  1.0f,  0.0f,  0.0f, // Cột 2
        0.0f, -0.6f,  1.0f,  0.0f, // Cột 3: Trục Z bị biến dạng theo Y (-0.6)
        0.0f,  0.0f,  0.0f,  1.0f  // Cột 4
    };

    glPushMatrix();
    glMultMatrixf(shearMatrix); // Nhân ma trận biến dạng vào ModelView
    glScalef(0.8f, 0.1f, 0.6f); // Scale sau khi đã biến dạng
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    // Ống khói vuông nối lên trần
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, -0.15f);
    glScalef(0.35f, 0.6f, 0.3f);
    drawTexturedBox(1.0f, 1.0f, 1.0f);
    glPopMatrix();

    glPopMatrix();

    GLfloat default_spec[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, default_spec);

    glPopMatrix();
}

void drawFridge() {
    glPushMatrix();
    glTranslatef(2.8f, 1.75f, 0.1f);

    // Lý thuyết 1: Phong Reflection cho Bề mặt Kim loại
    GLfloat metal_specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat metal_shininess[] = { 100.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, metal_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0f);

    // Thân tủ
    glColor3f(0.2f, 0.2f, 0.25f);
    drawTexturedBox(1.5f, 3.5f, 1.4f);

    // Cửa tủ
    glColor3f(0.75f, 0.8f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, 0.6f, 0.75f);
    drawTexturedBox(1.45f, 2.2f, 0.15f); // Ngăn mát
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, -1.2f, 0.75f);
    drawTexturedBox(1.45f, 1.0f, 0.15f); // Ngăn đá
    glPopMatrix();

    // Tay nắm nhựa
    GLfloat plastic_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, plastic_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);

    glColor3f(0.05f, 0.05f, 0.05f);
    glPushMatrix(); glTranslatef(-0.5f, 0.5f, 0.85f); drawTexturedBox(0.05f, 1.2f, 0.08f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.5f, -1.0f, 0.85f); drawTexturedBox(0.05f, 0.6f, 0.08f); glPopMatrix();

    // Lý thuyết 2: Emissive Material (Phát sáng LED)
    GLfloat led_emission[] = { 0.0f, 0.8f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, led_emission);

    // Màn hình & Lấy nước
    glColor3f(0.0f, 0.8f, 1.0f);
    glPushMatrix(); glTranslatef(0.3f, 1.0f, 0.83f); drawTexturedBox(0.2f, 0.1f, 0.02f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.3f, 0.7f, 0.84f); drawTexturedBox(0.1f, 0.1f, 0.01f); glPopMatrix();

    // Reset trạng thái
    GLfloat default_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat default_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, default_emission);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, default_specular);

    glPopMatrix();
}

// ==========================================================
// HÀM TỔNG HỢP GỌI RA Ở FILE CHÍNH
// ==========================================================
void drawKitchenInterior() {
    glPushMatrix();
    glTranslatef(5.0f, 0.0f, -2.5f);

    drawKitchenCounter();     // Bàn đá và tủ dưới
    drawHangingCabinets();    // Dàn tủ treo tường
    drawSink();               // Bồn rửa & vòi nước (MỚI)
    drawStoveAndHood();       // Bếp từ & máy hút mùi (MỚI)
    drawFridge();             // Tủ lạnh Inox LED

    glPopMatrix();
}