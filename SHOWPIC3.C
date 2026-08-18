#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <time.h>
#include <string.h>

#define TEXT_X 16
#define TEXT_Y 356
#define TEXT_W 240
#define TEXT_H 120

int current_bank = -1;
unsigned char far *vga = (unsigned char far *)MK_FP(0xA000, 0);

const char *dias_da_semana[] = {
    "Domingo", "Segunda-feira", "Terca-feira",
    "Quarta-feira", "Quinta-feira", "Sexta-feira", "Sabado"
};

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

void set_palette(unsigned char index, unsigned char r, unsigned char g, unsigned char b) {
    outp(0x03C8, index);
    outp(0x03C9, r);
    outp(0x03C9, g);
    outp(0x03C9, b);
}

void put_pixel(int x, int y, unsigned char color) {
    long offset;
    int new_bank;
    unsigned int vga_offset;

    if (x < 0 || x >= 640 || y < 0 || y >= 480) return;

    offset = (long)y * 640 + x;
    new_bank = offset >> 16;
    vga_offset = offset & 0xFFFF;

    if (new_bank != current_bank) {
        set_vesa_bank(new_bank);
        current_bank = new_bank;
    }
    vga[vga_offset] = color;
}

unsigned char far *get_rom_font(void) {
    struct REGPACK regs;
    regs.r_ax = 0x1130;
    regs.r_bx = 0x0600; /* bh = 06 */
    intr(0x10, &regs);
    return (unsigned char far *)MK_FP(regs.r_es, regs.r_bp);
}

void draw_char(unsigned char c, int x, int y, int scale, unsigned char color, unsigned char far *font) {
    int i, j, si, sj;
    unsigned char far *bitmap = font + (c * 16);
    
    for (i = 0; i < 16; i++) {
        unsigned char row = bitmap[i];
        for (j = 0; j < 8; j++) {
            if (row & (0x80 >> j)) {
                for (si = 0; si < scale; si++) {
                    for (sj = 0; sj < scale; sj++) {
                        put_pixel(x + j * scale + sj, y + i * scale + si, color);
                    }
                }
            }
        }
    }
}

void draw_string(const char *str, int x, int y, int scale, unsigned char color, unsigned char far *font) {
    while (*str) {
        draw_char(*str, x, y, scale, color, font);
        x += 8 * scale;
        str++;
    }
}

void update_text(const char *str, int start_x, int start_y, int scale, unsigned char far *font, unsigned char *bg_buffer) {
    int w = strlen(str) * 8 * scale + scale; 
    int h = 16 * scale + scale;
    int x, y;

    if (bg_buffer) {
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                int screen_x = start_x + x;
                int screen_y = start_y + y;
                
                if (screen_x >= TEXT_X && screen_x < TEXT_X + TEXT_W && 
                    screen_y >= TEXT_Y && screen_y < TEXT_Y + TEXT_H) {
                    
                    put_pixel(screen_x, screen_y, bg_buffer[(screen_y - TEXT_Y) * TEXT_W + (screen_x - TEXT_X)]);
                }
            }
        }
    }

    draw_string(str, start_x + 2, start_y + 2, scale, 254, font);
    draw_string(str, start_x, start_y, scale, 255, font);
}

int main(int argc, char *argv[]) {
    FILE *fp;
    unsigned char palette[1024];
    int i, x, y;
    unsigned char *buffer;
    unsigned char *bg_buffer;
    long offset;
    int new_bank;
    int w = 640, h = 480;
    unsigned int vga_offset;
    const char *filename;
    
    time_t t;
    struct tm *tm_info;
    struct time t_dos;

    char buf[32];
    unsigned char far *font;

    int last_hund = -1, last_sec = -1, last_min = -1, last_hour = -1;
    int last_day = -1, last_month = -1, last_year = -1, last_wday = -1;

    if (argc >= 2) {
        filename = argv[1];
    } else {
        filename = "PIC.BMP";
    }

    fp = fopen(filename, "rb");
    if (!fp) {
        printf("Erro ao abrir %s\n", filename);
        return 1;
    }

    fseek(fp, 54, SEEK_SET);
    fread(palette, 1, 1024, fp);

    set_vesa_mode(0x101);

    for (i = 0; i < 256; i++) {
        set_palette(i, palette[i*4+2] >> 2, palette[i*4+1] >> 2, palette[i*4] >> 2);
    }

    buffer = (unsigned char *)malloc(640);
    bg_buffer = (unsigned char *)malloc(TEXT_W * TEXT_H);
    
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
            
            if (x >= TEXT_X && x < TEXT_X + TEXT_W && y >= TEXT_Y && y < TEXT_Y + TEXT_H) {
                if (bg_buffer) {
                    bg_buffer[(y - TEXT_Y) * TEXT_W + (x - TEXT_X)] = buffer[x];
                }
            }
            
            vga_offset++;
            if (vga_offset == 0) {
                current_bank++;
                set_vesa_bank(current_bank);
            }
        }
    }

    free(buffer);
    fclose(fp);

    font = get_rom_font();

    set_palette(254, 0, 0, 0);       /* Preto para a sombra e fundo translucido */
    set_palette(255, 63, 63, 63);    /* Branco puro para o texto */

    /* Aplica o overlay no buffer inteiro para otimizar desenho das partes */
    if (bg_buffer) {
        for (y = 0; y < TEXT_H; y++) {
            for (x = 0; x < TEXT_W; x++) {
                int screen_x = TEXT_X + x;
                int screen_y = TEXT_Y + y;
                unsigned char color = bg_buffer[y * TEXT_W + x];
                
                if ((screen_x + screen_y) % 2 == 0) {
                    color = 254; 
                }
                
                bg_buffer[y * TEXT_W + x] = color;
                put_pixel(screen_x, screen_y, color);
            }
        }
    }

    /* Partes estaticas */
    update_text(":", 52, 440, 2, font, bg_buffer);
    update_text(":", 100, 440, 2, font, bg_buffer);
    update_text(".", 148, 440, 2, font, bg_buffer);

    update_text("/", 52, 400, 2, font, bg_buffer);
    update_text("/", 100, 400, 2, font, bg_buffer);

    while (!kbhit()) {
        gettime(&t_dos);
        time(&t);
        tm_info = localtime(&t);

        if (t_dos.ti_hund != last_hund) {
            sprintf(buf, "%02d", t_dos.ti_hund);
            update_text(buf, 164, 440, 2, font, bg_buffer);
            last_hund = t_dos.ti_hund;
        }

        if (t_dos.ti_sec != last_sec) {
            sprintf(buf, "%02d", t_dos.ti_sec);
            update_text(buf, 116, 440, 2, font, bg_buffer);
            last_sec = t_dos.ti_sec;
        }

        if (t_dos.ti_min != last_min) {
            sprintf(buf, "%02d", t_dos.ti_min);
            update_text(buf, 68, 440, 2, font, bg_buffer);
            last_min = t_dos.ti_min;
        }

        if (t_dos.ti_hour != last_hour) {
            sprintf(buf, "%02d", t_dos.ti_hour);
            update_text(buf, 20, 440, 2, font, bg_buffer);
            last_hour = t_dos.ti_hour;
        }

        if (tm_info->tm_mday != last_day) {
            sprintf(buf, "%02d", tm_info->tm_mday);
            update_text(buf, 20, 400, 2, font, bg_buffer);
            last_day = tm_info->tm_mday;
        }

        if (tm_info->tm_mon != last_month) {
            sprintf(buf, "%02d", tm_info->tm_mon + 1);
            update_text(buf, 68, 400, 2, font, bg_buffer);
            last_month = tm_info->tm_mon;
        }

        if (tm_info->tm_year != last_year) {
            sprintf(buf, "%04d", tm_info->tm_year + 1900);
            update_text(buf, 116, 400, 2, font, bg_buffer);
            last_year = tm_info->tm_year;
        }

        if (tm_info->tm_wday != last_wday) {
            sprintf(buf, "%-14s", dias_da_semana[tm_info->tm_wday]); /* Pad to clear old text */
            update_text(buf, 20, 360, 2, font, bg_buffer);
            last_wday = tm_info->tm_wday;
        }
    }

    getch(); 

    {
        union REGS regs;
        regs.x.ax = 0x0003;
        int86(0x10, &regs, &regs);
    }

    if (bg_buffer) free(bg_buffer);

    return 0;
}
