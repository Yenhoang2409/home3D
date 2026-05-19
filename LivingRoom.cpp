#include "LivingRoom.h"

// Mượn các hàm và biến từ file chính (DoAn_DoHoa_Fixed.cpp)
extern void drawTexturedBox(float w, float h, float d);
extern bool tvOn;
extern float tvR, tvG, tvB;
extern bool isTransparentPass; // Biến dùng để quản lý vẽ vật thể trong suốt
extern GLuint texWood;         // Mượn ảnh vân gỗ có sẵn

// ==========================================================
// 1. SOFA CHỮ L (HIERARCHICAL MODELING)
// ==========================================================
void drawSofa() {
    if (isTransparentPass) return; // Sofa không trong suốt nên bỏ qua ở pass 2

    glPushMatrix();
    // Vật liệu vải nhám (Không có độ bóng - Specular = 0)
    GLfloat fabric_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fabric_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

    // Màu xám thanh lịch cho Sofa
    glColor3f(0.5f, 0.5f, 0.55f);

    // Băng ghế dài
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    drawTexturedBox(4.0f, 0.6f, 1.2f);
    glPopMatrix();

    // Băng ghế góc chữ L
    glPushMatrix();
    glTranslatef(1.4f, 0.3f, 1.6f);
    drawTexturedBox(1.2f, 0.6f, 2.0f);
    glPopMatrix();

    // Tựa lưng
    glColor3f(0.45f, 0.45f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, 0.9f, -0.4f);
    drawTexturedBox(4.0f, 0.8f, 0.4f);
    glPopMatrix();

    // Tựa tay 2 bên
    glColor3f(0.4f, 0.4f, 0.45f);
    glPushMatrix(); glTranslatef(-1.8f, 0.7f, 0.0f); drawTexturedBox(0.4f, 0.6f, 1.2f); glPopMatrix();
    glPushMatrix(); glTranslatef(1.8f, 0.7f, 0.0f); drawTexturedBox(0.4f, 0.6f, 1.2f); glPopMatrix();

    glPopMatrix();
}

// ==========================================================
// 2. BÀN TRÀ KÍNH (ALPHA BLENDING)
// ==========================================================
void drawCoffeeTable() {
    glPushMatrix();

    // -- KHUNG BÀN VÀ CHÂN BÀN (Vẽ ở Pass 1 - Không trong suốt) --
    if (!isTransparentPass) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texWood);
        glColor3f(1.0f, 1.0f, 1.0f);

        // Chân bàn
        glPushMatrix(); glTranslatef(-0.9f, 0.2f, -0.5f); drawTexturedBox(0.1f, 0.4f, 0.1f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.9f, 0.2f, -0.5f); drawTexturedBox(0.1f, 0.4f, 0.1f); glPopMatrix();
        glPushMatrix(); glTranslatef(-0.9f, 0.2f, 0.5f); drawTexturedBox(0.1f, 0.4f, 0.1f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.9f, 0.2f, 0.5f); drawTexturedBox(0.1f, 0.4f, 0.1f); glPopMatrix();

        // Giá đỡ mặt kính
        glPushMatrix(); glTranslatef(0.0f, 0.4f, 0.0f); drawTexturedBox(1.9f, 0.05f, 1.1f); glPopMatrix();
        glDisable(GL_TEXTURE_2D);
    }
    // -- MẶT KÍNH (Vẽ ở Pass 2 - Có trong suốt) --
    else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GLfloat glass_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glass_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0f);

        glColor4f(0.2f, 0.6f, 0.8f, 0.4f); // Hệ số Alpha = 0.4 (Trong suốt 60%)
        glPushMatrix();
        glTranslatef(0.0f, 0.45f, 0.0f);
        drawTexturedBox(2.2f, 0.05f, 1.4f); // Mặt kính to hơn giá đỡ
        glPopMatrix();

        glDisable(GL_BLEND);
        GLfloat default_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, default_specular);
    }

    glPopMatrix();
}

// ==========================================================
// 3. HỆ THỐNG TV VÀ KỆ (DYNAMIC EMISSION)
// ==========================================================
void drawTVSystem() {
    if (isTransparentPass) return;
    glPushMatrix();

    // 3.1. Kệ TV (Gỗ)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texWood);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    drawTexturedBox(6.0f, 0.6f, 1.0f);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    // 3.2. Màn hình TV 3D treo tường
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, -0.4f); // Đẩy TV lên cao và lùi vào tường

    // Khung TV (Nhựa đen bóng)
    GLfloat tv_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tv_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90.0f);

    glColor3f(0.05f, 0.05f, 0.05f);
    drawTexturedBox(4.2f, 2.4f, 0.1f); // Khung viền mỏng 3D

    // Màn hình hiển thị
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.06f); // Nhô ra phía trước khung một chút

    if (tvOn) {
        // Thuật toán Emission: Lấy màu random từ hàm Update làm ánh sáng phát ra
        GLfloat screen_emission[] = { tvR, tvG, tvB, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, screen_emission);
        glColor3f(tvR, tvG, tvB);
    }
    else {
        // Khi tắt TV thì màn hình đen xì không phát sáng
        GLfloat screen_off[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, screen_off);
        glColor3f(0.02f, 0.02f, 0.02f);
    }

    drawTexturedBox(4.0f, 2.2f, 0.02f); // Kích thước màn hình nhỏ hơn viền
    glPopMatrix(); // Kết thúc màn hình

    // Reset ánh sáng để không dính sang vật khác
    GLfloat default_mat[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, default_mat);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, default_mat);
    glPopMatrix(); // Kết thúc TV

    glPopMatrix(); // Kết thúc hệ thống TV
}

// ==========================================================
// 4. HÀM TỔNG HỢP GỌI RA Ở FILE CHÍNH
// ==========================================================
void drawLivingRoomInterior() {
    glPushMatrix();

    // Tịnh tiến tới trung tâm phòng khách hiện tại của bạn
    glTranslatef(-7.5f, 0.0f, 4.0f);

    // Trải thảm lông (Màu trắng kem)
    if (!isTransparentPass) {
        glColor3f(0.9f, 0.9f, 0.85f);
        glPushMatrix();
        glTranslatef(0.0f, 0.02f, 0.0f); // Nổi lên trên sàn một chút
        drawTexturedBox(5.0f, 0.04f, 4.0f);
        glPopMatrix();
    }

    // Đặt hệ thống Sofa (Lệch sang trái)
    glPushMatrix();
    glTranslatef(-0.5f, 0.0f, 0.0f);
    drawSofa();
    glPopMatrix();

    // Đặt bàn trà thủy tinh (Ngay trước Sofa)
    glPushMatrix();
    glTranslatef(-0.5f, 0.0f, -1.8f);
    drawCoffeeTable();
    glPopMatrix();

    // Đặt Kệ và TV treo tường (Quay mặt đối diện Sofa)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -3.8f);
    drawTVSystem();
    glPopMatrix();

    glPopMatrix();
}