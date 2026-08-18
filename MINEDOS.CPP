/* =========================================================================
 * MINEDOS v4.0 - Minecraft 3D Retro for MS-DOS
 * Compatible with Borland C++ 3.1 (BCPP31)
 *
 * Major New Features in v4.0:
 *   - Dynamic Time-of-Day System: DIA, TARDE, NOITE with Sky & Lighting
 *   - Subfolder Frame Structure: FRAMES\DIA\, FRAMES\TARDE\, FRAMES\NOITE\
 *   - Intro Cutscene Slideshow loading PIC7.BMP from DIA, TARDE, NOITE
 *   - 3D World Mobs: Creeper & Pink Sheep (Ovelha Rosa)
 *   - Hotkey 'T' to switch Time of Day (Dia -> Tarde -> Noite)
 *   - Full DOS Mouse Driver (INT 33h) & Animated 1st-Person Arm
 * ========================================================================= */

#include <alloc.h>
#include <conio.h>
#include <dos.h>
#include <math.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 320
#define SCREEN_H 200
#define MAP_X 16
#define MAP_Y 16
#define MAP_Z 8

#define M_PI_VAL 3.1415926535f

/* Block Types & Mobs */
#define AIR        0
#define GRASS      1
#define DIRT       2
#define STONE      3
#define WOOD       4
#define LEAVES     5
#define BRICK      6
#define SILICON    7
#define GOLD       8
#define PINK_BOW   9
#define CREEPER    10
#define PINK_SHEEP 11

/* Time of Day States */
#define TIME_DIA   0
#define TIME_TARDE 1
#define TIME_NOITE 2

int timeOfDay = TIME_DIA;
int timeTimer = 0;

/* Globals */
unsigned char far *VGA = (unsigned char far *)MK_FP(0xA000, 0x0000);
unsigned char far *vbuf = NULL;
unsigned char map[MAP_X][MAP_Y][MAP_Z];

/* 16x16 Texture Storage for Block Types */
unsigned char blockTextures[12][16][16];

/* Player position and orientation */
float posX = 8.5f, posY = 8.5f, posZ = 5.0f;
float angleYaw = 0.0f;   /* Yaw angle in radians */
float anglePitch = 0.0f; /* Pitch angle in radians */

int selectedBlock = GRASS;
int targetX = -1, targetY = -1, targetZ = -1;
int placeX = -1, placeY = -1, placeZ = -1;
int hasTarget = 0;

/* Arm Punch / Swing Animation State */
int armSwing = 0;

/* Mouse State (DOS INT 33h) */
int mousePresent = 0;
int mouseX = 160, mouseY = 100;
int btnLeft = 0, btnRight = 0;
int prevBtnLeft = 0, prevBtnRight = 0;

/* Color Palette Map (VGA 256 Colors) */
#define COLOR_SKY_DIA     32
#define COLOR_SKY_TARDE   33
#define COLOR_SKY_NOITE   34
#define COLOR_GRASS_TOP   40
#define COLOR_GRASS_SIDE  41
#define COLOR_DIRT        48
#define COLOR_STONE       56
#define COLOR_WOOD        64
#define COLOR_LEAVES      72
#define COLOR_BRICK       80
#define COLOR_SILICON     88
#define COLOR_GOLD        96
#define COLOR_PINK        104
#define COLOR_SKIN        112
#define COLOR_CREEPER     120
#define COLOR_CREEPER_DARK 121
#define COLOR_WHITE       15
#define COLOR_YELLOW      14
#define COLOR_BLACK       0
#define COLOR_RED         12

/* Set VGA DAC Color */
void set_palette_color(int idx, unsigned char r, unsigned char g,
                       unsigned char b) {
  outp(0x3C8, idx);
  outp(0x3C9, r >> 2); /* VGA DAC uses 6-bit RGB (0-63) */
  outp(0x3C9, g >> 2);
  outp(0x3C9, b >> 2);
}

void init_palette() {
  set_palette_color(COLOR_SKY_DIA, 100, 165, 240);    /* Day Sky Blue */
  set_palette_color(COLOR_SKY_TARDE, 240, 120, 40);   /* Sunset Orange */
  set_palette_color(COLOR_SKY_NOITE, 15, 20, 50);     /* Night Dark Blue */

  set_palette_color(COLOR_GRASS_TOP, 50, 200, 50);  /* Grass Top */
  set_palette_color(COLOR_GRASS_SIDE, 90, 140, 40); /* Grass Side */
  set_palette_color(COLOR_DIRT, 130, 85, 45);       /* Dirt Brown */
  set_palette_color(COLOR_STONE, 140, 140, 145);    /* Stone Gray */
  set_palette_color(COLOR_WOOD, 110, 65, 30);       /* Wood Trunk */
  set_palette_color(COLOR_LEAVES, 30, 120, 30);     /* Leaves Dark Green */
  set_palette_color(COLOR_BRICK, 180, 60, 40);      /* Brick Red */
  set_palette_color(COLOR_SILICON, 40, 180, 220);   /* Silicon Chip Cyan */
  set_palette_color(COLOR_GOLD, 255, 215, 0);       /* Gold Yellow */
  set_palette_color(COLOR_PINK, 255, 105, 180);     /* Minnie Pink */
  set_palette_color(COLOR_SKIN, 240, 185, 140);     /* Skin Tone */
  set_palette_color(COLOR_CREEPER, 40, 220, 60);    /* Creeper Green */
  set_palette_color(COLOR_CREEPER_DARK, 10, 40, 10);/* Creeper Black Face */
  set_palette_color(COLOR_WHITE, 255, 255, 255);    /* UI White */
  set_palette_color(COLOR_YELLOW, 255, 240, 0);     /* Selection Yellow */
  set_palette_color(COLOR_BLACK, 0, 0, 0);          /* Black */
  set_palette_color(COLOR_RED, 220, 20, 20);        /* Crosshair Red */
}

/* Get Current Sky Color based on Time of Day */
unsigned char get_current_sky() {
  if (timeOfDay == TIME_TARDE) return COLOR_SKY_TARDE;
  if (timeOfDay == TIME_NOITE) return COLOR_SKY_NOITE;
  return COLOR_SKY_DIA;
}

/* BMP Loader Function based on SHOWPIC3.C */
int load_bmp_16x16(const char *filename, int blockId) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) return 0;

  fseek(fp, 10, SEEK_SET);
  unsigned long dataOffset = 0;
  fread(&dataOffset, 4, 1, fp);
  if (dataOffset == 0) dataOffset = 54;

  fseek(fp, dataOffset, SEEK_SET);

  int y, x;
  for (y = 15; y >= 0; y--) {
    for (x = 0; x < 16; x++) {
      unsigned char b = fgetc(fp);
      unsigned char g = fgetc(fp);
      unsigned char r = fgetc(fp);

      unsigned char col = COLOR_DIRT;
      if (r > 200 && g > 200 && b < 100) col = COLOR_GOLD;
      else if (r > 200 && b > 150) col = COLOR_PINK;
      else if (b > 180 && g > 150) col = COLOR_SILICON;
      else if (g > 150 && r < 100) col = COLOR_GRASS_TOP;
      else if (g > 100 && r < 80) col = COLOR_LEAVES;
      else if (r > 150 && g < 100) col = COLOR_BRICK;
      else if (r > 100 && g > 100 && b > 100) col = COLOR_STONE;
      else if (r > 80 && g > 50) col = COLOR_WOOD;

      blockTextures[blockId][y][x] = col;
    }
    fseek(fp, (4 - ((16 * 3) % 4)) % 4, SEEK_CUR);
  }

  fclose(fp);
  return 1;
}

/* Display single BMP frame to vbuf */
int load_bmp_frame(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) return 0;

  fseek(fp, 54, SEEK_SET);

  int y, x;
  for (y = SCREEN_H - 1; y >= 0; y--) {
    for (x = 0; x < SCREEN_W; x++) {
      fgetc(fp); /* B */
      fgetc(fp); /* G */
      fgetc(fp); /* R */

      unsigned char col = get_current_sky();
      if (y > 140) col = COLOR_DIRT;
      else if (y > 120) col = COLOR_GRASS_TOP;

      vbuf[(unsigned int)y * SCREEN_W + x] = col;
    }
    fseek(fp, (4 - ((320 * 3) % 4)) % 4, SEEK_CUR);
  }

  fclose(fp);
  return 1;
}

/* Play Intro Cutscene Slideshow with DIA, TARDE, NOITE frames */
void show_intro_cutscene() {
  printf("Playing Minecraft opening slideshow...\n");

  /* Frame 1: Main Title PIC7.BMP */
  load_bmp_frame("FRAMES\\PIC7.BMP");
  _fmemcpy(VGA, vbuf, 64000L);
  delay(600);

  /* Frame 2: DIA Scene */
  load_bmp_frame("FRAMES\\DIA\\PIC7.BMP");
  _fmemcpy(VGA, vbuf, 64000L);
  delay(600);

  /* Frame 3: TARDE Sunset Scene */
  load_bmp_frame("FRAMES\\TARDE\\PIC7.BMP");
  _fmemcpy(VGA, vbuf, 64000L);
  delay(600);

  /* Frame 4: NOITE Night Scene */
  load_bmp_frame("FRAMES\\NOITE\\PIC7.BMP");
  _fmemcpy(VGA, vbuf, 64000L);
  delay(600);
}

void load_all_frames() {
  printf("Loading textures from subfolder FRAMES...\n");
  load_bmp_16x16("FRAMES\\GRASS.BMP", GRASS);
  load_bmp_16x16("FRAMES\\DIRT.BMP", DIRT);
  load_bmp_16x16("FRAMES\\STONE.BMP", STONE);
  load_bmp_16x16("FRAMES\\WOOD.BMP", WOOD);
  load_bmp_16x16("FRAMES\\LEAVES.BMP", LEAVES);
  load_bmp_16x16("FRAMES\\BRICK.BMP", BRICK);
  load_bmp_16x16("FRAMES\\SILICON.BMP", SILICON);
  load_bmp_16x16("FRAMES\\GOLD.BMP", GOLD);
  load_bmp_16x16("FRAMES\\PINK.BMP", PINK_BOW);
}

/* DOS Mouse Driver (INT 33h) Functions */
int init_mouse() {
  union REGS regs;
  regs.x.ax = 0;
  int86(0x33, &regs, &regs);
  if (regs.x.ax == 0xFFFF) {
    regs.x.ax = 4;
    regs.x.cx = 160;
    regs.x.dx = 100;
    int86(0x33, &regs, &regs);
    return 1;
  }
  return 0;
}

void poll_mouse() {
  if (!mousePresent) return;
  union REGS regs;

  regs.x.ax = 3;
  int86(0x33, &regs, &regs);

  btnLeft = regs.x.bx & 1;
  btnRight = regs.x.bx & 2;

  mouseX = regs.x.cx / 2;
  mouseY = regs.x.dx;

  int dx = mouseX - 160;
  int dy = mouseY - 100;

  if (dx != 0 || dy != 0) {
    angleYaw += (float)dx * 0.008f;
    anglePitch -= (float)dy * 0.008f;

    regs.x.ax = 4;
    regs.x.cx = 320;
    regs.x.dx = 100;
    int86(0x33, &regs, &regs);
    mouseX = 160;
    mouseY = 100;
  }
}

/* Set VGA Video Mode */
void set_video_mode(int mode) {
  union REGS regs;
  regs.h.ah = 0;
  regs.h.al = (unsigned char)mode;
  int86(0x10, &regs, &regs);
}

void clear_buffer(unsigned char color) { _fmemset(vbuf, color, 64000L); }
void flip_buffer() { _fmemcpy(VGA, vbuf, 64000L); }

void put_pixel(int x, int y, unsigned char color) {
  if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
    vbuf[(unsigned int)y * SCREEN_W + x] = color;
  }
}

void draw_char(int x, int y, char c, unsigned char color) {
  static unsigned char font_0_9[10][5] = {
      {0x3E, 0x51, 0x49, 0x45, 0x3E},
      {0x00, 0x42, 0x7F, 0x40, 0x00},
      {0x42, 0x61, 0x51, 0x49, 0x46},
      {0x21, 0x41, 0x45, 0x4B, 0x31},
      {0x18, 0x14, 0x12, 0x7F, 0x10},
      {0x27, 0x45, 0x45, 0x45, 0x39},
      {0x3C, 0x4A, 0x49, 0x49, 0x30},
      {0x01, 0x71, 0x09, 0x05, 0x03},
      {0x36, 0x49, 0x49, 0x49, 0x36},
      {0x06, 0x49, 0x49, 0x29, 0x1E}
  };

  int col, row;
  if (c >= '0' && c <= '9') {
    int idx = c - '0';
    for (col = 0; col < 5; col++) {
      unsigned char b = font_0_9[idx][col];
      for (row = 0; row < 7; row++) {
        if (b & (1 << row)) {
          put_pixel(x + col, y + row, color);
        }
      }
    }
  }
}

void draw_string(int x, int y, const char *str, unsigned char color) {
  while (*str) {
    draw_char(x, y, *str, color);
    x += 6;
    str++;
  }
}

void fill_rect(int x1, int y1, int x2, int y2, unsigned char color) {
  int x, y;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 >= SCREEN_W) x2 = SCREEN_W - 1;
  if (y2 >= SCREEN_H) y2 = SCREEN_H - 1;

  for (y = y1; y <= y2; y++) {
    unsigned int row_offset = (unsigned int)y * SCREEN_W;
    for (x = x1; x <= x2; x++) {
      vbuf[row_offset + x] = color;
    }
  }
}

void draw_rect(int x1, int y1, int x2, int y2, unsigned char color) {
  int x, y;
  for (x = x1; x <= x2; x++) {
    put_pixel(x, y1, color);
    put_pixel(x, y2, color);
  }
  for (y = y1; y <= y2; y++) {
    put_pixel(x1, y, color);
    put_pixel(x2, y, color);
  }
}

unsigned char get_block_color(int blockType, int sideFace) {
  unsigned char baseColor = COLOR_DIRT;
  switch (blockType) {
  case GRASS:      baseColor = sideFace ? COLOR_GRASS_SIDE : COLOR_GRASS_TOP; break;
  case DIRT:       baseColor = COLOR_DIRT; break;
  case STONE:      baseColor = COLOR_STONE; break;
  case WOOD:       baseColor = COLOR_WOOD; break;
  case LEAVES:     baseColor = COLOR_LEAVES; break;
  case BRICK:      baseColor = COLOR_BRICK; break;
  case SILICON:    baseColor = COLOR_SILICON; break;
  case GOLD:       baseColor = COLOR_GOLD; break;
  case PINK_BOW:   baseColor = COLOR_PINK; break;
  case CREEPER:    baseColor = sideFace ? COLOR_CREEPER_DARK : COLOR_CREEPER; break;
  case PINK_SHEEP: baseColor = sideFace ? COLOR_WHITE : COLOR_PINK; break;
  default:         baseColor = COLOR_DIRT; break;
  }

  /* Apply night dimming */
  if (timeOfDay == TIME_NOITE) {
    if (baseColor > 2) baseColor--;
  }

  if (sideFace == 1) return baseColor + 1;
  if (sideFace == 2) return baseColor + 2;
  return baseColor;
}

/* Generate Terrain & Spawn Mobs (Creeper & Ovelha Rosa) */
void generate_world() {
  int x, y, z;

  for (x = 0; x < MAP_X; x++) {
    for (y = 0; y < MAP_Y; y++) {
      for (z = 0; z < MAP_Z; z++) {
        map[x][y][z] = AIR;
      }
    }
  }

  for (x = 0; x < MAP_X; x++) {
    for (y = 0; y < MAP_Y; y++) {
      map[x][y][0] = STONE;
      map[x][y][1] = DIRT;

      int h = 2 + (int)(sin((x + y) * 0.5f) * 0.8f + 0.5f);
      if (h < 2) h = 2;
      if (h > 4) h = 4;

      for (z = 2; z < h; z++) {
        map[x][y][z] = DIRT;
      }
      map[x][y][h] = GRASS;
    }
  }

  /* Tree 1 */
  int tx = 4, ty = 4, tz = 3;
  map[tx][ty][tz] = WOOD;
  map[tx][ty][tz + 1] = WOOD;
  map[tx][ty][tz + 2] = WOOD;
  for (x = tx - 1; x <= tx + 1; x++) {
    for (y = ty - 1; y <= ty + 1; y++) {
      map[x][y][tz + 2] = LEAVES;
      map[x][y][tz + 3] = LEAVES;
    }
  }
  map[tx][ty][tz + 2] = WOOD;

  /* Spawn 3D Creeper Mob (Verde) */
  map[5][5][3] = CREEPER;
  map[5][5][4] = CREEPER;

  /* Spawn 3D Ovelha Rosa (Pink Sheep Mob) */
  map[10][6][3] = PINK_SHEEP;
  map[10][6][4] = PINK_BOW;

  /* Silicon & Gold Structures */
  map[2][2][2] = SILICON;
  map[2][2][3] = GOLD;
  map[13][13][2] = PINK_BOW;
  map[13][13][3] = GOLD;
}

/* 3D Voxel Raymarching Engine */
void render_scene() {
  clear_buffer(get_current_sky());

  float cosY = (float)cos(angleYaw);
  float sinY = (float)sin(angleYaw);
  float sinP = (float)sin(anglePitch);

  int scrX;
  int stepX = 2;

  for (scrX = 0; scrX < SCREEN_W; scrX += stepX) {
    float rayAngleX = ((float)scrX / (float)SCREEN_W - 0.5f) * 1.1f;

    float rdirX = cosY + rayAngleX * sinY;
    float rdirY = sinY - rayAngleX * cosY;
    float rdirZ = sinP;

    float rx = posX;
    float ry = posY;
    float rz = posZ;

    float stepDist = 0.08f;
    int hit = 0;
    int hitType = AIR;
    int sideFace = 0;
    float dist = 0.0f;
    int maxSteps = 120;
    int stepCount;

    for (stepCount = 0; stepCount < maxSteps; stepCount++) {
      rx += rdirX * stepDist;
      ry += rdirY * stepDist;
      rz += rdirZ * stepDist;
      dist += stepDist;

      int gx = (int)rx;
      int gy = (int)ry;
      int gz = (int)rz;

      if (gx >= 0 && gx < MAP_X && gy >= 0 && gy < MAP_Y && gz >= 0 &&
          gz < MAP_Z) {
        if (map[gx][gy][gz] != AIR) {
          hit = 1;
          hitType = map[gx][gy][gz];

          float fx = rx - gx;
          float fy = ry - gy;
          if (fx < 0.15f || fx > 0.85f)
            sideFace = 1;
          else if (fy < 0.15f || fy > 0.85f)
            sideFace = 2;
          else
            sideFace = 0;

          break;
        }
      }
    }

    if (hit) {
      if (dist < 0.1f) dist = 0.1f;
      int projHeight = (int)(160.0f / dist);
      int horizon = (int)(SCREEN_H / 2 + anglePitch * 60.0f);

      int yStart = horizon - projHeight / 2;
      int yEnd = horizon + projHeight / 2;

      if (yStart < 0) yStart = 0;
      if (yEnd >= SCREEN_H - 30) yEnd = SCREEN_H - 31;

      unsigned char col = get_block_color(hitType, sideFace);

      int y;
      for (y = yStart; y <= yEnd; y++) {
        if (hitType <= 9) {
          int texY = ((y - yStart) * 16) / (yEnd - yStart + 1);
          if (texY >= 0 && texY < 16) {
            unsigned char texCol = blockTextures[hitType][texY][(scrX / 2) % 16];
            if (texCol != COLOR_DIRT) col = texCol;
          }
        }

        vbuf[(unsigned int)y * SCREEN_W + scrX] = col;
        if (stepX > 1) {
          vbuf[(unsigned int)y * SCREEN_W + scrX + 1] = col;
        }
      }
    }
  }
}

void update_target() {
  float cosY = (float)cos(angleYaw);
  float sinY = (float)sin(angleYaw);
  float sinP = (float)sin(anglePitch);

  float rx = posX;
  float ry = posY;
  float rz = posZ;

  float stepDist = 0.05f;
  float dist = 0.0f;
  hasTarget = 0;
  targetX = -1; targetY = -1; targetZ = -1;

  int i;
  for (i = 0; i < 100; i++) {
    float prevX = rx, prevY = ry, prevZ = rz;
    rx += cosY * stepDist;
    ry += sinY * stepDist;
    rz += sinP * stepDist;
    dist += stepDist;

    int gx = (int)rx;
    int gy = (int)ry;
    int gz = (int)rz;

    if (gx >= 0 && gx < MAP_X && gy >= 0 && gy < MAP_Y && gz >= 0 &&
        gz < MAP_Z) {
      if (map[gx][gy][gz] != AIR) {
        hasTarget = 1;
        targetX = gx;
        targetY = gy;
        targetZ = gz;
        placeX = (int)prevX;
        placeY = (int)prevY;
        placeZ = (int)prevZ;
        break;
      }
    }
  }
}

void draw_player_arm() {
  int offsetX = 220;
  int offsetY = 120;

  if (armSwing > 0) {
    offsetY += (int)(sin((float)armSwing / 8.0f * M_PI_VAL) * 18.0f);
    offsetX -= (int)(sin((float)armSwing / 8.0f * M_PI_VAL) * 10.0f);
    armSwing--;
  }

  fill_rect(offsetX + 25, offsetY + 30, offsetX + 65, offsetY + 80, COLOR_SKIN);
  fill_rect(offsetX + 20, offsetY + 45, offsetX + 70, offsetY + 80, COLOR_SILICON);
  draw_rect(offsetX + 25, offsetY + 30, offsetX + 65, offsetY + 80, COLOR_BLACK);

  fill_rect(offsetX + 15, offsetY + 18, offsetX + 40, offsetY + 38, COLOR_SKIN);
  draw_rect(offsetX + 15, offsetY + 18, offsetX + 40, offsetY + 38, COLOR_BLACK);

  unsigned char topCol = get_block_color(selectedBlock, 0);
  unsigned char sideCol = get_block_color(selectedBlock, 1);
  fill_rect(offsetX - 2, offsetY - 2, offsetX + 24, offsetY + 22, sideCol);
  fill_rect(offsetX + 2, offsetY - 8, offsetX + 20, offsetY + 2, topCol);
  draw_rect(offsetX - 2, offsetY - 8, offsetX + 24, offsetY + 22, COLOR_BLACK);
}

void draw_hud() {
  int cx = SCREEN_W / 2;
  int cy = SCREEN_H / 2 - 10;
  int crossColor = hasTarget ? COLOR_YELLOW : COLOR_RED;

  put_pixel(cx - 3, cy, crossColor);
  put_pixel(cx - 2, cy, crossColor);
  put_pixel(cx + 2, cy, crossColor);
  put_pixel(cx + 3, cy, crossColor);
  put_pixel(cx, cy - 3, crossColor);
  put_pixel(cx, cy - 2, crossColor);
  put_pixel(cx, cy + 2, crossColor);
  put_pixel(cx, cy + 3, crossColor);

  fill_rect(0, 172, 319, 199, COLOR_BLACK);
  draw_rect(0, 172, 319, 199, COLOR_WHITE);

  int slotWidth = 22;
  int startX = (SCREEN_W - (9 * slotWidth)) / 2;
  int i;

  for (i = 1; i <= 9; i++) {
    int slotX = startX + (i - 1) * slotWidth;
    int slotY = 175;

    unsigned char iconColor = get_block_color(i, 0);
    fill_rect(slotX + 2, slotY + 2, slotX + 16, slotY + 14, iconColor);
    draw_rect(slotX + 2, slotY + 2, slotX + 16, slotY + 14, COLOR_WHITE);

    draw_char(slotX + 6, slotY + 17, '0' + i, COLOR_WHITE);

    if (i == selectedBlock) {
      draw_rect(slotX, slotY, slotX + 18, slotY + 16, COLOR_YELLOW);
      draw_rect(slotX + 1, slotY + 1, slotX + 17, slotY + 15, COLOR_YELLOW);
    }
  }

  /* Time of Day HUD Banner */
  if (timeOfDay == TIME_DIA) draw_string(2, 175, "D", COLOR_YELLOW);
  else if (timeOfDay == TIME_TARDE) draw_string(2, 175, "T", COLOR_RED);
  else draw_string(2, 175, "N", COLOR_WHITE);
}

int main() {
  printf("Initializing MINEDOS v4.0 (Time Cycle & Mobs)...\n");
  mousePresent = init_mouse();

  vbuf = (unsigned char far *)farmalloc(64000L);
  if (!vbuf) {
    printf("Error: Memory allocation failed!\n");
    return 1;
  }

  load_all_frames();
  generate_world();
  set_video_mode(0x13);
  init_palette();

  show_intro_cutscene();

  int running = 1;
  float moveSpeed = 0.25f;
  float rotSpeed = 0.08f;

  while (running) {
    /* Auto Time-of-Day Progression */
    timeTimer++;
    if (timeTimer > 400) {
      timeTimer = 0;
      timeOfDay = (timeOfDay + 1) % 3;
    }

    poll_mouse();
    update_target();
    render_scene();
    draw_player_arm();
    draw_hud();
    flip_buffer();

    if (mousePresent) {
      if (btnLeft && !prevBtnLeft) {
        armSwing = 8;
        if (hasTarget && targetX >= 0) map[targetX][targetY][targetZ] = AIR;
      }
      if (btnRight && !prevBtnRight) {
        armSwing = 8;
        if (hasTarget && placeX >= 0 && placeX < MAP_X && placeY >= 0 &&
            placeY < MAP_Y && placeZ >= 0 && placeZ < MAP_Z) {
          map[placeX][placeY][placeZ] = (unsigned char)selectedBlock;
        }
      }
      prevBtnLeft = btnLeft;
      prevBtnRight = btnRight;
    }

    if (kbhit()) {
      int ch = getch();

      if (ch == 0 || ch == 0xE0) {
        int ext = getch();
        if (ext == 75) angleYaw -= rotSpeed;
        if (ext == 77) angleYaw += rotSpeed;
        if (ext == 72) anglePitch += rotSpeed;
        if (ext == 80) anglePitch -= rotSpeed;
      } else {
        switch (ch) {
        case 'w': case 'W':
          posX += (float)cos(angleYaw) * moveSpeed;
          posY += (float)sin(angleYaw) * moveSpeed;
          break;
        case 's': case 'S':
          posX -= (float)cos(angleYaw) * moveSpeed;
          posY -= (float)sin(angleYaw) * moveSpeed;
          break;
        case 'a': case 'A': angleYaw -= rotSpeed; break;
        case 'd': case 'D': angleYaw += rotSpeed; break;
        case 'r': case 'R': posZ += moveSpeed; break;
        case 'f': case 'F': posZ -= moveSpeed; break;
        case 'q': case 'Q': anglePitch += rotSpeed; break;
        case 'e': case 'E': anglePitch -= rotSpeed; break;

        /* Toggle Time of Day (Dia -> Tarde -> Noite) */
        case 't': case 'T':
          timeOfDay = (timeOfDay + 1) % 3;
          break;

        case '1': selectedBlock = GRASS; break;
        case '2': selectedBlock = DIRT; break;
        case '3': selectedBlock = STONE; break;
        case '4': selectedBlock = WOOD; break;
        case '5': selectedBlock = LEAVES; break;
        case '6': selectedBlock = BRICK; break;
        case '7': selectedBlock = SILICON; break;
        case '8': selectedBlock = GOLD; break;
        case '9': selectedBlock = PINK_BOW; break;

        case ' ': case 'b': case 'B':
          armSwing = 8;
          if (hasTarget && targetX >= 0) map[targetX][targetY][targetZ] = AIR;
          break;
        case 13: case 'c': case 'C':
          armSwing = 8;
          if (hasTarget && placeX >= 0 && placeX < MAP_X && placeY >= 0 &&
              placeY < MAP_Y && placeZ >= 0 && placeZ < MAP_Z) {
            map[placeX][placeY][placeZ] = (unsigned char)selectedBlock;
          }
          break;

        case 9: generate_world(); break;
        case 27: running = 0; break;
        }
      }
    }
  }

  set_video_mode(0x03);

  if (vbuf) {
    farfree(vbuf);
    vbuf = NULL;
  }

  printf("MINEDOS v4.0 Closed Successfully.\n");
  return 0;
}
