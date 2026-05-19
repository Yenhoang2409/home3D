// ==========================================================
// LivingRoom.cpp  -  Module Phong Khach 3D
// Tuong thich 100% voi DoAn_DoHoa_Fixed.cpp
//
// Cach tich hop vao du an:
//   1. Them #include "LivingRoom.h" vao DoAn_DoHoa_Fixed.cpp
//   2. Trong ham keyboard() cua file chinh, them:
//          LivingRoomKeyboard(key, x, y);
//   3. Trong ham update() cua file chinh, them:
//          LivingRoomUpdate(0.016f);
//   4. Ham drawLivingRoomInterior() da duoc goi san trong
//      drawGroundFloor() bang cach:
//          glPushMatrix();
//          glTranslatef(0.0f, 0.65f, 0.0f);
//          drawLivingRoomInterior();
//          glPopMatrix();
//
// Ky thuat ap dung (theo slide ly thuyet):
//   - Hierarchical Modeling : Sofa chu L (Push/PopMatrix)
//   - Phep bien doi          : Tinh tien, quay (Chap 6)
//   - Alpha Blending         : Mat ban tra kinh (2-Pass)
//   - Dynamic Emission       : Man hinh TV
//   - Ray Picking (AABB)     : Click chuot chon doi tuong
//   - Depth Buffer           : glDepthMask khi ve kinh
// ==========================================================

#include "LivingRoom.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

// ============================================================
// HANG SO NOI BO
// ============================================================
static const float LV_PI = 3.14159265f;
static const float LV_DEG2RAD = LV_PI / 180.0f;
static bool  lvLightOn = true; // bóng đèn

// Gioi han di chuyen trong pham vi phong khach (don vi noi bo)
static const float LV_BOUND_X = 4.5f;
static const float LV_BOUND_ZN = -3.5f;  // Z min (phia sau)
static const float LV_BOUND_ZP = 3.5f;  // Z max (phia truoc)

// ============================================================
// BIEN RIENG CUA MODULE (dinh nghia de file chinh extern)
// ============================================================
float sofaX = 0.0f;
float sofaZ = 2.2f;
float sofaAngle = 180.0f;

float tableX = 0.0f;
float tableZ = 0.4f;
float tableAngle = 0.0f;

int   lvSelectedObj = 0;   // 0=khong, 1=sofa, 2=ban

// ============================================================
// BIEN NOI BO (khong export ra ngoai)
// ============================================================
static float lvTvTimer = 0.0f;   // dem thoi gian cho mau TV
static float lvTvColorT = 0.0f;   // thoi gian mau sac TV (animation tron)

// ============================================================
// TIEN ICH: VE HOP CO TEXTURE (dung texWood cua file chinh)
// ============================================================
// Ve cuboid kich thuoc w x h x d, tam tai goc toa do hien tai.
// Moi mat co toa do texture de tile.
static void lv_drawBox(float w, float h, float d)
{
    float hw = w * 0.5f, hh = h * 0.5f, hd = d * 0.5f;
    glBegin(GL_QUADS);
    // +Z
    glNormal3f(0, 0, 1); glTexCoord2f(0, 0); glVertex3f(-hw, -hh, hd); glTexCoord2f(w, 0); glVertex3f(hw, -hh, hd); glTexCoord2f(w, h); glVertex3f(hw, hh, hd); glTexCoord2f(0, h); glVertex3f(-hw, hh, hd);
    // -Z
    glNormal3f(0, 0, -1); glTexCoord2f(0, 0); glVertex3f(hw, -hh, -hd); glTexCoord2f(w, 0); glVertex3f(-hw, -hh, -hd); glTexCoord2f(w, h); glVertex3f(-hw, hh, -hd); glTexCoord2f(0, h); glVertex3f(hw, hh, -hd);
    // -X
    glNormal3f(-1, 0, 0); glTexCoord2f(0, 0); glVertex3f(-hw, -hh, -hd); glTexCoord2f(d, 0); glVertex3f(-hw, -hh, hd); glTexCoord2f(d, h); glVertex3f(-hw, hh, hd); glTexCoord2f(0, h); glVertex3f(-hw, hh, -hd);
    // +X
    glNormal3f(1, 0, 0); glTexCoord2f(0, 0); glVertex3f(hw, -hh, hd); glTexCoord2f(d, 0); glVertex3f(hw, -hh, -hd); glTexCoord2f(d, h); glVertex3f(hw, hh, -hd); glTexCoord2f(0, h); glVertex3f(hw, hh, hd);
    // +Y
    glNormal3f(0, 1, 0); glTexCoord2f(0, 0); glVertex3f(-hw, hh, hd); glTexCoord2f(w, 0); glVertex3f(hw, hh, hd); glTexCoord2f(w, d); glVertex3f(hw, hh, -hd); glTexCoord2f(0, d); glVertex3f(-hw, hh, -hd);
    // -Y
    glNormal3f(0, -1, 0); glTexCoord2f(0, 0); glVertex3f(-hw, -hh, -hd); glTexCoord2f(w, 0); glVertex3f(hw, -hh, -hd); glTexCoord2f(w, d); glVertex3f(hw, -hh, hd); glTexCoord2f(0, d); glVertex3f(-hw, -hh, hd);
    glEnd();
}

// ============================================================
// DAT VAT LIEU - phu hop voi setMaterial cua file chinh
// ============================================================
static void lv_mat(float r, float g, float b,
    float sr, float sg, float sb, float shin,
    float er = 0, float eg = 0, float eb = 0)
{
    GLfloat dif[] = { r,  g,  b,  1.0f };
    GLfloat amb[] = { r * 0.35f, g * 0.35f, b * 0.35f, 1.0f };
    GLfloat spe[] = { sr, sg, sb, 1.0f };
    GLfloat emi[] = { er, eg, eb, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shin);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
    glColor3f(r, g, b);
}

// Tat emission ve 0 (reset sau khi ve TV)
static void lv_resetEmission()
{
    GLfloat z[] = { 0,0,0,1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, z);
}

// Highlight vat the dang chon (them mau vang nhe)
static void lv_applySelectionTint(bool selected)
{
    if (selected) {
        GLfloat em[] = { 0.12f, 0.10f, 0.02f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
    }
}

// ============================================================
// GIOI HAN VI TRI
// ============================================================
static void lv_clamp(float& x, float& z,
    float bx = LV_BOUND_X,
    float zn = LV_BOUND_ZN,
    float zp = LV_BOUND_ZP)
{
    if (x < -bx) x = -bx;
    if (x > bx) x = bx;
    if (z < zn) z = zn;
    if (z > zp) z = zp;
}

// ============================================================
// 1. SOFA CHU L  -  Hierarchical Modeling
//    Gom: ghe dai + ghe goc L + tua lung + tua tay + chan + goi
// ============================================================
static void drawSofa()
{
    if (isTransparentPass) return;

    glPushMatrix();
    // --- Phep bien doi: Tinh tien + Quay (Chap 6) ---
    glTranslatef(sofaX, 0.0f, sofaZ);
    glRotatef(sofaAngle, 0.0f, 1.0f, 0.0f);

    bool sel = (lvSelectedObj == 1);

    // -- Vai nham khong bong (Specular = 0) --
    lv_mat(0.48f, 0.48f, 0.54f, 0, 0, 0, 0);
    lv_applySelectionTint(sel);

    // Ghe dai chinh
    glPushMatrix(); glTranslatef(0.0f, 0.28f, 0.0f); lv_drawBox(3.80f, 0.45f, 1.20f); glPopMatrix();

    // Ghe goc chu L (bo sung)
    glPushMatrix(); glTranslatef(1.50f, 0.28f, 1.55f); lv_drawBox(1.10f, 0.45f, 1.90f); glPopMatrix();

    // Dem ngoi (goc R, G, B sang hon mot chut)
    lv_mat(0.58f, 0.56f, 0.62f, 0, 0, 0, 0);
    lv_applySelectionTint(sel);
    glPushMatrix(); glTranslatef(0.0f, 0.54f, 0.0f); lv_drawBox(3.60f, 0.12f, 1.10f); glPopMatrix();
    glPushMatrix(); glTranslatef(1.50f, 0.54f, 1.55f); lv_drawBox(0.90f, 0.12f, 1.75f); glPopMatrix();

    // Tua lung chinh
    lv_mat(0.42f, 0.42f, 0.48f, 0, 0, 0, 0);
    lv_applySelectionTint(sel);
    glPushMatrix(); glTranslatef(0.0f, 0.82f, -0.42f); lv_drawBox(3.80f, 0.75f, 0.38f); glPopMatrix();

    // Tua lung goc L
    glPushMatrix(); glTranslatef(1.82f, 0.82f, 1.25f); lv_drawBox(0.38f, 0.75f, 2.10f); glPopMatrix();

    // Tua tay 2 ben
    lv_mat(0.38f, 0.38f, 0.44f, 0, 0, 0, 0);
    lv_applySelectionTint(sel);
    glPushMatrix(); glTranslatef(-1.92f, 0.55f, 0.0f);  lv_drawBox(0.36f, 0.50f, 1.20f); glPopMatrix();
    glPushMatrix(); glTranslatef(1.92f, 0.55f, 0.0f);  lv_drawBox(0.36f, 0.50f, 1.20f); glPopMatrix();

    // Chan sofa (go toi, co bong - Specular cao)
    lv_mat(0.22f, 0.14f, 0.08f, 0.5f, 0.4f, 0.3f, 30);
    float legPos[][3] = {
        {-1.75f, 0.09f, -0.50f}, { 1.75f, 0.09f, -0.50f},
        {-1.75f, 0.09f,  0.50f}, { 2.25f, 0.09f,  0.50f},
        { 2.25f, 0.09f,  2.40f}, { 0.95f, 0.09f,  2.40f}
    };
    for (int i = 0; i < 6; i++) {
        glPushMatrix();
        glTranslatef(legPos[i][0], legPos[i][1], legPos[i][2]);
        lv_drawBox(0.10f, 0.18f, 0.10f);
        glPopMatrix();
    }

    // Goi tua (mau be am)
    lv_mat(0.65f, 0.50f, 0.35f, 0.1f, 0.1f, 0.1f, 8);
    lv_applySelectionTint(sel);
    glPushMatrix(); glTranslatef(-0.85f, 0.72f, -0.18f); lv_drawBox(0.65f, 0.48f, 0.28f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.65f, 0.72f, -0.18f); lv_drawBox(0.65f, 0.48f, 0.28f); glPopMatrix();

    lv_resetEmission();
    glPopMatrix(); // het sofa
}

// ============================================================
// 2. BAN TRA KINH  -  Alpha Blending (2-Pass Rendering)
//    Pass 1 (isTransparentPass=false): chan ban + khung go
//    Pass 2 (isTransparentPass=true ): mat kinh RGBA alpha
// ============================================================
static void drawCoffeeTable()
{
    glPushMatrix();
    glTranslatef(tableX, 0.0f, tableZ);
    glRotatef(tableAngle, 0.0f, 1.0f, 0.0f);

    bool sel = (lvSelectedObj == 2);

    // ---- PASS 1: Chan ban va khung (duc, ve binh thuong) ----
    if (!isTransparentPass) {

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texWood);
        lv_mat(1, 1, 1, 0.6f, 0.5f, 0.4f, 25);
        lv_applySelectionTint(sel);

        // 4 chan ban
        float cp[][3] = {
            {-0.78f, 0.20f, -0.37f}, { 0.78f, 0.20f, -0.37f},
            {-0.78f, 0.20f,  0.37f}, { 0.78f, 0.20f,  0.37f}
        };
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glTranslatef(cp[i][0], cp[i][1], cp[i][2]);
            lv_drawBox(0.09f, 0.40f, 0.09f);
            glPopMatrix();
        }

        // Ke ngang phia duoi
        glPushMatrix(); glTranslatef(0, 0.10f, 0); lv_drawBox(1.45f, 0.06f, 0.68f); glPopMatrix();

        // Khung do mat kinh (sat sat mep duoi mat kinh)
        glPushMatrix(); glTranslatef(0, 0.41f, 0); lv_drawBox(1.82f, 0.05f, 0.88f); glPopMatrix();

        glDisable(GL_TEXTURE_2D);
    }

    // ---- PASS 2: Mat kinh trong suot (Alpha Blending - Chap 7) ----
    // Ly thuyet: dung Blending de hoa tron mau vat the voi nen
    // Cong thuc: C_out = alpha * C_src + (1-alpha) * C_dst
    // -> Alpha = 0.38: mat kinh trong suot 62%, giu lai 38% mau xanh
    if (isTransparentPass) {

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Khong ghi vao depth buffer khi ve trong suot
        // Tranh loi: vat phia sau bi an boi mat kinh du mat kinh trong suot
        glDepthMask(GL_FALSE);

        // Vat lieu kinh: bong cao, Specular trang
        GLfloat sp[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat dif[] = { 0.22f, 0.58f, 0.82f, 0.38f }; // alpha = 0.38
        GLfloat amb[] = { 0.08f, 0.22f, 0.35f, 0.38f };
        GLfloat em[] = { 0,0,0,1 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sp);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 110.0f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
        glColor4f(0.22f, 0.58f, 0.82f, 0.38f);

        glPushMatrix();
        glTranslatef(0, 0.44f, 0);
        lv_drawBox(2.05f, 0.04f, 1.10f); // Mat kinh lon hon khung
        glPopMatrix();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // Reset specular
        GLfloat def[] = { 0,0,0,1 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, def);
    }

    lv_resetEmission();
    glPopMatrix();
}

// ============================================================
// 3. HE THONG TV VA KE  -  Dynamic Emission (Chap 7)
//    Ke go: texture texWood
//    Man hinh: GL_EMISSION thay doi theo tvR/tvG/tvB (file chinh quan ly)
//    Anh sang TV: GL_LIGHT2 (neu file chinh cau hinh)
// ============================================================
static void drawTVSystem()
{
    if (isTransparentPass) return;

    // Toa do: dat sat tuong phia sau (Z = -4.2 so voi goc phong khach)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -4.20f);

    // === 3.1 Ke TV bang go ===
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texWood);
    lv_mat(1, 1, 1, 0.5f, 0.4f, 0.3f, 20);

    // Than chinh ke
    glPushMatrix(); glTranslatef(0, 0.28f, 0); lv_drawBox(5.20f, 0.55f, 0.75f); glPopMatrix();
    // Canh trai ke
    glPushMatrix(); glTranslatef(-2.42f, 0.62f, 0); lv_drawBox(0.36f, 0.65f, 0.75f); glPopMatrix();
    // Canh phai ke
    glPushMatrix(); glTranslatef(2.42f, 0.62f, 0); lv_drawBox(0.36f, 0.65f, 0.75f); glPopMatrix();
    // Ngan giua (chia 2 o)
    glPushMatrix(); glTranslatef(0, 0.62f, 0); lv_drawBox(0.28f, 0.65f, 0.75f); glPopMatrix();
    // Mat sau ke
    glPushMatrix(); glTranslatef(0, 0.55f, -0.34f); lv_drawBox(5.20f, 1.10f, 0.06f); glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // === 3.2 Gia do TV (sat xi) ===
    lv_mat(0.18f, 0.18f, 0.20f, 0.9f, 0.9f, 0.9f, 90);
    // Thanh doc
    glPushMatrix(); glTranslatef(0, 1.08f, 0.28f); lv_drawBox(0.08f, 0.78f, 0.08f); glPopMatrix();
    // Thanh ngang do TV
    glPushMatrix(); glTranslatef(0, 1.42f, 0.10f); lv_drawBox(1.10f, 0.06f, 0.42f); glPopMatrix();

    // === 3.3 Khung TV (nhua den bong) ===
    glPushMatrix();
    glTranslatef(0.0f, 2.70f, 0.0f);

    lv_mat(0.06f, 0.06f, 0.07f, 0.95f, 0.95f, 0.95f, 100);
    lv_drawBox(4.30f, 2.45f, 0.11f);   // Vien ngoai

    // === 3.4 Man hinh - Dynamic Emission ===
    // Ly thuyet: GL_EMISSION la mau phat ra khong phu thuoc nguon sang
    // Khi tvOn=true: man hinh tu phat sang voi mau tvR/tvG/tvB
    // tvR/tvG/tvB duoc cap nhat moi frame trong update() cua file chinh:
    //   if (tvOn) { tvR = rand%10/10.0f; tvG = ...; tvB = ...; }
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.062f);  // Nho ra truoc vien mot chut

    if (tvOn) {
        // Emission chinh la mau phat sang
        GLfloat em[] = { tvR, tvG, tvB, 1.0f };
        GLfloat dif[] = { tvR * 0.7f + 0.3f, tvG * 0.7f + 0.3f, tvB * 0.7f + 0.3f, 1.0f };
        GLfloat sp[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sp);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
        glColor3f(tvR, tvG, tvB);
    }
    else {
        // Tat TV: man hinh den, khong phat sang
        lv_mat(0.03f, 0.03f, 0.04f, 0.2f, 0.2f, 0.2f, 20);
    }
    lv_drawBox(4.00f, 2.18f, 0.02f);

    lv_resetEmission();
    glPopMatrix(); // het man hinh
    glPopMatrix(); // het khung TV

    // === 3.5 Den trang tri ke TV (LED strip mau vang) ===
    GLfloat emLED[] = { 0.9f, 0.75f, 0.3f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emLED);
    glColor3f(0.9f, 0.75f, 0.3f);
    glPushMatrix(); glTranslatef(0, 0.62f, 0.385f); lv_drawBox(4.80f, 0.04f, 0.02f); glPopMatrix();
    lv_resetEmission();

    // =================================================================
    // 3.6 HIỂN THỊ PHÍM GỢI Ý (HINT) KHI LẠI GẦN TIVI (2D HUD)
    // =================================================================
    GLfloat mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);

    // Tính khoảng cách từ mắt Camera đến TIVI
    float distance = std::sqrt(mv[12] * mv[12] + mv[13] * mv[13] + mv[14] * mv[14]);

    if (distance < 25.0f && !isTransparentPass) {
        // 1. Chuyển sang hệ tọa độ 2D (HUD) để chữ luôn cố định giữa màn hình máy tính
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600); // Khớp với tỷ lệ HUD trong main.cpp

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Tắt chiều sâu và ánh sáng để vẽ chữ 2D đè lên trên cùng
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        extern bool tvOn;
        const char* hint = tvOn ? "Nhan [T] de TAT Tivi" : "Nhan [T] de BAT Tivi";

        // 2. TẠO VIỀN ĐEN (Bóng đổ chữ): Vẽ chữ màu đen lệch đi 2 pixel
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(322, 250); // Đặt ở gần giữa dưới màn hình
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        // 3. Vẽ chữ màu vàng đè lên đúng tọa độ gốc
        glColor3f(1.0f, 0.9f, 0.1f);
        glRasterPos2f(320, 252);
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        // 4. Bật lại trạng thái 3D ban đầu
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    glPopMatrix(); // het TV system
}

// ============================================================
// 4. THAM LON (trang tri san)
// ============================================================
static void drawRug()
{
    if (isTransparentPass) return;

    // Tham dat duoi sofa va ban tra
    lv_mat(0.42f, 0.35f, 0.58f, 0, 0, 0, 0);
    glPushMatrix();
    glTranslatef(0.2f, 0.01f, 1.2f);
    lv_drawBox(4.80f, 0.03f, 4.20f);
    glPopMatrix();

    // Duong vien tham (soc vang nhe)
    lv_mat(0.65f, 0.55f, 0.35f, 0, 0, 0, 0);
    glPushMatrix(); glTranslatef(0.2f, 0.025f, 1.2f); lv_drawBox(4.82f, 0.015f, 0.06f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.2f, 0.025f, 3.18f); lv_drawBox(4.82f, 0.015f, 0.06f); glPopMatrix();
    glPushMatrix(); glTranslatef(-2.18f, 0.025f, 1.2f); lv_drawBox(0.06f, 0.015f, 4.20f); glPopMatrix();
    glPushMatrix(); glTranslatef(2.60f, 0.025f, 1.2f); lv_drawBox(0.06f, 0.015f, 4.20f); glPopMatrix();
}

// ============================================================
// 5. CAY CANH GOC PHONG
// ============================================================
static void drawPlant(float x, float z)
{
    if (isTransparentPass) return;

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // Chau cay (tron - dung glutSolidCone + Sphere)
    lv_mat(0.55f, 0.38f, 0.22f, 0.2f, 0.1f, 0.05f, 15);
    glPushMatrix();
    glRotatef(180.0f, 1, 0, 0);  // Lat nguoc cone de lam chau
    glutSolidCone(0.22, 0.30, 12, 4);
    glPopMatrix();

    // Dat chau
    glPushMatrix(); glTranslatef(0, 0.01f, 0); lv_drawBox(0.44f, 0.02f, 0.44f); glPopMatrix();

    // Than cay
    lv_mat(0.25f, 0.16f, 0.09f, 0, 0, 0, 0);
    glPushMatrix(); glTranslatef(0, 0.52f, 0); lv_drawBox(0.07f, 0.55f, 0.07f); glPopMatrix();

    // Tan la (3 qua cau xanh)
    lv_mat(0.18f, 0.52f, 0.18f, 0.05f, 0.15f, 0.05f, 10);
    glPushMatrix(); glTranslatef(0, 0.95f, 0); glutSolidSphere(0.38, 10, 8); glPopMatrix();
    lv_mat(0.22f, 0.48f, 0.20f, 0, 0, 0, 0);
    glPushMatrix(); glTranslatef(0.22f, 0.82f, 0.12f); glutSolidSphere(0.24, 8, 6); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f, 0.86f, -0.10f); glutSolidSphere(0.20, 8, 6); glPopMatrix();

    glPopMatrix();
}

// ============================================================
// 6. DEN DOC GOC PHONG KHACH
// ============================================================
static void drawFloorLamp(float x, float z)
{
    if (isTransparentPass) return;

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // Chan de
    lv_mat(0.18f, 0.18f, 0.20f, 0.8f, 0.8f, 0.8f, 70);
    glPushMatrix(); glTranslatef(0, 0.04f, 0); lv_drawBox(0.28f, 0.08f, 0.28f); glPopMatrix();

    // Than den (thanh kim loai tron)
    GLUquadric* q = gluNewQuadric();
    glPushMatrix();
    glTranslatef(0, 0.08f, 0);
    glRotatef(-90.0f, 1, 0, 0);
    lv_mat(0.22f, 0.22f, 0.24f, 0.9f, 0.9f, 0.9f, 80);
    gluCylinder(q, 0.04, 0.04, 1.55, 10, 2);
    glPopMatrix();

    // Chup den (cone)
    lv_mat(0.85f, 0.80f, 0.65f, 0.1f, 0.1f, 0.1f, 10);
    glPushMatrix();
    glTranslatef(0, 1.75f, 0);
    glRotatef(180.0f, 1, 0, 0);
    glutSolidCone(0.28, 0.38, 14, 4);
    glPopMatrix();

    // Bong den (phat sang)
    GLfloat emL[] = { 0.95f, 0.88f, 0.65f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emL);
    glColor3f(0.95f, 0.88f, 0.65f);
    glPushMatrix();
    glTranslatef(0, 1.60f, 0);
    glutSolidSphere(0.08, 8, 6);
    glPopMatrix();
    lv_resetEmission();

    gluDeleteQuadric(q);
    glPopMatrix();
}

// ============================================================
// 7. KHUNG TRANH TUONG
// ============================================================
static void drawWallArt(float x, float y, float z,
    float w, float h,
    float fr, float fg, float fb,
    float pr, float pg, float pb)
{
    if (isTransparentPass) return;

    glPushMatrix();
    glTranslatef(x, y, z);

    // Vien khung
    lv_mat(fr, fg, fb, 0.4f, 0.3f, 0.2f, 20);
    lv_drawBox(w + 0.14f, h + 0.14f, 0.06f);

    // Mat tranh
    lv_mat(pr, pg, pb, 0, 0, 0, 0);
    glPushMatrix(); glTranslatef(0, 0, 0.04f); lv_drawBox(w, h, 0.01f); glPopMatrix();

    glPopMatrix();
}

// ============================================================
// 8. GHE DON BEN CANH SOFA
// ============================================================
static void drawArmchair(float x, float z, float angle)
{
    if (isTransparentPass) return;

    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(angle, 0, 1, 0);

    // De ghe
    lv_mat(0.55f, 0.38f, 0.28f, 0.1f, 0.1f, 0.1f, 15);
    glPushMatrix(); glTranslatef(0, 0.26f, 0); lv_drawBox(0.85f, 0.38f, 0.82f); glPopMatrix();

    // Dem ngoi
    lv_mat(0.62f, 0.45f, 0.32f, 0, 0, 0, 0);
    glPushMatrix(); glTranslatef(0, 0.48f, 0); lv_drawBox(0.78f, 0.10f, 0.75f); glPopMatrix();

    // Tua lung
    lv_mat(0.52f, 0.36f, 0.26f, 0, 0, 0, 0);
    glPushMatrix(); glTranslatef(0, 0.75f, -0.34f); lv_drawBox(0.85f, 0.60f, 0.10f); glPopMatrix();

    // Tua tay 2 ben
    glPushMatrix(); glTranslatef(-0.44f, 0.58f, 0); lv_drawBox(0.10f, 0.30f, 0.80f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.44f, 0.58f, 0); lv_drawBox(0.10f, 0.30f, 0.80f); glPopMatrix();

    // Chan (4 chan go)
    lv_mat(0.22f, 0.14f, 0.08f, 0.5f, 0.4f, 0.3f, 30);
    float lp[][3] = { {-0.36f,0.09f,-0.34f},{0.36f,0.09f,-0.34f},
                     {-0.36f,0.09f, 0.34f},{0.36f,0.09f, 0.34f} };
    for (int i = 0; i < 4; i++) {
        glPushMatrix(); glTranslatef(lp[i][0], lp[i][1], lp[i][2]);
        lv_drawBox(0.08f, 0.18f, 0.08f); glPopMatrix();
    }
    glPopMatrix();
}

// ============================================================
// VẼ QUẠT TRẦN - Tích hợp tính năng hiện Hint 2D khi lại gần
// ============================================================
extern float fanAngle; // Lấy góc quay từ main.cpp sang
extern bool fanOn;     // Lấy trạng thái bật tắt quạt từ main.cpp

static void drawCeilingFan(float x, float y, float z)
{
    if (isTransparentPass) return;

    glPushMatrix();
    glTranslatef(x, y, z); // Đưa quạt lên trần nhà

    // 1. Trục treo quạt (Cố định, không quay)
    lv_mat(0.2f, 0.2f, 0.2f, 0.8f, 0.8f, 0.8f, 50); // Nhựa đen bọc nhôm
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    lv_drawBox(0.05f, 0.3f, 0.05f);
    glPopMatrix();

    // 2. Bầu quạt & Cánh quạt (Sẽ xoay theo fanAngle)
    glPushMatrix();
    glRotatef(fanAngle, 0.0f, 1.0f, 0.0f); // Lệnh này giúp quạt quay

    // Bầu quạt (Cục tròn ở giữa)
    lv_mat(0.8f, 0.8f, 0.8f, 0.5f, 0.5f, 0.5f, 20); // Màu trắng xám
    glPushMatrix();
    glutSolidSphere(0.12, 16, 16);
    glPopMatrix();

    // 3 Cánh quạt (Chia đều 120 độ)
    lv_mat(0.4f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f, 10); // Cánh gỗ
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glRotatef(i * 120.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.6f, 0.0f, 0.0f); // Dời cánh ra khỏi tâm bầu quạt
        lv_drawBox(1.0f, 0.02f, 0.15f); // Kích thước 1 cánh
        glPopMatrix();
    }

    glPopMatrix(); // Hết phần xoay

    // =================================================================
    // HIỂN THỊ PHÍM GỢI Ý (HINT) KHI LẠI GẦN QUẠT (2D HUD Overlay)
    // =================================================================
    GLfloat mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);

    // Tính khoảng cách từ mắt Camera đến tâm khối chiếc quạt
    float distance = std::sqrt(mv[12] * mv[12] + mv[13] * mv[13] + mv[14] * mv[14]);

    // Nếu đứng trong bán kính vùng quạt (khoảng cách < 22.0f)
    if (distance < 22.0f) {
        // Tạm thời chuyển sang chế độ ma trận 2D HUD để cố định chữ trên màn hình
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Cô lập ánh sáng và độ sâu để chữ hiển thị rõ nhất
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        const char* hint = fanOn ? "Nhan [F] de TAT Quat" : "Nhan [F] de BAT Quat";

        // 1. Tạo bóng chữ màu đen (Drop Shadow) tránh bị chìm nền
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(322, 200); // Đặt ở cao độ Y=200 (hơi thấp hơn dòng chữ TIVI một chút để không đè nhau)
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        // 2. Chữ chính màu xanh Cyan (Xanh ngọc sáng) để phân biệt màu với phím Tivi
        glColor3f(0.0f, 1.0f, 1.0f);
        glRasterPos2f(320, 202);
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        // Trả lại trạng thái chiếu sáng 3D cho các vật thể khác
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    // =================================================================

    glPopMatrix(); // Hết quạt trần
}

// ============================================================
// VẼ ĐÈN ỐP TRẦN (Kích hoạt nguồn sáng thật GL_LIGHT1)
// ============================================================
static void drawCeilingLight(float x, float y, float z) {
    if (isTransparentPass) return;
    glPushMatrix();
    glTranslatef(x, y, z);

    // 1. Đế đèn ốp trần bằng viền kim loại
    lv_mat(0.2f, 0.2f, 0.2f, 0.8f, 0.8f, 0.8f, 80);
    glPushMatrix(); glTranslatef(0, 0.05f, 0); lv_drawBox(0.8f, 0.05f, 0.8f); glPopMatrix();

    // 2. Chụp mica phát sáng & Xử lý Nguồn sáng thật
    if (lvLightOn) {
        // Làm bóng đèn tự phát sáng rực rỡ
        GLfloat em[] = { 1.0f, 0.95f, 0.8f, 1.0f }; // Vàng ấm
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
        glColor3f(1.0f, 0.95f, 0.8f);

        // KÍCH HOẠT ĐÈN CHIẾU SÁNG TOÀN CĂN PHÒNG
        glEnable(GL_LIGHT1);
        GLfloat lightPos[] = { 0.0f, -0.5f, 0.0f, 1.0f }; // Vị trí phát sáng tại tâm bóng đèn
        GLfloat lightDif[] = { 0.6f, 0.6f, 0.5f, 1.0f };  // Ánh sáng tỏa ra
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDif);
    }
    else {
        // Khi tắt: Mica trắng đục, tắt nguồn sáng
        lv_mat(0.9f, 0.9f, 0.9f, 0.2f, 0.2f, 0.2f, 20);
        glDisable(GL_LIGHT1);
    }

    glPushMatrix();
    glutSolidSphere(0.3, 20, 20); // Bầu đèn tròn
    glPopMatrix();

    lv_resetEmission();
    glPopMatrix();
}

// ============================================================
// VẼ CÔNG TẮC ĐIỆN & HIỆN HUD KHI LẠI GẦN
// ============================================================
static void drawLightSwitch(float x, float y, float z) {
    if (isTransparentPass) return;
    glPushMatrix();
    glTranslatef(x, y, z);

    // 1. Khung công tắc (Áp sát vào tường trái X = -4.88f)
    lv_mat(0.9f, 0.9f, 0.9f, 0.1f, 0.1f, 0.1f, 10);
    lv_drawBox(0.02f, 0.15f, 0.1f); // Hộp mỏng dẹt

    // 2. Nút bấm điện (Bật thì hiện đèn đỏ nhỏ, Tắt hiện đèn xám)
    if (lvLightOn) {
        lv_mat(0.9f, 0.2f, 0.2f, 0.5f, 0.5f, 0.5f, 50); // Nút đỏ rực
        glPushMatrix(); glTranslatef(0.015f, 0.02f, 0.0f); lv_drawBox(0.01f, 0.04f, 0.04f); glPopMatrix();
    }
    else {
        lv_mat(0.3f, 0.3f, 0.3f, 0.5f, 0.5f, 0.5f, 50); // Nút xám chìm
        glPushMatrix(); glTranslatef(0.015f, -0.02f, 0.0f); lv_drawBox(0.01f, 0.04f, 0.04f); glPopMatrix();
    }

    // 3. HUD: CHỈ HIỆN CHỮ KHI ĐỨNG GẦN CÔNG TẮC
    GLfloat mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    float distance = std::sqrt(mv[12] * mv[12] + mv[13] * mv[13] + mv[14] * mv[14]);

    if (distance < 15.0f) { // Nếu khoảng cách tới công tắc < 15
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
        glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);

        const char* hint = lvLightOn ? "Nhan [B] de TAT Den Phong Khach" : "Nhan [B] de BAT Den Phong Khach";

        // Bóng chữ màu đen
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(282, 150); // Y=150 để nằm dưới dòng chữ của Quạt trần
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        // Chữ chính màu Cam rực rỡ
        glColor3f(1.0f, 0.6f, 0.0f);
        glRasterPos2f(280, 152);
        for (const char* c = hint; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    }
    glPopMatrix();
}

// ============================================================
// HAM TONG HOP  -  Goi tu drawGroundFloor() trong file chinh
//
// Goc toa do (0,0,0) o day tuong ung voi vi tri da duoc
// glTranslatef(0, 0.65f, 0) trong drawGroundFloor().
// => Y=0 trong ham nay = mat san go cua phong khach.
// ============================================================
void drawLivingRoomInterior()
{
    // Dich chuyen sang vi tri phong khach trong ngoi nha
    // (Dieu chinh neu can thiet cho phu hop toa do tong the)
    glPushMatrix();
    glTranslatef(-5.5f, 0.0f, 1.5f);

    // --- Tham lon ---
    drawRug();

    // --- Sofa chu L (Hierarchical Modeling) ---
    drawSofa();

    // --- Ban tra kinh (Alpha Blending - 2 pass) ---
    // Pass 1 (isTransparentPass=false): chan + khung go
    // Pass 2 (isTransparentPass=true ): mat kinh alpha
    drawCoffeeTable();

    // --- He thong TV va ke (Dynamic Emission) ---
    drawTVSystem();

    // --- Ghe don ---
    drawArmchair(3.2f, 0.4f, -90.0f);

    // --- Den doc (2 cai) ---
    drawFloorLamp(-4.8f, 3.2f);
    drawFloorLamp(3.2f, -3.5f);

    // --- Cay canh ---
    drawPlant(4.5f, 3.0f);
    drawPlant(-4.5f, -3.8f);

    // --- Tranh tuong phia sau (sat tuong TV) ---
    // Dat 2 buc tranh 2 ben TV
    drawWallArt(-3.8f, 2.20f, -4.88f, 0.90f, 1.20f,
        0.45f, 0.30f, 0.18f, 0.25f, 0.45f, 0.68f);
    drawWallArt(3.8f, 2.20f, -4.88f, 0.90f, 1.20f,
        0.45f, 0.30f, 0.18f, 0.68f, 0.30f, 0.25f);

    // --- Tranh tuong ben canh ---
    drawWallArt(-4.88f, 2.50f, 0.0f, 0.08f, 1.50f,
        0.40f, 0.28f, 0.16f, 0.80f, 0.72f, 0.55f);

    // --- Quạt trần ---
    drawCeilingFan(0.0f, 3.6f, 0.0f);

    // Đèn trần cỡ lớn: Đặt ngay giữa phòng khách (gần quạt)
    drawCeilingLight(0.0f, 3.6f, -2.0f);

    // Công tắc điện: Ốp sát vào bức tường bên trái (X = -4.88f) vừa tầm tay với (Y = 1.3f)
    drawLightSwitch(-4.88f, 1.3f, 2.0f);

    glPopMatrix();
}

// ============================================================
// KEYBOARD HANDLER - Goi trong keyboard() cua file chinh
// ============================================================
// Phim dieu khien:
//   1 / 2 / 3   : Chon sofa / ban / bo chon
//   I / K       : Tien / lui doi tuong dang chon (truc Z)
//   J / L       : Trai / phai doi tuong dang chon (truc X)
//   U / O       : Xoay trai / phai doi tuong dang chon
// ============================================================
void LivingRoomKeyboard(unsigned char key, int /*x*/, int /*y*/)
{
    const float STEP = 0.18f;
    const float ROT = 8.0f;

    switch (key) {
        // Chon doi tuong
    case '1': lvSelectedObj = (lvSelectedObj == 1) ? 0 : 1; break;
    case '2': lvSelectedObj = (lvSelectedObj == 2) ? 0 : 2; break;
    case '3': lvSelectedObj = 0; break;

        // Di chuyen
    case 'b': case 'B':
        lvLightOn = !lvLightOn;
        break;
    case 'i': case 'I':
        if (lvSelectedObj == 1) { sofaZ -= STEP; lv_clamp(sofaX, sofaZ); }
        if (lvSelectedObj == 2) { tableZ -= STEP; lv_clamp(tableX, tableZ); }
        break;
    case 'k': case 'K':
        // 'K' da duoc dung cho window, nen dung phim hoa thuong de tranh xung dot
        if (lvSelectedObj == 1) { sofaZ += STEP; lv_clamp(sofaX, sofaZ); }
        if (lvSelectedObj == 2) { tableZ += STEP; lv_clamp(tableX, tableZ); }
        break;
    case 'j': case 'J':
        if (lvSelectedObj == 1) { sofaX -= STEP; lv_clamp(sofaX, sofaZ); }
        if (lvSelectedObj == 2) { tableX -= STEP; lv_clamp(tableX, tableZ); }
        break;
    case 'l': case 'L':
        if (lvSelectedObj == 1) { sofaX += STEP; lv_clamp(sofaX, sofaZ); }
        if (lvSelectedObj == 2) { tableX += STEP; lv_clamp(tableX, tableZ); }
        break;
    case 'u': case 'U':
        if (lvSelectedObj == 1) sofaAngle += ROT;
        if (lvSelectedObj == 2) tableAngle += ROT;
        break;
    case 'o': case 'O':
        // 'o'/'O' da duoc dung cho mainDoor, nhung van the them dieu kien
        if (lvSelectedObj == 1) { sofaAngle -= ROT; return; } // Neu dang chon sofa thi an o
        if (lvSelectedObj == 2) { tableAngle -= ROT; return; }
        // Neu khong chon gi thi de file chinh xu ly phim 'o' (open door)
        break;
    }
}

// ============================================================
// UPDATE ANIMATION - Goi trong update() cua file chinh moi frame
// dt: thoi gian giua 2 frame (giay)
// ============================================================
void LivingRoomUpdate(float dt)
{
    lvTvTimer += dt;
}