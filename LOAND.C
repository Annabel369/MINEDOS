#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

void set_vesa_mode(int mode) {
  union REGS regs;
  regs.x.ax = 0x4F02;
  regs.x.bx = mode;
  int86(0x10, &regs, &regs);
}

void set_vesa_bank(int bank) {
  union REGS regs;
  regs.x.ax = 0x4F05;
  regs.x.bx = 0;
  regs.x.dx = bank;
  int86(0x10, &regs, &regs);
}

void set_palette(unsigned char index, unsigned char r, unsigned char g,
                 unsigned char b) {
  outp(0x03C8, index);
  outp(0x03C9, r);
  outp(0x03C9, g);
  outp(0x03C9, b);
}

void draw_rect(int rx, int ry, int rw, int rh, unsigned char color) {
  int x, y;
  int current_bank = -1;
  long offset;
  int bank;
  unsigned int vga_offset;
  unsigned char far *vga;

  for (y = ry; y < ry + rh; y++) {
    offset = (long)y * 640 + rx;
    for (x = 0; x < rw; x++) {
      bank = offset >> 16;
      vga_offset = offset & 0xFFFF;
      if (bank != current_bank) {
        set_vesa_bank(bank);
        current_bank = bank;
      }
      vga = (unsigned char far *)MK_FP(0xA000, vga_offset);
      *vga = color;
      offset++;
    }
  }
}

int main() {
  FILE *fp;
  unsigned char palette[1024];
  unsigned char far *vga = (unsigned char far *)MK_FP(0xA000, 0);
  int i, x, y;
  unsigned char *buffer;
  long offset;
  int current_bank = -1;
  int new_bank;
  int w = 640, h = 480;
  unsigned int vga_offset;

  fp = fopen("PIC7.BMP", "rb");
  if (!fp) {
    printf("Erro ao abrir PIC.BMP\n");
    return 1;
  }

  fseek(fp, 54, SEEK_SET);
  fread(palette, 1, 1024, fp);

  set_vesa_mode(0x101);

  for (i = 0; i < 256; i++) {
    set_palette(i, palette[i * 4 + 2] >> 2, palette[i * 4 + 1] >> 2,
                palette[i * 4] >> 2);
  }

  // Cores para barra de progresso (Sobrescreve finais da paleta)
  set_palette(253, 0, 0, 0);    // Preto
  set_palette(254, 63, 63, 63); // Branco
  set_palette(255, 0, 60, 0);   // Verde Creeper

  buffer = (unsigned char *)malloc(640);
  for (y = h - 1; y >= 0; y--) {
    fread(buffer, 1, 640, fp);

    offset = (long)y * w;
    new_bank = offset >> 16;
    vga_offset = offset & 0xFFFF;

    if (new_bank != current_bank) {
      set_vesa_bank(new_bank);
      current_bank = new_bank;
    }

    for (x = 0; x < w; x++) {
      vga[vga_offset] = buffer[x];
      vga_offset++;
      if (vga_offset == 0) {
        current_bank++;
        set_vesa_bank(current_bank);
      }
    }
  }

  free(buffer);
  fclose(fp);

  // Desenha o contorno da barra de progresso e o fundo
  draw_rect(98, 398, 444, 34, 254);  // Borda branca
  draw_rect(100, 400, 440, 30, 253); // Fundo preto

  // Loop da barra de progresso (10 segundos = 100 steps de 100ms)
  // A cada passo preenchemos um pouco mais da barra
  for (i = 1; i <= 100; i++) {
    int width = (440 * i) / 100;
    int total_blocks = 10;
    int current_blocks = (i * total_blocks) / 100;
    int b;

    // Limpa a barra
    draw_rect(100, 400, 440, 30, 253); // Fundo preto

    // Desenha os blocos verdes ativos
    for (b = 0; b < current_blocks; b++) {
      draw_rect(105 + (b * 43), 405, 38, 20, 255);
    }

    delay(100);
  }

  // Espera por uma tecla apos o termino (opcional, pode remover depois se
  // quiser que saia automatico)
  getch();

  {
    union REGS regs;
    regs.x.ax = 0x0003;
    int86(0x10, &regs, &regs);
  }

  return 0;
}
